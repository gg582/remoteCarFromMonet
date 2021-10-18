/*
 * car.h -- definitions for the char module
 *
 */

#ifndef _CAR_H_
#define _CAR_H_


/*
 * Macros to help debugging
 */

#undef PDEBUG             /* undef it, just in case */
#ifdef CAR_DEBUG
#  ifdef __KERNEL__
     /* This one if debugging is on, and kernel space */
#    define PDEBUG(fmt, args...) printk( KERN_DEBUG "car: " fmt, ## args)
#  else
     /* This one for user space */
#    define PDEBUG(fmt, args...) fprintf(stderr, fmt, ## args)
#  endif
#else
#  define PDEBUG(fmt, args...) /* not debugging: nothing */
#endif

#undef PDEBUGG
#define PDEBUGG(fmt, args...) /* nothing: it's a placeholder */

#ifndef CAR_MAJOR
#define CAR_MAJOR 0   /* dynamic major by default */
#endif

#ifndef CAR_NR_DEVS
#define CAR_NR_DEVS 3   /* car0 through car3 */
#endif

#ifndef CAR_P_NR_DEVS
#define CAR_P_NR_DEVS 3  /* carpipe0 through carpipe3 */
#endif

#ifndef CAR_P_NR_MOTORS
#define CAR_P_NR_MOTORS 1  /* carpipe0 through carpipe3 */
#endif

/*
 * The bare device is a variable-length region of memory.
 * Use a linked list of indirect blocks.
 *
 * "car_dev->data" points to an array of pointers, each
 * pointer refers to a memory area of CAR_QUANTUM bytes.
 *
 * The array (quantum-set) is CAR_QSET long.
 */
#ifndef CAR_QUANTUM
#define CAR_QUANTUM 4000
#endif

#ifndef CAR_QSET
#define CAR_QSET    1000
#endif

/*
 * The pipe device is a simple circular buffer. Here its default size
 */
#ifndef CAR_P_BUFFER
#define CAR_P_BUFFER 4000
#endif

/*
 * Representation of car quantum sets.
 */
struct car_qset {
	void **data;
	struct car_qset *next;
};

struct car_dev {
	struct car_qset *data;  /* Pointer to first quantum set */
	int quantum;              /* the current quantum size */
	int qset;                 /* the current array size */
	unsigned long size;       /* amount of data stored here */
	unsigned int access_key;  /* used by caruid and carpriv */
	struct semaphore sem;     /* mutual exclusion semaphore     */
	struct cdev cdev;	  /* Char device structure		*/
};

/*
 * Split minors in two parts
 */
#define TYPE(minor)	(((minor) >> 4) & 0xf)	/* high nibble */
#define NUM(minor)	((minor) & 0xf)		/* low  nibble */

/*
 * The different configurable parameters
 */
extern int car_major;     /* main.c */
extern int car_nr_devs;
extern int car_quantum;
extern int car_qset;

extern int car_p_buffer;	/* pipe.c */

/*
 * Prototypes for shared functions
 */

int     car_p_init(dev_t dev);
int	car_motor_init(dev_t dev);

void    car_p_cleanup(void);
void	car_motor_cleanup(void);

int     car_access_init(dev_t dev);
void    car_access_cleanup(void);

int     car_trim(struct car_dev *dev);

ssize_t car_read(struct file *filp, char __user *buf, size_t count,
                   loff_t *f_pos);
ssize_t car_write(struct file *filp, const char __user *buf, size_t count,
                    loff_t *f_pos);
loff_t  car_llseek(struct file *filp, loff_t off, int whence);

#endif /* _CAR_H_ */
