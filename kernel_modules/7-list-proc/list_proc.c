#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
/* TODO: add missing headers */
#include <linux/sched.h>
#include <linux/sched/signal.h>

MODULE_DESCRIPTION("List current processes");
MODULE_AUTHOR("Kernel Hacker");
MODULE_LICENSE("GPL");

static int my_proc_init(void)
{
	struct task_struct *task = current;

	/* TODO: print current process pid and its name */
	pr_info("The pid of the current process %s is %u.\n", task->comm, task->pid);
	printk("[virtual memory areas of the current process]\nstart : 0x%lx\nend : 0x%lx\n",task->mm->mmap->vm_start,task->mm->mmap->vm_end);
	//printk("[virtual memory areas of the current process]\nstart : 0x%lx\nend : 0x%lx\n",task->active_mm->mmap->vm_start,task->active_mm->mmap->vm_end);

	/* TODO: print the pid and name of all processes */
	printk("[Process list]\nPID\t\tNAME\n");
	for_each_process(task){
		printk("%u\t\t%s\n", task->pid, task->comm);
	}

	return 0;
}

static void my_proc_exit(void)
{
	/* TODO: print current process pid and name */
	struct task_struct *task = current;
	pr_info("The pid of the current process %s is %u.\n", task->comm, task->pid);
}

module_init(my_proc_init);
module_exit(my_proc_exit);
