/*
 * Kernel API lab
 *
 * sched-spin.c: Sleeping in atomic context
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>

MODULE_DESCRIPTION("Sleep while atomic");
MODULE_AUTHOR("SO2");
MODULE_LICENSE("GPL");

static int sched_spin_init(void)
{
	spinlock_t lock;
	struct task_struct *task;
	
	spin_lock_init(&lock);

	pr_info("The pid of the current process %s is %u.\n", current->comm, current->pid);
	pr_info("current process state : %lu\n", current->state);
	pr_info("The pid of the parent process %s is %u.\n", current->parent->comm, current->parent->pid);
	pr_info("parent process state : %lu\n", current->parent->state);

	/* TODO 0: Use spin_lock to aquire the lock */
	//spin_lock(&lock);

	set_current_state(TASK_INTERRUPTIBLE);
	pr_info("After run set_cuurent_state(TASK_INTERRUPTIBLE)\n");
	pr_info("current process state : %lu\n", current->state);
	
	printk("<Process list>\nPID\t\tSTATE\tNAME\n");
	for_each_process(task){
		if (task->state)
			continue;
		printk("%u\t\t%lu\t%s\n", task->pid, task->state, task->comm);
	}
	/* Try to sleep for 5 seconds. */
	schedule_timeout(5 * HZ);
	pr_info("After run schedule_timeout(5 * HZ)\n");
	pr_info("current process state : %lu\n", current->state);

	/* TODO 0: Use spin_unlock to release the lock */
	//spin_unlock(&lock);

	return 0;
}

static void sched_spin_exit(void)
{
}

module_init(sched_spin_init);
module_exit(sched_spin_exit);
