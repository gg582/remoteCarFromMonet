/*
 * main.c -- the bare car char module
 *
 * Copyright (C) 2001 Alessandro Rubini and Jonathan Corbet
 * Copyright (C) 2001 O'Reilly & Associates
 *
 * The source code in this file can be freely used, adapted,
 * and redistributed in source or binary form, so long as an
 * acknowledgment appears in derived source files.  The citation
 * should list that the code comes from the book "Linux Device
 * Drivers" by Alessandro Rubini and Jonathan Corbet, published
 * by O'Reilly & Associates.   No warranty is attached;
 * we cannot take responsibility for errors or fitness for use.
 *
 */

#include <linux/configfs.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>

#include <linux/kernel.h>	/* printk() */
#include <linux/slab.h>		/* kmalloc() */
#include <linux/fs.h>		/* everything... */
#include <linux/errno.h>	/* error codes */
#include <linux/types.h>	/* size_t */
#include <linux/proc_fs.h>
#include <linux/fcntl.h>	/* O_ACCMODE */
#include <linux/seq_file.h>
#include <linux/cdev.h>
#include <linux/semaphore.h>
//#include <asm/system.h>		/* cli(), *_flags */
#include <asm/uaccess.h>	/* copy_*_user */

#include "car.h"		/* local definitions */

/*
 * Our parameters which can be set at load time.
 */

int car_major =   CAR_MAJOR;
int car_minor =   0;
int car_nr_devs = CAR_NR_DEVS;	/* number of bare car devices */
int car_quantum = CAR_QUANTUM;
int car_qset =    CAR_QSET;

module_param(car_major, int, S_IRUGO);
module_param(car_minor, int, S_IRUGO);
module_param(car_nr_devs, int, S_IRUGO);
module_param(car_quantum, int, S_IRUGO);
module_param(car_qset, int, S_IRUGO);

MODULE_AUTHOR("Alessandro Rubini, Jonathan Corbet");
MODULE_LICENSE("Dual BSD/GPL");

struct car_dev *car_devices;	/* allocated in car_init_module */

/*
 * Empty out the car device; must be called with the device
 * semaphore held.
 */
int car_trim(struct car_dev *dev)
{
	struct car_qset *next, *dptr;
	int qset = dev->qset;   /* "dev" is not-null */
	int i;

	for (dptr = dev->data; dptr; dptr = next) { /* all the list items */
		if (dptr->data) {
			for (i = 0; i < qset; i++)
				kfree(dptr->data[i]);
			kfree(dptr->data);
			dptr->data = NULL;
		}
		next = dptr->next;
		kfree(dptr);
	}
	dev->size = 0;
	dev->quantum = car_quantum;
	dev->qset = car_qset;
	dev->data = NULL;
	return 0;
}

/*
 * Open and close
 */

int car_open(struct inode *inode, struct file *filp)
{
	struct car_dev *dev; /* device information */

	dev = container_of(inode->i_cdev, struct car_dev, cdev);
	filp->private_data = dev; /* for other methods */

	/* now trim to 0 the length of the device if open was write-only */
	if ( (filp->f_flags & O_ACCMODE) == O_WRONLY) {
		if (down_interruptible(&dev->sem))
			return -ERESTARTSYS;
		car_trim(dev); /* ignore errors */
		up(&dev->sem);
	}
	return 0;          /* success */
}

int car_release(struct inode *inode, struct file *filp)
{
	return 0;
}
/*
 * Follow the list
 */
struct car_qset *car_follow(struct car_dev *dev, int n)
{
	struct car_qset *qs = dev->data;

        /* Allocate first qset explicitly if need be */
	if (! qs) {
		qs = dev->data = kmalloc(sizeof(struct car_qset), GFP_KERNEL);
		if (qs == NULL)
			return NULL;  /* Never mind */
		memset(qs, 0, sizeof(struct car_qset));
	}

	/* Then follow the list */
	while (n--) {
		if (!qs->next) {
			qs->next = kmalloc(sizeof(struct car_qset), GFP_KERNEL);
			if (qs->next == NULL)
				return NULL;  /* Never mind */
			memset(qs->next, 0, sizeof(struct car_qset));
		}
		qs = qs->next;
		continue;
	}
	return qs;
}

/*
 * Data management: read and write
 */

ssize_t car_read(struct file *filp, char __user *buf, size_t count,
                loff_t *f_pos)
{
	struct car_dev *dev = filp->private_data; 
	struct car_qset *dptr;	/* the first listitem */
	int quantum = dev->quantum, qset = dev->qset;
	int itemsize = quantum * qset; /* how many bytes in the listitem */
	int item, s_pos, q_pos, rest;
	ssize_t retval = 0;

	if (down_interruptible(&dev->sem))
		return -ERESTARTSYS;
	if (*f_pos >= dev->size)
		goto out;
	if (*f_pos + count > dev->size)
		count = dev->size - *f_pos;

	/* find listitem, qset index, and offset in the quantum */
	item = (long)*f_pos / itemsize;
	rest = (long)*f_pos % itemsize;
	s_pos = rest / quantum; q_pos = rest % quantum;

	/* follow the list up to the right position (defined elsewhere) */
	dptr = car_follow(dev, item);

	if (dptr == NULL || !dptr->data || ! dptr->data[s_pos])
		goto out; /* don't fill holes */

	/* read only up to the end of this quantum */
	if (count > quantum - q_pos)
		count = quantum - q_pos;

	if (copy_to_user(buf, dptr->data[s_pos] + q_pos, count)) {
		retval = -EFAULT;
		goto out;
	}
	*f_pos += count;
	retval = count;

  out:
	up(&dev->sem);
	return retval;
}

ssize_t car_write(struct file *filp, const char __user *buf, size_t count,
                loff_t *f_pos)
{
	struct car_dev *dev = filp->private_data;
	struct car_qset *dptr;
	int quantum = dev->quantum, qset = dev->qset;
	int itemsize = quantum * qset;
	int item, s_pos, q_pos, rest;
	ssize_t retval = -ENOMEM; /* value used in "goto out" statements */

	if (down_interruptible(&dev->sem))
		return -ERESTARTSYS;

	/* find listitem, qset index and offset in the quantum */
	item = (long)*f_pos / itemsize;
	rest = (long)*f_pos % itemsize;
	s_pos = rest / quantum; q_pos = rest % quantum;

	/* follow the list up to the right position */
	dptr = car_follow(dev, item);
	if (dptr == NULL)
		goto out;
	if (!dptr->data) {
		dptr->data = kmalloc(qset * sizeof(char *), GFP_KERNEL);
		if (!dptr->data)
			goto out;
		memset(dptr->data, 0, qset * sizeof(char *));
	}
	if (!dptr->data[s_pos]) {
		dptr->data[s_pos] = kmalloc(quantum, GFP_KERNEL);
		if (!dptr->data[s_pos])
			goto out;
	}
	/* write only up to the end of this quantum */
	if (count > quantum - q_pos)
		count = quantum - q_pos;

	if (copy_from_user(dptr->data[s_pos]+q_pos, buf, count)) {
		retval = -EFAULT;
		goto out;
	}
	*f_pos += count;
	retval = count;

        /* update the size */
	if (dev->size < *f_pos)
		dev->size = *f_pos;

  out:
	up(&dev->sem);
	return retval;
}

/*
 * The "extended" operations -- only seek
 */

loff_t car_llseek(struct file *filp, loff_t off, int whence)
{
	struct car_dev *dev = filp->private_data;
	loff_t newpos;

	switch(whence) {
	  case 0: /* SEEK_SET */
		newpos = off;
		break;

	  case 1: /* SEEK_CUR */
		newpos = filp->f_pos + off;
		break;

	  case 2: /* SEEK_END */
		newpos = dev->size + off;
		break;

	  default: /* can't happen */
		return -EINVAL;
	}
	if (newpos < 0) return -EINVAL;
	filp->f_pos = newpos;
	return newpos;
}

struct file_operations car_fops = {
	.owner =    THIS_MODULE,
	.llseek =   car_llseek,
	.read =     car_read,
	.write =    car_write,
	.open =     car_open,
	.release =  car_release,
};

/*
 * Finally, the module stuff
 */

/*
 * The cleanup function is used to handle initialization failures as well.
 * Thefore, it must be careful to work correctly even if some of the items
 * have not been initialized
 */
void car_cleanup_module(void)
{
	int i;
	dev_t devno = MKDEV(car_major, car_minor);

	/* Get rid of our char dev entries */
	if (car_devices) {
		for (i = 0; i < car_nr_devs; i++) {
			car_trim(car_devices + i);
			cdev_del(&car_devices[i].cdev);
		}
		kfree(car_devices);
	}

	/* cleanup_module is never called if registering failed */
	unregister_chrdev_region(devno, car_nr_devs);

	/* and call the cleanup functions for friend devices */
	car_p_cleanup();
	car_motor_cleanup();
}

/*
 * Set up the char_dev structure for this device.
 */
static void car_setup_cdev(struct car_dev *dev, int index)
{
	int err, devno = MKDEV(car_major, car_minor + index);
    
	cdev_init(&dev->cdev, &car_fops);
	dev->cdev.owner = THIS_MODULE;
	dev->cdev.ops = &car_fops;
	err = cdev_add (&dev->cdev, devno, 1);
	/* Fail gracefully if need be */
	if (err)
		printk(KERN_NOTICE "Error %d adding car%d", err, index);
}


int car_init_module(void)
{
	int result, i;
	dev_t dev = 0;
/*
 * Get a range of minor numbers to work with, asking for a dynamic
 * major unless directed otherwise at load time.
 */
	if (car_major) {
		dev = MKDEV(car_major, car_minor);
		result = register_chrdev_region(dev, car_nr_devs, "car");
	} else {
		result = alloc_chrdev_region(&dev, car_minor, car_nr_devs, "car");
		car_major = MAJOR(dev);
	}
	if (result < 0) {
		printk(KERN_WARNING "car: can't get major %d\n", car_major);
		return result;
	}

        /* 
	 * allocate the devices -- we can't have them static, as the number
	 * can be specified at load time
	 */
	car_devices = kmalloc(car_nr_devs * sizeof(struct car_dev), GFP_KERNEL);
	if (!car_devices) {
		result = -ENOMEM;
		goto fail;  /* Make this more graceful */
	}
	memset(car_devices, 0, car_nr_devs * sizeof(struct car_dev));

        /* Initialize each device. */
	for (i = 0; i < car_nr_devs; i++) {
		car_devices[i].quantum = car_quantum;
		car_devices[i].qset = car_qset;
		sema_init(&car_devices[i].sem , 1 );
		car_setup_cdev(&car_devices[i], i);
	}

        /* At this point call the init function for creating the car_pipe devices */
	dev = MKDEV(car_major, car_minor + car_nr_devs);
	dev += car_p_init(dev);
	dev += car_motor_init(dev);

	return 0; /* succeed */

  fail:
	car_cleanup_module();
	return result;
}

module_init(car_init_module);
module_exit(car_cleanup_module);
