#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>	/* printk(), min() */
#include <linux/slab.h>		/* kmalloc() */
#include <linux/fs.h>		/* everything... */
#include <linux/proc_fs.h>
#include <linux/errno.h>	/* error codes */
#include <linux/types.h>	/* size_t */
#include <linux/fcntl.h>
#include <linux/poll.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>

#include "car.h"		/* local definitions */
#include "ioctl_car_cmd.h"

struct car_motor_tun {
        wait_queue_head_t inq, outq;       /* read and write queues */
        char *buffer, *end;                /* begin of buf, end of buf */
        int buffersize;                    /* used in pointer arithmetic */
        char *rp, *wp;                     /* where to read, where to write */
        int nreaders, nwriters;            /* number of openings for r/w */
        struct semaphore sem;              /* mutual exclusion semaphore */
        struct cdev cdev;                  /* Char device structure */
};

/* parameters */
static int car_motor_nr_devs = CAR_P_NR_MOTORS;	/* number of pipe devices */
int car_motor_buffer =  CAR_P_BUFFER;	/* buffer size */
dev_t car_motor_devno;			/* Our first device number */

module_param(car_motor_nr_devs, int, 0);	/* FIXME check perms */
module_param(car_motor_buffer, int, 0);

static struct car_motor_tun *car_motor_devices;

static int spacefree(struct car_motor_tun *dev);
/*
 * Open and close
 */
static int car_motor_open(struct inode *inode, struct file *filp)
{
	struct car_motor_tun *dev;

	dev = container_of(inode->i_cdev, struct car_motor_tun, cdev);
	filp->private_data = dev;

	if (down_interruptible(&dev->sem))
		return -ERESTARTSYS;
	if (!dev->buffer) {
		/* allocate the buffer */
		dev->buffer = kmalloc(car_motor_buffer, GFP_KERNEL);
		if (!dev->buffer) {
			up(&dev->sem);
			return -ENOMEM;
		}
	}
	dev->buffersize = car_motor_buffer;
	dev->end = dev->buffer + dev->buffersize;
	dev->rp = dev->wp = dev->buffer; /* rd and wr from the beginning */

	/* use f_mode,not  f_flags: it's cleaner (fs/open.c tells why) */
	if (filp->f_mode & FMODE_READ)
		dev->nreaders++;
	if (filp->f_mode & FMODE_WRITE)
		dev->nwriters++;
	up(&dev->sem);

	return nonseekable_open(inode, filp);
}

static int car_motor_release(struct inode *inode, struct file *filp)
{
	struct car_motor_tun *dev = filp->private_data;

	down(&dev->sem);
	if (filp->f_mode & FMODE_READ)
		dev->nreaders--;
	if (filp->f_mode & FMODE_WRITE)
		dev->nwriters--;
	if (dev->nreaders + dev->nwriters == 0) {
		kfree(dev->buffer);
		dev->buffer = NULL; /* the other fields are not checked on open */
	}
	up(&dev->sem);
	return 0;
}

/*
 * Data management: read and write
*/
static ssize_t car_motor_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
	struct car_motor_tun *dev = filp->private_data;

	if (down_interruptible(&dev->sem))
		return -ERESTARTSYS;

	PDEBUG("\" (car_motor_read) dev->wp:%p    dev->rp:%p\" \n",dev->wp,dev->rp);

	while (dev->rp == dev->wp) { /* nothing to read */
		up(&dev->sem); /* release the lock */
		if (filp->f_flags & O_NONBLOCK)
			return -EAGAIN;
		PDEBUG("\"%s\" reading: going to sleep\n", current->comm);
		if (wait_event_interruptible(dev->inq, (dev->rp != dev->wp)))
			return -ERESTARTSYS; /* signal: tell the fs layer to handle it */
		/* otherwise loop, but first reacquire the lock */
		if (down_interruptible(&dev->sem))
			return -ERESTARTSYS;
	}
	/* ok, data is there, return something */
	if (dev->wp > dev->rp)
		count = min(count, (size_t)(dev->wp - dev->rp));
	else /* the write pointer has wrapped, return data up to dev->end */
		count = min(count, (size_t)(dev->end - dev->rp));
	if (copy_to_user(buf, dev->rp, count)) {
		up (&dev->sem);
		return -EFAULT;
	}
	dev->rp += count;
	if (dev->rp == dev->end)
		dev->rp = dev->buffer; /* wrapped */
	up (&dev->sem);

	/* finally, awake any writers and return */
	wake_up_interruptible(&dev->outq);
	PDEBUG("\"%s\" did read %li bytes\n",current->comm, (long)count);
	return count;
}

/* Wait for space for writing; caller must hold device semaphore.  On
 * error the semaphore will be released before returning. */
static int car_getwritespace(struct car_motor_tun *dev, struct file *filp)
{
	while (spacefree(dev) == 0) { /* full */
		DEFINE_WAIT(wait);
		
		up(&dev->sem);
		if (filp->f_flags & O_NONBLOCK)
			return -EAGAIN;
		PDEBUG("\"%s\" writing: going to sleep\n",current->comm);
		prepare_to_wait(&dev->outq, &wait, TASK_INTERRUPTIBLE);
		if (spacefree(dev) == 0)
			schedule();
		finish_wait(&dev->outq, &wait);
		if (signal_pending(current))
			return -ERESTARTSYS; /* signal: tell the fs layer to handle it */
		if (down_interruptible(&dev->sem))
			return -ERESTARTSYS;
	}
	return 0;
}	

/* How much space is free? */
static int spacefree(struct car_motor_tun *dev)
{
	if (dev->rp == dev->wp)
		return dev->buffersize - 1;
	return ((dev->rp + dev->buffersize - dev->wp) % dev->buffersize) - 1;
}

static long car_motor_ioctl (struct file *filp, unsigned int command, unsigned long arg) 
{
	struct car_motor_tun *dev = filp->private_data;
	int result;
	size_t count;
       
	if (down_interruptible(&dev->sem))
		return -ERESTARTSYS;

	result = car_getwritespace(dev, filp);

	if (result)
		return result; /* car_getwritespace called up(&dev->sem) */

	PDEBUG("\"%s\" ioctl is called\n",current->comm);

	count = min(count, (size_t)spacefree(dev));

	if (dev->wp >= dev->rp)
		count = min(count, (size_t)(dev->end - dev->wp)); /* to end-of-buf */
	else 
		count = min(count, (size_t)(dev->rp - dev->wp - 1));

	if ( count > MAX_CMD_STR_LEN ) return -1;

	count = MAX_CMD_STR_LEN;

	switch (command) {
		case	PI_CMD_STOP	: memcpy ( dev->wp, STOP, MAX_CMD_STR_LEN); break;
		case	PI_CMD_LEFT 	: memcpy ( dev->wp, LEFT, MAX_CMD_STR_LEN); break;
		case	PI_CMD_RIGHT	: memcpy ( dev->wp, RIGHT, MAX_CMD_STR_LEN); break;
		case	PI_CMD_FORWARD	: memcpy ( dev->wp, FORWARD, MAX_CMD_STR_LEN); break;
		case	PI_CMD_BACKWARD	: memcpy ( dev->wp, BACKWARD, MAX_CMD_STR_LEN); break;
	}

	dev->wp += count;

	if (dev->wp == dev->end)
		dev->wp = dev->buffer; /* wrapped */

	up(&dev->sem);

	/* finally, awake any reader */
	wake_up_interruptible(&dev->inq);  /* blocked in read() and select() */

	PDEBUG("\"%s\" (ioctl --> return)\n",current->comm);

	return command;
}

static ssize_t car_motor_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
	struct car_motor_tun *dev = filp->private_data;
	int result;

	if (down_interruptible(&dev->sem))
		return -ERESTARTSYS;

	/* Make sure there's space to write */
	result = car_getwritespace(dev, filp);
	if (result)
		return result; /* car_getwritespace called up(&dev->sem) */

	/* ok, space is there, accept something */
	count = min(count, (size_t)spacefree(dev));
	if (dev->wp >= dev->rp)
		count = min(count, (size_t)(dev->end - dev->wp)); /* to end-of-buf */
	else /* the write pointer has wrapped, fill up to rp-1 */
		count = min(count, (size_t)(dev->rp - dev->wp - 1));
	PDEBUG("Going to accept %li bytes to %p from %p\n", (long)count, dev->wp, buf);
	if (copy_from_user(dev->wp, buf, count)) {
		up (&dev->sem);
		return -EFAULT;
	}
	dev->wp += count;
	if (dev->wp == dev->end)
		dev->wp = dev->buffer; /* wrapped */
	PDEBUG("\" (car_motor_write) dev->wp:%p    dev->rp:%p\" \n",dev->wp,dev->rp);
	up(&dev->sem);

	/* finally, awake any reader */
	wake_up_interruptible(&dev->inq);  /* blocked in read() and select() */

	PDEBUG("\"%s\" did write %li bytes in motor\n",current->comm, (long)count);
	return count;
}

static unsigned int car_motor_poll(struct file *filp, poll_table *wait)
{
	struct car_motor_tun *dev = filp->private_data;
	unsigned int mask = 0;

	/*
	 * The buffer is circular; it is considered full
	 * if "wp" is right behind "rp" and empty if the
	 * two are equal.
	 */
	down(&dev->sem);
	poll_wait(filp, &dev->inq,  wait);
	poll_wait(filp, &dev->outq, wait);
	if (dev->rp != dev->wp)
		mask |= POLLIN | POLLRDNORM;	/* readable */
	if (spacefree(dev))
		mask |= POLLOUT | POLLWRNORM;	/* writable */
	up(&dev->sem);
	return mask;
}

/*
 * The file operations for the pipe device
 * (some are overlayed with bare car)
 */
struct file_operations car_motor_tun_fops = {
	.owner =	THIS_MODULE,
	.llseek =	no_llseek,
	.read =		car_motor_read,
	.write =	car_motor_write,
	.poll =		car_motor_poll,
	.unlocked_ioctl = car_motor_ioctl,
	.open =		car_motor_open,
	.release =	car_motor_release,
};

/*
 * Set up a cdev entry.
 */
static void car_motor_setup_cdev(struct car_motor_tun *dev, int index)
{
	int err, devno = car_motor_devno + index;
    
	cdev_init(&dev->cdev, &car_motor_tun_fops);
	dev->cdev.owner = THIS_MODULE;
	err = cdev_add (&dev->cdev, devno, 1);
	/* Fail gracefully if need be */
	if (err)
		printk(KERN_NOTICE "Error %d adding car_motor %d", err, index);
}

/*
 * Initialize the pipe devs; return how many we did.
 */
int car_motor_init(dev_t firstdev)
{
	int i, result;

	result = register_chrdev_region(firstdev, car_motor_nr_devs, "car_motor");

	if (result < 0) {
		printk(KERN_NOTICE "Unable to get carp region, error %d\n", result);
		return 0;
	}

	car_motor_devno = firstdev;

	car_motor_devices = kmalloc(car_motor_nr_devs * sizeof(struct car_motor_tun), GFP_KERNEL);

	if (car_motor_devices == NULL) {
		unregister_chrdev_region(firstdev, car_motor_nr_devs);
		return 0;
	}

	memset(car_motor_devices, 0, car_motor_nr_devs * sizeof(struct car_motor_tun));

	for (i = 0; i < car_motor_nr_devs; i++) {
		init_waitqueue_head(&(car_motor_devices[i].inq));
		init_waitqueue_head(&(car_motor_devices[i].outq));
		sema_init(&car_motor_devices[i].sem , 1 );
		car_motor_setup_cdev(car_motor_devices + i, i);
	}
	return car_motor_nr_devs;
}

/*
 * This is called by cleanup_module or on failure.
 * It is required to never fail, even if nothing was initialized first
 */
void car_motor_cleanup(void)
{
	int i;

	if (!car_motor_devices)
		return; /* nothing else to release */

	for (i = 0; i < car_motor_nr_devs; i++) {
		cdev_del(&car_motor_devices[i].cdev);
		kfree(car_motor_devices[i].buffer);
	}
	kfree(car_motor_devices);
	unregister_chrdev_region(car_motor_devno, car_motor_nr_devs);
	car_motor_devices = NULL; /* pedantic */
}
