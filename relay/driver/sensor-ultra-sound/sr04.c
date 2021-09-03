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

#define GPIO_TRIG 		20
#define GPIO_ECHO		21// GPIO PIN 

dev_t 	dev = 0 ;
char 	* ECHO_DESC = "ECHO" ;

volatile unsigned long measuredData = 0 ;
volatile unsigned long startTime = 0 ;
volatile unsigned long endTime = 0 ;

static struct class * 	devClass ;
static struct cdev 	myCharDevice ;

static int  __init DeviceInit ( void ) ;
static void __exit DeviceExit ( void ) ;

// Device Functions 
static int 	DeviceOpen 	( struct inode * inode , struct file * file ) ;
static ssize_t 	DeviceRead 	( struct file * file , char __user * buf , size_t len , loff_t * off ) ;
static ssize_t 	DeviceWrite 	( struct file * file , const char __user * buf , size_t len , loff_t * off ) ; 
static int 	DeviceRelease 	( struct inode * inode , struct file * file ) ;

// Match File Operation Functions into Structure 

struct mutex mutex4Trigger ;

static struct file_operations fOpStruct = {
	.owner = THIS_MODULE , 
	.read = DeviceRead ,
	.write = DeviceWrite ,
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

	uint32_t res = 0 ;

	unsigned long distance = 0 ;

	distance = measuredData ;

	res = copy_to_user ( buf , & distance , sizeof ( unsigned long ) ) ;
	if ( res > 0 ) {
		printk ( "ERROR : NOT ALL THE BYTES HAS BEEN COPIED TO USER \n" );
		return -1 ;
	} 
	return len ;
}
static ssize_t DeviceWrite ( struct file * file , const char __user * buf , size_t len , loff_t * off ) {

	uint32_t ret = 0;
	unsigned long diffTime ;

	ret = mutex_lock_interruptible ( & mutex4Trigger ) ;

	gpio_set_value ( GPIO_TRIG , 0 ) ;
	udelay ( 10 ) ;
	gpio_set_value ( GPIO_TRIG , 1 ) ;
	udelay ( 10 ) ;
	gpio_set_value ( GPIO_TRIG , 0 ) ;

	while ( gpio_get_value ( GPIO_ECHO ) == 0 );

	startTime = ktime_to_ns ( ktime_get () ) ;

	while ( gpio_get_value ( GPIO_ECHO ) == 1 );

	endTime = ktime_to_ns ( ktime_get () ) ;

	mutex_unlock ( & mutex4Trigger ) ;

	diffTime = ktime_to_ns ( ktime_sub ( endTime , startTime ) ) ;

	printk ( "startTime : %lu , endTime : %lu, diffTime : %lu \n" , startTime , endTime, diffTime ) ;

	measuredData = diffTime*170l/10000000l ;

	return ret ;
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
	gpio_export ( GPIO_TRIG , false ) ;

        printk ( "Deviceconvert c Driver Insert : Success\n" ) ;
	mutex_init ( & mutex4Trigger ) ;
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
