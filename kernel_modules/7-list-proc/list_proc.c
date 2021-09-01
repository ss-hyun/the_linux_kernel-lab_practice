#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
/* TODO: add missing headers */
#include <linux/sched/signal.h>
//#include <linux/mm.h>

#define flag(c,f) (vm->vm_flags&(f)?(c):'-')

MODULE_DESCRIPTION("List current processes");
MODULE_AUTHOR("Kernel Hacker");
MODULE_LICENSE("GPL");

static int my_proc_init(void)
{
	struct task_struct *task;
	struct vm_area_struct *vm;
	
	/* TODO: print current process pid and its name */
	pr_info("The pid of the current process %s is %u.\n", current->comm, current->pid);
	printk("<virtual memory areas of the current process>\nstart-end\n");
	for (vm = current->mm->mmap; vm ; vm = vm->vm_next){
		printk("0x%08lx-0x%08lx\n", vm->vm_start, vm->vm_end);
		//printk("0x%08lx-0x%08lx\t%c%c%c%c\t%s\n", vm->vm_start, vm->vm_end, flag('r',VM_READ), flag('w',VM_WRITE), flag('x',VM_EXEC), flag('p',VM_SHARED));
	}

	/* TODO: print the pid and name of all processes */
	printk("<Process list>\nPID\t\tNAME\n");
	for_each_process(task){
		printk("%u\t\t%s\n", task->pid, task->comm);
	}
	
	return 0;
}

static void my_proc_exit(void)
{
	/* TODO: print current process pid and name */
	pr_info("The pid of the current process %s is %u.\n", current->comm, current->pid);
}

module_init(my_proc_init);
module_exit(my_proc_exit);
