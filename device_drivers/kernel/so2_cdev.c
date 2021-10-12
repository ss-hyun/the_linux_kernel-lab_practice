/*
 * Character device drivers lab
 *
 * All tasks
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#include <linux/wait.h>

#include "../include/so2_cdev.h"

MODULE_DESCRIPTION("SO2 character device");
MODULE_AUTHOR("SO2");
MODULE_LICENSE("GPL");

#define LOG_LEVEL	KERN_INFO

#define MY_MAJOR		42
#define MY_MINOR		0
#define NUM_MINORS		1
#define MODULE_NAME		"so2_cdev"
#define MESSAGE			"hello\n"
#define IOCTL_MESSAGE		"Hello ioctl"

#ifndef BUFSIZ
#define BUFSIZ		4096
#endif

struct so2_device_data {
	/* TODO 2: add cdev member */
	struct cdev cdev;
	/* TODO 4: add buffer with BUFSIZ elements */
	char read_buf[BUFSIZ];
	size_t size;
	/* TODO 7: extra members for home */
	wait_queue_head_t wq;
	int flag;
	char ioctl_buf[BUFFER_SIZE];
	/* TODO 3: add atomic_t access variable to keep track if file is opened */
	atomic_t access;
};

struct so2_device_data devs[NUM_MINORS];

static int so2_cdev_open(struct inode *inode, struct file *file)
{
	struct so2_device_data *data;

	/* TODO 2: print message when the device file is open. */
	pr_info("Device so_cdev opens.\n");

	/* TODO 3: inode->i_cdev contains our cdev struct,
	 use container_of to obtain a pointer to so2_device_data */
	data = container_of(inode->i_cdev, struct so2_device_data, cdev);

	file->private_data = data;

	/* TODO 3: return immediately if access is != 0, use atomic_cmpxchg */
	// if(atomic_cmpxchg(&data->access, 0, 1)){
	// 	return -EBUSY;
	// }
	atomic_inc(&data->access);

	// set_current_state(TASK_INTERRUPTIBLE);
	// schedule_timeout(10 * HZ);

	return 0;
}

static int so2_cdev_release(struct inode *inode, struct file *file)
{
	/* TODO 2: print message when the device file is closed. */
	pr_info("Device so_cdev closes.\n");

#ifndef EXTRA
	struct so2_device_data *data =
		(struct so2_device_data *) file->private_data;

	/* TODO 3: reset access variable to 0, use atomic_set */
	// atomic_set(&data->access, 0);
	atomic_dec(&data->access);

#endif
	return 0;
}

static ssize_t
so2_cdev_read(struct file *file,
		char __user *user_buffer,
		size_t size, loff_t *offset)
{
	struct so2_device_data *data = (struct so2_device_data *) file->private_data;
	size_t to_read;

#ifdef EXTRA
	/* TODO 7: extra tasks for home */
#endif
	/* TODO 4: Copy data->buffer to user_buffer, use copy_to_user */
	to_read = min(data->size - *offset, size);

	if (to_read <= 0)
		return 0;

	if (copy_to_user(user_buffer, data->read_buf + *offset, to_read))
    	return -EFAULT;

	*offset += to_read;
	return to_read;
}

static ssize_t
so2_cdev_write(struct file *file,
		const char __user *user_buffer,
		size_t size, loff_t *offset)
{
	struct so2_device_data *data = (struct so2_device_data *) file->private_data;

	/* TODO 5: copy user_buffer to data->buffer, use copy_from_user */
	if (copy_from_user(data->read_buf + *offset, user_buffer, size))
		return -EFAULT;
	data->size = size;

	/* TODO 7: extra tasks for home */

	*offset += size;
	return size;
}

static long
so2_cdev_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	struct so2_device_data *data =
		(struct so2_device_data *) file->private_data;
	int ret = 0;
	int remains;

	switch (cmd) {
	/* TODO 6: if cmd = MY_IOCTL_PRINT, display IOCTL_MESSAGE */
	case MY_IOCTL_PRINT:
		pr_info(IOCTL_MESSAGE);
		break;
	/* TODO 7: extra tasks, for home */
	case MY_IOCTL_SET_BUFFER:	// writing a message to the device
		if (copy_from_user(data->ioctl_buf, arg, BUFFER_SIZE))
			ret = -EFAULT;
		break;
	case MY_IOCTL_GET_BUFFER:	// read a message from device
		if (copy_to_user(arg, data->ioctl_buf, BUFFER_SIZE))
			ret = -EFAULT;
		break;
	case MY_IOCTL_DOWN:			// add the process to a queue
		data->flag = 0;
		wait_event_interruptible(data->wq, data->flag != 0);
		break;
	case MY_IOCTL_UP:			// remove the process from a queue
		data->flag = 1 ;
		wake_up_interruptible(&data->wq);
		break;
	default:
		ret = -EINVAL;
	}

	return ret;
}

static const struct file_operations so2_fops = {
	.owner = THIS_MODULE,
	/* TODO 2: add open and release functions */
	.open = so2_cdev_open,
	.release = so2_cdev_release,
	/* TODO 4: add read function */
	.read = so2_cdev_read,
	/* TODO 5: add write function */
	.write = so2_cdev_write,
	/* TODO 6: add ioctl function */
	.unlocked_ioctl = so2_cdev_ioctl,
};

static int so2_cdev_init(void)
{
	int err;
	int i;

	/* TODO 1: register char device region for MY_MAJOR and NUM_MINORS starting at MY_MINOR */
	err = register_chrdev_region(MKDEV(MY_MAJOR, MY_MINOR), NUM_MINORS, MODULE_NAME);

	if (err != 0){
		pr_info("[ERR %d] Registration failed.\n", err);
		return err;
	}
	pr_info("Registration success!\n");

	for (i = 0; i < NUM_MINORS; i++) {
#ifdef EXTRA
		/* TODO 7: extra tasks, for home */
#else
		/*TODO 4: initialize buffer with MESSAGE string */
		devs[i].size = strlen(MESSAGE);
		strncpy(devs[i].read_buf, MESSAGE, devs[i].size);
#endif
		/* TODO 7: extra tasks for home */
		init_waitqueue_head(&devs[i].wq);
		devs[i].flag = 1;
		strlcpy(devs[i].ioctl_buf, IOCTL_MESSAGE, BUFFER_SIZE);
		/* TODO 3: set access variable to 0, use atomic_set */
		atomic_set(&devs[i].access, 0);
		/* TODO 2: init and add cdev to kernel core */
		cdev_init(&devs[i].cdev, &so2_fops);
		cdev_add(&devs[i].cdev, MKDEV(MY_MAJOR, i), NUM_MINORS);
	}

	return 0;
}

static void so2_cdev_exit(void)
{
	int i;

	for (i = 0; i < NUM_MINORS; i++) {
		/* TODO 2: delete cdev from kernel core */
		cdev_del(&devs[i].cdev);
	}

	/* TODO 1: unregister char device region, for MY_MAJOR and NUM_MINORS starting at MY_MINOR */
	unregister_chrdev_region(MKDEV(MY_MAJOR, MY_MINOR), NUM_MINORS);
	pr_info("Unregistration success!\n");
}

module_init(so2_cdev_init);
module_exit(so2_cdev_exit);
