#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/errno.h>
#include <linux/io.h>
#include <linux/fcntl.h>
#include <linux/mman.h>
#include <stdbool.h>
#include <linux/ktime.h>
#include <linux/interrupt.h>
#include <linux/mutex.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <linux/gpio.h>
#include <linux/types.h>
#include <asm/io.h>

#define GPIO_TRIG 				20
#define GPIO_ECHO				21// GPIO PIN 

dev_t 	dev = 0 ;
char 	* ECHO_DESC = "ECHO" ;

#define MUTEX_ENABLE	1

#if (MUTEX_ENABLE == 1 )
   #define MUTEX_INIT(x)			mutex_init(x);
   #define MUTEX_LOCK_INTERRUPTIBLE(x)		mutex_lock_interruptible(x);
   #define MUTEX_UNLOCK(x)			mutex_unlock(x);
#else
   #define MUTEX_INIT(x)			
   #define MUTEX_LOCK_INTERRUPTIBLE(x)		
   #define MUTEX_UNLOCK(x)			
#endif

static struct class * 	devClass ;
static struct cdev 	myCharDevice ;

static int  __init DeviceInit ( void ) ;
static void __exit DeviceExit ( void ) ;

// Device Functions 
static int 	DeviceOpen 	( struct inode * inode , struct file * file ) ;
static ssize_t 	DeviceRead 	( struct file * file , char __user * buf , size_t len , loff_t * off ) ;
static int 	DeviceRelease 	( struct inode * inode , struct file * file ) ;

static uint32_t measureDistance ( void );

// Match File Operation Functions into Structure 

struct mutex mutex4Trigger ;

static struct file_operations fOpStruct = {
	.owner = THIS_MODULE , 
	.read = DeviceRead ,
	.open = DeviceOpen ,
	.release = DeviceRelease
};

static int DeviceOpen ( struct inode * inode , struct file * file ) {

	printk ( "Device File Opened \n " ) ;

	return 0 ;
}

static int DeviceRelease ( struct inode * inode , struct file * file ) {

	printk ( "Device File Closed \n" ) ;

	return 0 ;
}

static ssize_t DeviceRead ( struct file * file , char __user * buf , size_t len , loff_t * off ) {	

	uint32_t  res;
	uint32_t  distance;

	distance = measureDistance ();

	res = copy_to_user ( buf , & distance , sizeof ( unsigned long ) ) ;

	if ( res > 0 ) {
		printk ( "ERROR : NOT ALL THE BYTES HAS BEEN COPIED TO USER \n" );
		return 0 ;
	} 

	return (ssize_t) distance;
}

static uint32_t measureDistance ( void ) {

	ssize_t 	ret = 0;
	uint32_t 	diffTime ;

	uint32_t 	counter = 0;
	const uint32_t 	COUNT_LIMIT = 1000000;

	uint32_t measuredData = 0 ;
	uint32_t startTime = 0 ;
	uint32_t endTime = 0 ;

	printk (KERN_DEBUG "%d", __LINE__);

	ret = MUTEX_LOCK_INTERRUPTIBLE ( & mutex4Trigger )

	counter = 0;
	gpio_set_value ( GPIO_TRIG , 0 ) ;

	udelay ( 10 ) ;
	gpio_set_value ( GPIO_TRIG , 1 ) ;
	udelay ( 10 ) ;
	gpio_set_value ( GPIO_TRIG , 0 ) ;

	while ( (counter ++ < COUNT_LIMIT ) && gpio_get_value ( GPIO_ECHO ) == 0 );

	startTime = ktime_to_ns ( ktime_get () );

	if ( counter >= COUNT_LIMIT ) {
		printk ("ERROR: echo not respond");
		MUTEX_UNLOCK ( & mutex4Trigger )
		return 0;
	}

	counter = 0;

	while ( (counter ++ < COUNT_LIMIT ) && gpio_get_value ( GPIO_ECHO ) == 1 );

	endTime = ktime_to_ns ( ktime_get () ) ;

	if ( counter >= COUNT_LIMIT ) {
		printk ("ERROR: echo not respond");
		MUTEX_UNLOCK ( & mutex4Trigger )
		return 0;
	}

	MUTEX_UNLOCK ( & mutex4Trigger )

	diffTime = ktime_to_ns ( ktime_sub ( endTime , startTime ) ) ;

	measuredData = diffTime*170l/10000000l ;

	return measuredData;
}

static int __init DeviceInit ( void ) {

        dev = MKDEV ( MAJOR ( dev ) , MINOR ( dev ) ) ;
        if ( ( alloc_chrdev_region ( & dev , 0 , 1 , "SR04" ) ) < 0 )  { 
                printk  ( "ERROR : CANNOT ALLOCATE MAJOR NUMBER \n" ) ;
                gpio_free ( GPIO_TRIG ) ;
                return ENOMEM ;
        }

        printk ( "Major = %d , Minor = %d \n" , MAJOR ( dev ) , MINOR ( dev ) ) ;
        if ( ( devClass = class_create ( THIS_MODULE , "sr04class" ) ) == NULL ) {
                printk ( "ERROR : CANNOT CREATE THE STRUCT CLASS \n" ) ;
                class_destroy ( devClass ) ;
                return ENOMEM ;
        }

        cdev_init ( & myCharDevice , & fOpStruct ) ;

        if ( cdev_add ( & myCharDevice , dev , 1 ) < 0 ) {
                printk ( "ERROR : CANNOT ADD THE DEVICE" ) ;
                cdev_del ( & myCharDevice ) ;
                return EEXIST ;
        }


        if ( ( device_create ( devClass , NULL , dev , NULL , "car/sr04" ) ) == NULL ) {
                printk ( "ERROR : CANNOT CREATE THE DEVICE \n" );
                device_destroy ( devClass , dev ) ; 
                return EEXIST ;
        }

        if ( ! ( gpio_is_valid ( GPIO_TRIG ) ) ) { 
                printk ( "ERROR: GPIO %d IS NOT VALID\n" , GPIO_TRIG ) ;
                device_destroy ( devClass , dev ) ;
                return EINVAL ;
        }

        if ( gpio_request ( GPIO_TRIG , NULL ) < 0 ) {
                printk ( "ERROR: CANNOT REQUEST GPIO %d\n", GPIO_TRIG ) ;
                gpio_free ( GPIO_TRIG ) ;
                return EINVAL ;
        }


        gpio_direction_output   ( GPIO_TRIG , 1 ) ;
	gpio_direction_input 	( GPIO_ECHO ) ;
	gpio_export ( GPIO_TRIG , false ) ;

        printk ( "Deviceconvert c Driver Insert : Success\n" ) ;
	MUTEX_INIT ( & mutex4Trigger )

        return 0 ;
}

static void __exit DeviceExit ( void ) {
   	gpio_free 	( GPIO_TRIG ) ;
        device_destroy 	( devClass , dev ) ;
        class_destroy 	( devClass ) ;
        cdev_del 	( & myCharDevice ) ;
        unregister_chrdev_region ( dev , 1 ) ;
        printk 		( "Device Driver Remove : Success\n" ) ;
}

module_init ( DeviceInit ) ;
module_exit ( DeviceExit ) ;

MODULE_LICENSE ( "GPL" ) ;



