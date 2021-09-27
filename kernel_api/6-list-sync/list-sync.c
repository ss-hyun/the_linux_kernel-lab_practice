/*
 * Linux API lab
 *
 * list-sync.c - Synchronize access to a list
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/sched/signal.h>

MODULE_DESCRIPTION("Full list processing with synchronization");
MODULE_AUTHOR("SO2");
MODULE_LICENSE("GPL");

struct task_info {
	pid_t pid;
	unsigned long timestamp;
	atomic_t count;
	struct list_head list;
};

static struct list_head head;

/* TODO 1: you can use either a spinlock or rwlock, define it here */
DEFINE_SPINLOCK(lock);

static struct task_info *task_info_alloc(int pid)
{
	struct task_info *ti;

	ti = kmalloc(sizeof(*ti), GFP_KERNEL);
	if (ti == NULL)
		return NULL;
	ti->pid = pid;
	ti->timestamp = jiffies;
	atomic_set(&ti->count, 0);

	return ti;
}

static struct task_info *task_info_find_pid(int pid)
{
	struct list_head *p;
	struct task_info *ti;

	list_for_each(p, &head) {
		ti = list_entry(p, struct task_info, list);
		if (ti->pid == pid) {
			return ti;
		}
	}

	return NULL;
}

// void task_info_add_to_list(int pid)
static void task_info_add_to_list(int pid)
{
	struct task_info *ti;
	unsigned long jiff;
	
	/* TODO 1: Protect list, is this read or write access? */
	spin_lock(&lock);
	
	ti = task_info_find_pid(pid);
	if (ti != NULL) {
		ti->timestamp = jiffies;
		atomic_inc(&ti->count);
		/* TODO: Guess why this comment was added  here */
		spin_unlock(&lock);
		return;
	}

	/* TODO 1: critical section ends here */
	spin_unlock(&lock);

	ti = task_info_alloc(pid);

	/* TODO 1: protect list access, is this read or write access? */
	spin_lock(&lock);

	list_add(&ti->list, &head);
	/*
	list_add로는 추가되는 노드 수가 아주 적은 테스트 상황에서 spinlock의 효과를 확인하기 어렵다.
	때문에 같은 메모리에 동시에 접근하도록 해주는 테스트 모듈을 추가하고,
	list_add와 같은 작업을 하지만 각 단계 사이에 시간 지연을 추가한 코드를 사용하여
	spinlock의 효과를 확인할 수 있도록 해주었다.
	ti->list.next = head.next;
	jiff=jiffies;
	while(jiff+2!=jiffies);
	ti->list.prev = &head;
	jiff=jiffies;
	while(jiff+5!=jiffies);
	head.next->prev = &ti->list;
	jiff=jiffies;
	while(jiff+3!=jiffies);
	head.next = &ti->list;
	*/

	/* TODO 1: critical section ends here */
	spin_unlock(&lock);
}
//EXPORT_SYMBOL(task_info_add_to_list);

void task_info_add_for_current(void)
{
	task_info_add_to_list(current->pid);
	task_info_add_to_list(current->parent->pid);
	task_info_add_to_list(next_task(current)->pid);
	task_info_add_to_list(next_task(next_task(current))->pid);
}
/* TODO 2: Export the kernel symbol */
EXPORT_SYMBOL(task_info_add_for_current);

void task_info_print_list(const char *msg)
{
	struct list_head *p;
	struct task_info *ti;

	pr_info("%s: [ ", msg);

	/* TODO 1: Protect list, is this read or write access? */
	spin_lock(&lock);
	
	list_for_each(p, &head) {
		ti = list_entry(p, struct task_info, list);
		pr_info("(%d, %lu) ", ti->pid, ti->timestamp);
	}

	/* TODO 1: Critical section ends here */
	spin_unlock(&lock);

	pr_info("]\n");
}
/* TODO 2: Export the kernel symbol */
EXPORT_SYMBOL(task_info_print_list);

// void task_info_print_list_rev(const char *msg)
// {
// 	struct list_head *p;
// 	struct task_info *ti;

// 	pr_info("%s: [ ", msg);

// 	/* TODO 1: Protect list, is this read or write access? */
// 	spin_lock(&lock);
// 	for (p = (&head)->prev; p != (&head); p = p->prev)
// 	{
// 		ti = list_entry(p, struct task_info, list);
// 		pr_info("(%d, %lu) ", ti->pid, ti->timestamp);
// 	}

// 	/* TODO 1: Critical section ends here */
// 	spin_unlock(&lock);

// 	pr_info("]\n");
// }
// /* TODO 2: Export the kernel symbol */
// EXPORT_SYMBOL(task_info_print_list_rev);

void task_info_remove_expired(void)
{
	struct list_head *p, *q;
	struct task_info *ti;

	/* TODO 1: Protect list, is this read or write access? */
	spin_lock(&lock);

	list_for_each_safe(p, q, &head) {
		ti = list_entry(p, struct task_info, list);
		if (jiffies - ti->timestamp > 3 * HZ && atomic_read(&ti->count) < 5) {
			list_del(p);
			kfree(ti);
		}
	}
	/* TODO 1: Critical section ends here */
	spin_unlock(&lock);
}
/* TODO 2: Export the kernel symbol */
EXPORT_SYMBOL(task_info_remove_expired);

static void task_info_purge_list(void)
{
	struct list_head *p, *q;
	struct task_info *ti;

	/* TODO 1: Protect list, is this read or write access? */
	spin_lock(&lock);

	list_for_each_safe(p, q, &head) {
		ti = list_entry(p, struct task_info, list);
		list_del(p);
		kfree(ti);
	}

	/* TODO 1: Critical sections ends here */
	spin_unlock(&lock);
}

static int list_sync_init(void)
{	
	// struct task_info *ti;

	INIT_LIST_HEAD(&head);

	// ti = list_entry(head.prev, struct task_info, list);
	// printk("%px\n",ti);

	task_info_add_for_current();
	task_info_print_list("after first add");

	set_current_state(TASK_INTERRUPTIBLE);
	schedule_timeout(5 * HZ);
	
	return 0;
}

static void list_sync_exit(void)
{
	struct task_info *ti;

	if (head.prev != &head){
		ti = list_entry(head.prev, struct task_info, list);
		atomic_set(&ti->count, 10);
	}

	task_info_remove_expired();
	task_info_print_list("after removing expired");
	task_info_purge_list();
}

module_init(list_sync_init);
module_exit(list_sync_exit);
