/*
 * PSO - Memory Mapping Lab(#11)
 *
 * Exercise #2: memory mapping using vmalloc'd kernel areas
 */

#include <linux/version.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/sched.h>
#include <linux/sched/mm.h>
#include <linux/mm.h>
#include <asm/io.h>
#include <linux/uaccess.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

#include "../test/mmap-test.h"


MODULE_DESCRIPTION("simple mmap driver");
MODULE_AUTHOR("PSO");
MODULE_LICENSE("Dual BSD/GPL");

#define MY_MAJOR	42

/* how many pages do we actually vmalloc */
#define NPAGES		16

/* character device basic structure */
static struct cdev mmap_cdev;

/* pointer to the vmalloc'd area, rounded up to a page boundary */
static char *vmalloc_area;

static int my_open(struct inode *inode, struct file *filp)
{
	return 0;
}

static int my_release(struct inode *inode, struct file *filp)
{
	return 0;
}

static ssize_t my_read(struct file *file, char __user *user_buffer,
		size_t size, loff_t *offset)
{
	/* TODO 2: check size doesn't exceed our mapped area size */
	if (size > NPAGES * PAGE_SIZE)
		size = NPAGES * PAGE_SIZE;

	/* TODO 2: copy from mapped area to user buffer */
	if (copy_to_user(user_buffer, vmalloc_area, size))
    	return -EFAULT;	

	return size;
}

static ssize_t my_write(struct file *file, const char __user *user_buffer,
		size_t size, loff_t *offset)
{
	/* TODO 2: check size doesn't exceed our mapped area size */
	if (size > NPAGES * PAGE_SIZE)
		size = NPAGES * PAGE_SIZE;

	/* TODO 2: copy from user buffer to mapped area */
	if (copy_from_user(vmalloc_area, user_buffer, size))
		return -EFAULT;
	
	return size;
}

static int my_mmap(struct file *filp, struct vm_area_struct *vma)
{
	int ret ,i;
	long length = vma->vm_end - vma->vm_start;
	unsigned long end = vma->vm_end;
	char *vmalloc_area_ptr = vmalloc_area;
	unsigned long pfn;

	if (length > NPAGES * PAGE_SIZE)
		return -EIO;
	
	/* TODO 1: map pages individually */
	while (length) {
		ret = remap_pfn_range(vma, end - length, vmalloc_to_pfn((void *)vmalloc_area_ptr), 
								PAGE_SIZE, vma->vm_page_prot);	
		if (ret < 0) {
			pr_err("[my_mmap] could not map the address area.\n");
			return -EIO;
		}
		vmalloc_area_ptr += PAGE_SIZE;
		length -= PAGE_SIZE;
	}
	return 0;
}

static const struct file_operations mmap_fops = {
	.owner = THIS_MODULE,
	.open = my_open,
	.release = my_release,
	.mmap = my_mmap,
	.read = my_read,
	.write = my_write
};

static char* vma_name(struct vm_area_struct *vma_iter)
{
	if (vma_iter->vm_ops && vma_iter->vm_file)
		return vma_iter->vm_file->f_path.dentry->d_name.name;
		
	if (vma_iter->vm_mm->start_brk <= vma_iter->vm_start && vma_iter->vm_mm->brk >= vma_iter->vm_end)
		return "[ heap ]";
	
	if (vma_iter->vm_end >= vma_iter->vm_mm->start_stack && vma_iter->vm_mm->start_stack >= vma_iter->vm_start){
		return "[ stack ]";
	}

	return "[ anon ]";
}

static int my_seq_show(struct seq_file *seq, void *v)
{
	struct mm_struct *mm;
	struct vm_area_struct *vma_iterator;
	unsigned long total = 0, size;

	/* TODO 3: Get current process' mm_struct */
	mm = get_task_mm(current);

	/* TODO 3: Iterate through all memory mappings and print ranges */
	pr_info("[vm_start:vm_end]\tsize\tmode\tmapping\n");
	for (vma_iterator = mm->mmap; vma_iterator != NULL; vma_iterator = vma_iterator->vm_next) {
		total += size = vma_iterator->vm_end - vma_iterator->vm_start;
		pr_info("[%08lx:%08lx]\t%luK\t%c%c%c%c\t%s\n",
				 vma_iterator->vm_start, vma_iterator->vm_end,
				 size >> 10, 
				 vma_iterator->vm_flags&VM_READ?'r':'-', 
				 vma_iterator->vm_flags&VM_WRITE?'w':'-',
				 vma_iterator->vm_flags&VM_EXEC?'x':'-',
				 vma_iterator->vm_flags&VM_SHARED?'s':'-',
				 vma_name(vma_iterator));
	}
	pr_info("total\t%luK\t%s\n", total >> 10, current->comm);

	/* TODO 3: Release mm_struct */
	atomic_dec(&mm->mm_users);

	/* TODO 3: write the total count to file  */
	seq_printf(seq, "%lu", total);

	return 0;
}

static int my_seq_open(struct inode *inode, struct file *file)
{
	/* TODO 3: Register the display function */
	return single_open(file, my_seq_show, NULL);
}

static const struct proc_ops my_proc_ops = {
	.proc_open    = my_seq_open,
	.proc_read    = seq_read,
	.proc_lseek   = seq_lseek,
	.proc_release = single_release,
};

static int __init my_init(void)
{
	int ret = 0;
	int i;
	/* TODO 3: create a new entry in procfs */
	if (!proc_create(PROC_ENTRY_NAME, 0, NULL, &my_proc_ops)) {
		ret = -ENOMEM;
		goto out;
	}

	ret = register_chrdev_region(MKDEV(MY_MAJOR, 0), 1, "mymap");
	if (ret < 0) {
		pr_err("could not register region\n");
		goto out_no_chrdev;
	}

	/* TODO 1: allocate NPAGES using vmalloc */
	vmalloc_area = vmalloc(NPAGES * PAGE_SIZE);
	if (vmalloc_area == NULL) {
		ret = -ENOMEM;
		pr_err("[my_init] memory allocation failed.\n");
		goto out_unreg;
	}

	/* TODO 1: mark pages as reserved */
	for (i = 0; i < NPAGES * PAGE_SIZE; i += PAGE_SIZE){
		SetPageReserved(vmalloc_to_page((unsigned long)vmalloc_area + i));
	}

	/* TODO 1: write data in each page */
	for (i = 0; i < NPAGES * PAGE_SIZE; i += PAGE_SIZE) {
		vmalloc_area[i] = 0xaa;
		vmalloc_area[i + 1] = 0xbb;
		vmalloc_area[i + 2] = 0xcc;
		vmalloc_area[i + 3] = 0xdd;
	}

	cdev_init(&mmap_cdev, &mmap_fops);
	ret = cdev_add(&mmap_cdev, MKDEV(MY_MAJOR, 0), 1);
	if (ret < 0) {
		pr_err("could not add device\n");
		goto out_vfree;
	}

	return 0;

out_vfree:
	vfree(vmalloc_area);
out_unreg:
	unregister_chrdev_region(MKDEV(MY_MAJOR, 0), 1);
out_no_chrdev:
	remove_proc_entry(PROC_ENTRY_NAME, NULL);
out:
	return ret;
}

static void __exit my_exit(void)
{
	int i;

	cdev_del(&mmap_cdev);

	/* TODO 1: clear reservation on pages and free mem.*/
	for (i = 0; i < NPAGES * PAGE_SIZE; i += PAGE_SIZE)
		ClearPageReserved(vmalloc_to_page((unsigned long)vmalloc_area + i));
	vfree(vmalloc_area);

	unregister_chrdev_region(MKDEV(MY_MAJOR, 0), 1);
	/* TODO 3: remove proc entry */
	remove_proc_entry(PROC_ENTRY_NAME, NULL);
}

module_init(my_init);
module_exit(my_exit);
