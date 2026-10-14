#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/fs_context.h>
#include <linux/mount.h>
#include <linux/namei.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/file.h>
#include <linux/statfs.h>
#include <linux/string.h>

#define CHIFFREFS_NAME  "chiffrefs"
#define CHIFFREFS_MAGIC 0x43484652

struct chiffrefs_context {
	int n;
	char *source;
};

/* --------------------------------------------------------- */
/* Construction du chemin vers le fichier réel               */
/* --------------------------------------------------------- */

static char *chiffrefs_real_path(struct chiffrefs_context *ctx,
				 const char *name)
{
	return kasprintf(GFP_KERNEL, "%s/%s", ctx->source, name);
}

/* --------------------------------------------------------- */
/* Lecture : le fichier réel est lu puis chaque caractère     */
/* est décalé de n positions                                  */
/* --------------------------------------------------------- */

static ssize_t chiffrefs_read(struct file *file,
			      char __user *buf,
			      size_t count,
			      loff_t *ppos)
{
	struct inode *inode = file_inode(file);
	struct chiffrefs_context *ctx = inode->i_sb->s_fs_info;
	struct path path;
	struct file *real_file;
	char *real_path;
	char *kbuf;
	ssize_t ret;
	size_t i;

	if (*ppos >= i_size_read(inode))
		return 0;

	if (count > PAGE_SIZE)
		count = PAGE_SIZE;

	real_path = chiffrefs_real_path(ctx, inode->i_private);
	if (!real_path)
		return -ENOMEM;

	ret = kern_path(real_path, LOOKUP_FOLLOW, &path);
	kfree(real_path);

	if (ret)
		return ret;

	real_file = dentry_open(&path, O_RDONLY, current_cred());
	path_put(&path);

	if (IS_ERR(real_file))
		return PTR_ERR(real_file);

	kbuf = kmalloc(count, GFP_KERNEL);
	if (!kbuf) {
		fput(real_file);
		return -ENOMEM;
	}

	ret = kernel_read(real_file, kbuf, count, ppos);

	fput(real_file);

	if (ret <= 0) {
		kfree(kbuf);
		return ret;
	}

	/* Décalage César */
	for (i = 0; i < ret; i++) {
		unsigned char c = kbuf[i];

		if (c >= 'a' && c <= 'z')
			kbuf[i] = 'a' + ((c - 'a' + ctx->n) % 26);
		else if (c >= 'A' && c <= 'Z')
			kbuf[i] = 'A' + ((c - 'A' + ctx->n) % 26);
	}

	if (copy_to_user(buf, kbuf, ret)) {
		kfree(kbuf);
		return -EFAULT;
	}

	kfree(kbuf);

	return ret;
}
static ssize_t chiffrefs_write(struct file *file,
                               const char __user *buf,
                               size_t count,
                               loff_t *ppos)
{
    struct inode *inode = file_inode(file);
    struct chiffrefs_context *ctx = inode->i_sb->s_fs_info;
    struct file *real_file;
    char *path;
    char *kbuf;
    ssize_t ret;
    size_t i;

    if (!inode->i_private)
        return -ENOENT;

    path = chiffrefs_real_path(ctx, inode->i_private);
    if (!path)
        return -ENOMEM;

    kbuf = memdup_user(buf, count);
    if (IS_ERR(kbuf)) {
        kfree(path);
        return PTR_ERR(kbuf);
    }

    /*
     * Le fichier virtuel présente les données avec un décalage.
     * On inverse donc le décalage avant de les écrire dans le
     * répertoire de fond.
     */
    for (i = 0; i < count; i++) {
        if (kbuf[i] >= 'a' && kbuf[i] <= 'z')
            kbuf[i] = 'a' +
                (kbuf[i] - 'a' - (ctx->n % 26) + 26) % 26;
        else if (kbuf[i] >= 'A' && kbuf[i] <= 'Z')
            kbuf[i] = 'A' +
                (kbuf[i] - 'A' - (ctx->n % 26) + 26) % 26;
    }

    real_file = filp_open(path, O_WRONLY | O_TRUNC, 0);
    if (IS_ERR(real_file)) {
        ret = PTR_ERR(real_file);
        goto out;
    }

    ret = kernel_write(real_file, kbuf, count, ppos);

    if (ret > 0)
        i_size_write(inode, ret);

    filp_close(real_file, NULL);

out:
    kfree(kbuf);
    kfree(path);

    return ret;
}

/* --------------------------------------------------------- */
/* Opérations sur les fichiers                                */
/* --------------------------------------------------------- */

static const struct file_operations chiffrefs_file_ops = {
    .read = chiffrefs_read,
    .write = chiffrefs_write,
    .llseek = default_llseek,
};
static const struct inode_operations chiffrefs_file_inode_ops = {
};
/* --------------------------------------------------------- */
/* lookup : quand le VFS cherche chiffred/test, on vérifie    */
/* si clear/test existe réellement                             */
/* --------------------------------------------------------- */

static struct dentry *chiffrefs_lookup(struct inode *dir,
				       struct dentry *dentry,
				       unsigned int flags)
{
	struct chiffrefs_context *ctx = dir->i_sb->s_fs_info;
	struct inode *inode;
	struct path path;
	char *real_path;
	int ret;

	real_path = chiffrefs_real_path(ctx, dentry->d_name.name);
	if (!real_path)
		return ERR_PTR(-ENOMEM);

	ret = kern_path(real_path, LOOKUP_FOLLOW, &path);
	kfree(real_path);

	if (ret)
		return ERR_PTR(-ENOENT);

	if (!S_ISREG(d_inode(path.dentry)->i_mode)) {
		path_put(&path);
		return ERR_PTR(-ENOENT);
	}

	inode = new_inode(dir->i_sb);
	if (!inode) {
		path_put(&path);
		return ERR_PTR(-ENOMEM);
	}

	inode->i_ino = get_next_ino();
	inode->i_mode = S_IFREG | 0644;
	inode->i_uid = GLOBAL_ROOT_UID;
	inode->i_gid = GLOBAL_ROOT_GID;

	inode->i_size = i_size_read(d_inode(path.dentry));

	inode->i_op = &chiffrefs_file_inode_ops;
	inode->i_fop = &chiffrefs_file_ops;

	inode->i_private = kstrdup(dentry->d_name.name, GFP_KERNEL);

	if (!inode->i_private) {
		iput(inode);
		path_put(&path);
		return ERR_PTR(-ENOMEM);
	}

	path_put(&path);

	d_add(dentry, inode);

	return NULL;
}

/* --------------------------------------------------------- */
/* Répertoire racine                                          */
/* --------------------------------------------------------- */

static const struct inode_operations chiffrefs_dir_inode_ops = {
	.lookup = chiffrefs_lookup,
};

static void chiffrefs_evict_inode(struct inode *inode)
{
        kfree(inode->i_private);
        inode->i_private = NULL;
        clear_inode(inode);
}

static const struct super_operations chiffrefs_super_ops = {
        .statfs = simple_statfs,
        .evict_inode = chiffrefs_evict_inode,
};

/* --------------------------------------------------------- */
/* Création du superblock                                     */
/* --------------------------------------------------------- */

static int chiffrefs_fill_super(struct super_block *sb,
				struct fs_context *fc)
{
	struct inode *inode;
	struct chiffrefs_context *ctx = fc->fs_private;

	pr_info("chiffrefs: fill_super() appelé\n");

	sb->s_magic = CHIFFREFS_MAGIC;
	sb->s_op = &chiffrefs_super_ops;
	sb->s_blocksize = PAGE_SIZE;
	sb->s_blocksize_bits = PAGE_SHIFT;
	sb->s_fs_info = ctx;

	inode = new_inode(sb);
	if (!inode)
		return -ENOMEM;

	inode->i_ino = 1;
	inode->i_mode = S_IFDIR | 0755;
	inode->i_uid = GLOBAL_ROOT_UID;
	inode->i_gid = GLOBAL_ROOT_GID;

	inode->i_op = &chiffrefs_dir_inode_ops;
	inode->i_fop = &simple_dir_operations;

	sb->s_root = d_make_root(inode);

	if (!sb->s_root)
		return -ENOMEM;

	return 0;
}

/* --------------------------------------------------------- */
/* get_tree                                                    */
/* --------------------------------------------------------- */

static int chiffrefs_get_tree(struct fs_context *fc)
{
        struct chiffrefs_context *ctx = fc->fs_private;
        struct path source_path;
        int ret;

        if (!ctx || !ctx->source || ctx->source[0] == '\0')
                return -EINVAL;

        ret = kern_path(ctx->source, LOOKUP_FOLLOW, &source_path);
        if (ret)
                return ret;

        if (!d_is_dir(source_path.dentry)) {
                path_put(&source_path);
                return -ENOTDIR;
        }

        path_put(&source_path);

        pr_info("chiffrefs: get_tree() appelé\n");
        return get_tree_nodev(fc, chiffrefs_fill_super);
}

/* --------------------------------------------------------- */
/* Options de montage                                         */
/* --------------------------------------------------------- */

static int chiffrefs_parse_param(struct fs_context *fc,
                                 struct fs_parameter *param)
{
        struct chiffrefs_context *ctx = fc->fs_private;
        int ret;

        if (!ctx || !param || !param->key)
                return -EINVAL;

        if (strcmp(param->key, "source") == 0) {
                char *new_source;

                if (!param->string || param->string[0] == '\0')
                        return -EINVAL;

                new_source = kstrdup(param->string, GFP_KERNEL);
                if (!new_source)
                        return -ENOMEM;

                kfree(ctx->source);
                ctx->source = new_source;

                pr_info("chiffrefs: source=%s\n", ctx->source);
                return 0;
        }

        if (strcmp(param->key, "n") == 0) {
                if (!param->string)
                        return -EINVAL;

                ret = kstrtoint(param->string, 10, &ctx->n);
                if (ret)
                        return ret;

                ctx->n = ((ctx->n % 26) + 26) % 26;
                pr_info("chiffrefs: n=%d\n", ctx->n);
                return 0;
        }

        pr_err("chiffrefs: option inconnue: %s\n", param->key);
        return -EINVAL;
}

/* --------------------------------------------------------- */
/* fs_context                                                  */
/* --------------------------------------------------------- */

static const struct fs_context_operations chiffrefs_context_ops = {
	.parse_param = chiffrefs_parse_param,
	.get_tree = chiffrefs_get_tree,
};

static int chiffrefs_init_fs_context(struct fs_context *fc)
{
	struct chiffrefs_context *ctx;

	pr_info("chiffrefs: init_fs_context() appelé\n");

	ctx = kzalloc(sizeof(*ctx), GFP_KERNEL);

	if (!ctx)
		return -ENOMEM;

	ctx->n = 0;
	ctx->source = NULL;

	fc->fs_private = ctx;
	fc->ops = &chiffrefs_context_ops;

	return 0;
}

/* --------------------------------------------------------- */
/* démontage                                                   */
/* --------------------------------------------------------- */

static void chiffrefs_kill_sb(struct super_block *sb)
{
	struct chiffrefs_context *ctx = sb->s_fs_info;

	kill_anon_super(sb);

	if (ctx) {
		kfree(ctx->source);
		kfree(ctx);
	}
}

/* --------------------------------------------------------- */
/* filesystem                                                   */
/* --------------------------------------------------------- */

static struct file_system_type chiffrefs_fs_type = {
	.owner = THIS_MODULE,
	.name = CHIFFREFS_NAME,
	.init_fs_context = chiffrefs_init_fs_context,
	.kill_sb = chiffrefs_kill_sb,
	.fs_flags = FS_USERNS_MOUNT,
};

/* --------------------------------------------------------- */
/* module                                                       */
/* --------------------------------------------------------- */

static int __init chiffrefs_init(void)
{
	int ret;

	ret = register_filesystem(&chiffrefs_fs_type);

	if (ret) {
		pr_err("chiffrefs: impossible d'enregistrer le filesystem\n");
		return ret;
	}

	pr_info("chiffrefs: filesystem enregistré\n");

	return 0;
}

static void __exit chiffrefs_exit(void)
{
	unregister_filesystem(&chiffrefs_fs_type);

	pr_info("chiffrefs: filesystem désenregistré\n");
}

module_init(chiffrefs_init);
module_exit(chiffrefs_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("TP3");
MODULE_DESCRIPTION("TP3 - VFS chiffrefs");
MODULE_ALIAS_FS(CHIFFREFS_NAME);
