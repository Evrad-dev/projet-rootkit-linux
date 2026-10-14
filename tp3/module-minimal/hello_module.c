#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Groupe Projet Noyau Linux");
MODULE_DESCRIPTION("Module pedagogique minimal illustrant le cycle de vie d'un LKM");
MODULE_VERSION("1.0");

static int __init hello_module_init(void)
{
    pr_info("hello_module: initialisation reussie\n");
    return 0;
}

static void __exit hello_module_exit(void)
{
    pr_info("hello_module: nettoyage termine\n");
}

module_init(hello_module_init);
module_exit(hello_module_exit);
