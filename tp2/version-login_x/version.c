// SPDX-License-Identifier: GPL-2.0

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/string.h>

#include "version.h"

#define DEVICE_NAME "version"
#define MAX_VERSION_LEN 256

static char original_version[MAX_VERSION_LEN];
static char current_version[MAX_VERSION_LEN];

static int version_open(struct inode *inode, struct file *file)
{
    return 0;
}

static ssize_t version_read(struct file *file,
                            char __user *buf,
                            size_t count,
                            loff_t *ppos)
{
    return simple_read_from_buffer(
        buf,
        count,
        ppos,
        current_version,
        strlen(current_version)
    );
}

static ssize_t version_write(struct file *file,
                             const char __user *buf,
                             size_t count,
                             loff_t *ppos)
{
    size_t len = min(count, (size_t)(MAX_VERSION_LEN - 1));

    if (copy_from_user(current_version, buf, len))
        return -EFAULT;

    current_version[len] = '\0';

    return count;
}

static long version_ioctl(struct file *file,
                          unsigned int cmd,
                          unsigned long arg)
{
    switch (cmd) {

    case VERSION_MODIFIED:
        if (strcmp(current_version, original_version) != 0)
            return 1;

        return 0;

    case VERSION_RESET:
        strscpy(current_version,
                original_version,
                MAX_VERSION_LEN);
        return 0;

    default:
        return -ENOTTY;
    }
}

static const struct file_operations version_fops = {
    .owner = THIS_MODULE,
    .open = version_open,
    .read = version_read,
    .write = version_write,
    .unlocked_ioctl = version_ioctl,
    .llseek = noop_llseek,
};

static struct miscdevice version_device = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = DEVICE_NAME,
    .fops = &version_fops,
    .mode = 0666,
};

static int __init version_init(void)
{
    struct file *file;
    loff_t pos = 0;
    ssize_t ret;

    file = filp_open("/proc/sys/kernel/version",
                     O_RDONLY,
                     0);

    if (IS_ERR(file)) {
        pr_err("version: impossible d'ouvrir /proc/sys/kernel/version\n");
        return PTR_ERR(file);
    }

    ret = kernel_read(file,
                      original_version,
                      MAX_VERSION_LEN - 1,
                      &pos);

    filp_close(file, NULL);

    if (ret < 0)
        return ret;

    original_version[ret] = '\0';

    strscpy(current_version,
            original_version,
            MAX_VERSION_LEN);

    ret = misc_register(&version_device);

    if (ret) {
        pr_err("version: erreur misc_register\n");
        return ret;
    }

    pr_info("version: module chargé\n");

    return 0;
}

static void __exit version_exit(void)
{
    misc_deregister(&version_device);
    pr_info("version: module déchargé\n");
}

module_init(version_init);
module_exit(version_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("TP2");
MODULE_DESCRIPTION("Module misc /dev/version");
