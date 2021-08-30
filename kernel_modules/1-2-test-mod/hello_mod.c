#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>

MODULE_DESCRIPTION("Simple module");
MODULE_AUTHOR("Kernel Hacker");
MODULE_LICENSE("GPL");

#define KERN_N_DISPLAY KERN_SOH "7"

static int my_hello_init(void)
{
	pr_debug("Hello!\n");
	printk(KERN_N_DISPLAY "The log level of this message is %s\n", KERN_N_DISPLAY);
	return 0;
}

static void hello_exit(void)
{
	pr_debug("Goodbye!\n");
}

module_init(my_hello_init);
module_exit(hello_exit);
