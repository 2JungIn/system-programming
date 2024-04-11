#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

static int hello_init(void);
static void hello_exit(void);


static int hello_init(void)
{
    printk(KERN_INFO "hello kernel!\n");
    
    return 0;
}

static void hello_exit(void)
{
    printk(KERN_INFO "bye kernel!\n");
}

module_init( hello_init );
module_exit( hello_exit );

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("hello module");
MODULE_AUTHOR("User");