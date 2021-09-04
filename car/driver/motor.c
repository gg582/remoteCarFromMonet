#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <linux/gpio.h>
#include <linux/kthread.h>
#include <linux/threads.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <asm/io.h>
#include <linux/sched.h>
#include <linux/smpboot.h>
#include <stdbool.h>

#include "../common/ioctl_car_cmd.h"

#define BUFSIZE				2 // Defines Read Buffer
#define WHEEL1 				18 // GPIO PIN 
#define WHEEL2 				23
#define WHEEL3 				24
#define WHEEL4 				25
#define ZEROCHAR 			48 // ZEROCHAR == '0'
#define HIGH				1
#define LOW					0
#define MAX_LEVEL			99
#define MAX_WHEEL_NUMBER		4

#define MAX_SPEED			99
#define MID_SPEED			40
#define MIN_SPEED			0

#define TIME_MARGIN			100
#define TIME_LAZY			1000

enum {
		WHEEL_IDX_1,
		WHEEL_IDX_2,
		WHEEL_IDX_3,
		WHEEL_IDX_4
};

dev_t		dev  = 0 ;
bool		flag = 1 ;
int32_t		level [MAX_WHEEL_NUMBER];

#define SET_LEVEL(wheel_index,levelValue)  do {level [wheel_index] = levelValue; } while (false)

int32_t			wheelPinNumber [MAX_WHEEL_NUMBER] = { WHEEL1, WHEEL2, WHEEL3, WHEEL4 };
static struct   task_struct *writerTask;
static struct 	mutex 	mutexMotor;


static struct 	class * devClass ;
static struct 	cdev myCharDevice ;

static int __init DeviceInit ( void ) ;
static void __exit DeviceExit ( void ) ;

static bool    GPIO_init    (  void );
static bool    createThread (  void );
static int32_t pinHandler   ( void* argument );

// Device Functions 
static int DeviceOpen ( struct inode * inode , struct file * file ) ;
static int DeviceRelease ( struct inode * inode , struct file * file ) ;

static void Forward ( void ) ;
static void Backward( void ) ;
static void Left    ( void ) ;
static void Right   ( void ) ;

static long chardevIoctl ( struct file * , unsigned int , unsigned long ) ;

static void pinWrite ( int ) ;

// Match File Operation Functions into Structure 
static struct file_operations fOpStruct = {
	.owner = THIS_MODULE , 
	.read  = NULL,
	.write = NULL,
	.open  = DeviceOpen ,
	.release = DeviceRelease ,
	.unlocked_ioctl = chardevIoctl
};

static int DeviceOpen ( struct inode * inode , struct file * file ) {
	printk ( "Device File Opened \n " ) ;
	return 0 ;
}

static int DeviceRelease ( struct inode * inode , struct file * file ) {
	printk ( "Device File Closed \n" )  ;
	return 0 ;
}

static int __init DeviceInit ( void )
{

	dev = MKDEV ( MAJOR ( dev ) , MINOR ( dev ) ) ;

	if ( ( alloc_chrdev_region (&dev, 0, 1, "motor" ) ) < 0 )  { 
		printk  ( KERN_INFO "ERROR : cannot allocate major number" ) ;
		return -1 ;
	}

	printk ( "Major = %d , Minor = %d \n" , MAJOR ( dev ) , MINOR ( dev ) ) ;

	cdev_init ( &myCharDevice , &fOpStruct ) ;
	
	if ( cdev_add ( & myCharDevice , dev , 1 ) < 0 ) {
		printk ( KERN_INFO "ERROR : cannot add the device") ;
		goto r_class;
	}

	if ( ( devClass = class_create ( THIS_MODULE , "motorclass" ) ) == NULL ) {
		printk ( KERN_INFO "ERROR : cannot create the struct class" ) ;
		goto r_class;
	}

	if ( ( device_create ( devClass , NULL , dev , NULL , "/car/motor" ) ) == NULL ) {
		printk ( "ERROR : CANNOT CREATE THE DEVICE" );
		goto r_device;
	}

	if ( !GPIO_init () ) goto r_device;

	if ( !createThread () ) goto r_device;

	printk ( KERN_INFO "motor is ready" );

	return 0;

r_device:
		class_destroy(devClass);

r_class:
		unregister_chrdev_region(dev,1);
		cdev_del ( & myCharDevice );

	return -1;
}

static bool GPIO_init (void )
{
	int i;

	for ( i = 0; i < MAX_WHEEL_NUMBER; i ++ ) 
	{
		if ( !(gpio_is_valid ( wheelPinNumber [i]) ) ) {
			printk ( "ERROR : GPIO %d IS NOT VALID" , WHEEL1 ) ;
			device_destroy ( devClass , dev ) ;
			return false;
		}
		
		if ( gpio_request ( wheelPinNumber [i], NULL ) < 0 ) {
			printk ( "ERROR : CANNOT REQUEST GPIO %d" , wheelPinNumber [i]) ;
			gpio_free( wheelPinNumber [i]);
			return false;
		}
		
		gpio_direction_output ( wheelPinNumber [i], 0 );

		gpio_export ( wheelPinNumber [i], false );

	}

	printk ( KERN_INFO "GPIO pins are ready" );

	return true;
}

static int32_t pinHandler ( void* argument )
{
	int idx;

	while(!kthread_should_stop()) {
		for ( idx = 0; idx < MAX_WHEEL_NUMBER; idx ++ ) pinWrite (idx); 
		udelay (TIME_LAZY);
	}

	return 0;
}

static bool createThread (void) 
{

	mutex_init ( &mutexMotor );

	writerTask = kthread_run ( pinHandler, NULL, "motor-control");

	if ( !writerTask ) {
		printk ( KERN_INFO "NOICE: errors in kthread creation");
		return false;
	} else {
		wake_up_process(writerTask);
	}

	printk ( KERN_INFO "NOICE: kthreads are ready");

    return true;
}

static void __exit DeviceExit () {
	int i;

	for ( i = 0; i < MAX_WHEEL_NUMBER; i ++ ) gpio_free	( wheelPinNumber [i]);

	printk (KERN_INFO "GPIO pins are released");

    device_destroy 	( devClass , dev ) ;
    class_destroy 	( devClass ) ;
    cdev_del 		( & myCharDevice ) ;

    unregister_chrdev_region ( dev , 1 ) ;

	printk (KERN_INFO "Devices are released");

	if ( writerTask ) kthread_stop ( writerTask );

    printk 	( "Device Driver Remove : Success" );
}

static void pinWrite ( int idx ) 
{
	int32_t range 		= MAX_LEVEL;
	int32_t pinNumber	= wheelPinNumber [idx];
	int32_t mark;
	int32_t space;
	int32_t ret;

	ret = mutex_lock_interruptible ( &mutexMotor );

	if ( level [ idx ] > range ) level [idx ] = range;
	
	mark  = level [ idx ];
	space = range - mark ;
	
	if (mark > 0) gpio_set_value (pinNumber, HIGH) ;
	
	udelay (mark * TIME_MARGIN);
	
	if (space > 0) gpio_set_value (pinNumber, LOW) ;
	
	udelay (space * TIME_MARGIN);

	mutex_unlock ( &mutexMotor );

}

static void Forward ( void )

{ 
	SET_LEVEL ( WHEEL_IDX_1, MAX_SPEED);
	SET_LEVEL ( WHEEL_IDX_2, MAX_SPEED);
	SET_LEVEL ( WHEEL_IDX_3, MIN_SPEED);
	SET_LEVEL ( WHEEL_IDX_4, MIN_SPEED);
}

static void Backward ( void )
{ 
	SET_LEVEL ( WHEEL_IDX_1, MIN_SPEED);
	SET_LEVEL ( WHEEL_IDX_2, MIN_SPEED);
	SET_LEVEL ( WHEEL_IDX_3, MAX_SPEED);
	SET_LEVEL ( WHEEL_IDX_4, MAX_SPEED);
}

// FAST TRUN LEFT
static void Left (void)
{
	SET_LEVEL ( WHEEL_IDX_1, MID_SPEED);
	SET_LEVEL ( WHEEL_IDX_2, MAX_SPEED);
	SET_LEVEL ( WHEEL_IDX_3, MAX_SPEED);
	SET_LEVEL ( WHEEL_IDX_4, MID_SPEED);
}

static void Right (void)
{
	SET_LEVEL ( WHEEL_IDX_1, MAX_SPEED);
	SET_LEVEL ( WHEEL_IDX_2, MID_SPEED);
	SET_LEVEL ( WHEEL_IDX_3, MID_SPEED);
	SET_LEVEL ( WHEEL_IDX_4, MAX_SPEED);
}

static void Stop (void)
{
	SET_LEVEL ( WHEEL_IDX_1, MIN_SPEED);
	SET_LEVEL ( WHEEL_IDX_2, MIN_SPEED);
	SET_LEVEL ( WHEEL_IDX_3, MIN_SPEED);
	SET_LEVEL ( WHEEL_IDX_4, MIN_SPEED);
}

static long chardevIoctl ( struct file * file , unsigned int command , unsigned long arg ) {
	switch ( command ) {
		case PI_CMD_LEFT    : Left (); 	   printk ( KERN_INFO "COMMAND: left"); 	break; 
		case PI_CMD_RIGHT   : Right(); 	   printk ( KERN_INFO "COMMAND: right");	break;
		case PI_CMD_FORWARD : Forward();   printk ( KERN_INFO "COMMAND: forward"); 	break;
		case PI_CMD_BACKWARD: Backward (); printk ( KERN_INFO "COMMAND: backward"); break;
		case PI_CMD_STOP	: Stop (); 	   printk ( KERN_INFO "COMMAND: stop"); 	break;
	}
	return command;
}

module_init ( DeviceInit ) ;
module_exit ( DeviceExit ) ;

MODULE_LICENSE ( "GPL" ) ;



