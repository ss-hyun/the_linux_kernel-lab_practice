/*
 * Kernel API lab
 * 
 * mem.c - Memory allocation in Linux
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/ctype.h>

MODULE_DESCRIPTION("Print memory");
MODULE_AUTHOR("SO2");
MODULE_LICENSE("GPL");

static char *mem;

static int mem_init(void)
{
	size_t i;
	
	mem = kmalloc(4096 * sizeof(*mem), GFP_KERNEL);
	if (mem == NULL)
		goto err_mem;
	
	pr_info("chars: ");
	for (i = 0; i < 4096; i++) {
		if (isalpha(mem[i]))
			printk(KERN_CONT "%c%u ", mem[i], i);
		else{
			printk(KERN_CONT "\n");
			printk("%c%u\n", mem[i], i);
		}
	}
	
	return 0;

err_mem:
	return -1;
}

static void mem_exit(void)
{
	kfree(mem);
}

module_init(mem_init);
module_exit(mem_exit);
