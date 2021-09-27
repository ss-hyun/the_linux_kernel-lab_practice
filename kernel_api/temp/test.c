#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/slab.h>

MODULE_DESCRIPTION("Test module");
MODULE_AUTHOR("SO2");
MODULE_LICENSE("GPL");

// int c, d;
// extern void task_info_add_to_list(int pid);
// extern void task_info_print_list(const char *msg);

/*
  error list : memory 쓰기 오류 추정 (잘못된 접근)
- Kernel panic - not syncing: stack-protector: Kernel stack is corrupted in: list_sync_]
  ---[end Kernel panic – not syncing: stack-protector: Kernel stack is corrupted in: l-loop: module loaded
- initcall list_sync_init+0x0/0x329 [list_sync] returned with preemption imbalance
- BUG: unable to handle page fault for address: ffffffb8
  #PF: supervisor write access in kernel mode
  #PF: error_code(0x0002) - not-present page
  BUG: sleeping function called from invalid context at include/linux/cgroup-defs.h:753
  */

static int test_init(void)
{
	int a, b, *add, *add2, size, i;
	char *t;

	/*
	 module 6에서 task_info 노드 추가 시에 delay를 주면,
	 아래 코드를 사용하고 module 7과 이 test module을 동시에 loading할 때
	 race condition이 발생하는 것을 확인할 수 있다.
	*/
	// for(i=2;i<100;i++){
	// 	if(i%10==4)
	// 		task_info_add_to_list(i);
	// }

	/*
	add = &a;
	*add = 1;
	*(add - 1) = 2;
	size = &a - &b;
	printk("%d\n",size);
	*(add + 1) = 7;
	printk("%d %p %px %d %p %px\n",a, &a, &a, b, &b, &b);
	printk("%x %p %px\n", add, add, add);
	printk("%x\n", *(add - 1));
	//*(add + 8) = 8;
	printk("%x\n", *(add + 1));
	printk("%x\n", *(add + 2));
	printk("%x\n", *(add + 3));
	printk("%x\n", *(add + 8));
	printk("%x %x %x\n", &add, &add2, &size);
	add2 = &c;
	*add2 = 3;
	*(add2 - 1) = 4;
	*(add2 + 1) = 5;
	*(add2 + 20) = 5;
	printk("%d %p %px %d %p %px\n", c, &c, &c, d, &d, &d);
	printk("%x %p %px\n", add2, add2, add2);
	printk("%d\n", *(add2 - 1));
	printk("%d\n", *(add2 + 1));

	t = kmalloc(10*sizeof(*t), GFP_KERNEL);
	printk("before kfree t addr : 0x%px\n", t);
	printk("read string before t[i] initialization : %s\n", t);
	printk("t : ");
	for (i=0; i<10; i++){
		printk(KERN_CONT "%c ",t[i]);
		t[i] = 'a';
	}
	printk(KERN_CONT "\n");
	printk("read string : %s\n", t);
	t[i] = NULL;
	t[33] = 'e';
	printk("read char : t[33] - %c\n", t[33]);
	t[10000] = 'e';
	printk("read char : t[10000] - %c\n", t[10000]);
	printk("plus null at t[10], read string : %s\n", t);
	printk("read int : %d\n", t);
	printk("read char : %c\n", t);
	kfree(t);
	printk("after kfree t addr : 0x%px\n", t);
	printk("t[9] addr : 0x%px\n", t+9);
	// t[i-1] = NULL; // t[9]
	// upper line : error
	// Slab corruption (Tainted: G           O     ): kmalloc-32 start=c4574820, len=32
	// 000: 6b 6b 6b 6b 6b 6b 6b 6b 6b 00 6b 6b 6b 6b 6b 6b  kkkkkkkkk.kkkkkk
	// Prev obj: start=c4574800, len=32
	// 000: 6b 6b 6b 6b 6b 6b 6b 6b 6b 6b 6b 6b 6b 6b 6b 6b  kkkkkkkkkkkkkkkk
	// 010: 6b 6b 6b 6b 6b 6b 6b 6b 6b 6b 6b 6b 6b 6b 6b a5  kkkkkkkkkkkkkkk.
	// Next obj: start=c4574840, len=32
	// 000: 2e 6e 6f 74 65 2e 4c 69 6e 75 78 00 5a 5a 5a 5a  .note.Linux.ZZZZ
	// 010: 5a 5a 5a 5a 5a 5a 5a 5a 5a 5a 5a 5a 5a 5a 5a a5  ZZZZZZZZZZZZZZZ.
	
	printk("read string : %s\n", t);
	printk("read int : %d\n", t);
	printk("read char : t - %c, t[0] - %c\n", t, t[0]);
	printk("t : ");
	for (i=0; i<10; i++){
		printk(KERN_CONT "%c ",t[i]);
	}
	printk(KERN_CONT "\n");
	*/
	
	return 0;
}

static void test_exit(void)
{
}

module_init(test_init);
module_exit(test_exit);
