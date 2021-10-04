#include <linux/delay.h>
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
#include <asm/io.h>

#define DEVICE_NUM 4

#define GPIOPIN_LEFT 16 // GPIO PIN 
#define GPIOPIN_RIGHT 12
#define BOTTOM_LEFT 8
#define BOTTOM_RIGHT 7
int deviceArray [ DEVICE_NUM ] = { GPIOPIN_LEFT , GPIOPIN_RIGHT , BOTTOM_LEFT , BOTTOM_RIGHT } ;
char  * names [ DEVICE_NUM ] = { "GPIOPIN_LEFT" , "GPIOPIN_RIGHT" , "BOTTOM_LEFT" , "BOTTOM_RIGHT" } ;
char * devnames [ DEVICE_NUM ] = { "car/ir0" , "car/ir1" , "car/ir2" , "car/ir3" } ;
int minorNumber [ DEVICE_NUM ] ;
char * classArr [ DEVICE_NUM ] = { "irClass1" , "irClass2" , "irClass3" , "irClass4" } ; 
char * region [ DEVICE_NUM ] = { "IR_REGION1" , "IR_REGION2" , "IR_REGION3" , "IR_REGION4" } ;
dev_t dev [ DEVICE_NUM ] ;
static struct class * devClass [ DEVICE_NUM ] ;
static struct cdev myCharDevice [ DEVICE_NUM ] ;

static int __init DeviceInit ( void ) ;
static void __exit DeviceExit ( void ) ;

// Device Function 
static int DeviceOpen ( struct inode * inode , struct file * file ) ;
static int DeviceRelease ( struct inode * inode , struct file * file ) ;
static ssize_t DeviceReadLeft ( struct file * file , char __user * buf , size_t len , loff_t * off ) ;
static ssize_t DeviceReadRight ( struct file * file , char __user * buf , size_t len , loff_t * off ) ;
static ssize_t DeviceReadBottomLeft ( struct file * file , char __user * buf , size_t len , loff_t * off ) ;
static ssize_t DeviceReadBottomRight ( struct file * file , char __user * buf , size_t len , loff_t * off ) ;
static ssize_t DeviceWrite ( struct file * file , const char __user * buf , size_t len , loff_t * off ) ; 
// Match File Operation Functions into Structure 
static struct file_operations fOpStruct [ DEVICE_NUM ] = {
	{
		.owner = THIS_MODULE , 
		.read = DeviceReadLeft ,
		.write = DeviceWrite ,
		.open = DeviceOpen ,
		.release = DeviceRelease
	} , 
	{
		.owner = THIS_MODULE , 
		.read = DeviceReadRight ,
		.write = DeviceWrite ,
		.open = DeviceOpen ,
		.release = DeviceRelease
	} ,   
	{
		.owner = THIS_MODULE , 
		.read = DeviceReadBottomLeft ,
		.write = DeviceWrite ,
		.open = DeviceOpen ,
		.release = DeviceRelease
	} ,
	{
		.owner = THIS_MODULE , 
		.read = DeviceReadBottomRight ,
		.write = DeviceWrite ,
		.open = DeviceOpen ,
		.release = DeviceRelease
	}  
};
static int DeviceOpen ( struct inode * inode , struct file * file ) {
	printk ( "Device File Opened \n " ) ;
	return 0 ;
}
static int DeviceRelease ( struct inode * inode , struct file * file ) {
	printk ( "Device File Closed \n" ) ;
	return 0 ;
}
static ssize_t DeviceReadLeft ( struct file * file , char __user * buf , size_t len , loff_t * off ) {
	uint32_t gpioState = gpio_get_value ( GPIOPIN_LEFT ) ;
	gpioState = ! gpioState ;
	if ( copy_to_user ( buf , & gpioState , sizeof ( uint32_t ) ) > 0 ) {
		printk ( "ERROR : NOT ALL THE BYTES HAS BEEN COPIED TO USER \n" );
	}
	printk ("Read Function : LEFTIR = %d \n" , gpioState ) ;

	return ( ssize_t ) ( gpioState ) ;
}
static ssize_t DeviceReadRight ( struct file * file , char __user * buf , size_t len , loff_t * off ) {
	uint32_t gpioState = gpio_get_value ( GPIOPIN_RIGHT ) ;
	gpioState = ! gpioState ;
	if ( copy_to_user ( buf , & gpioState , sizeof ( uint32_t ) ) > 0 ) {
		printk ( "ERROR : NOT ALL THE BYTES HAS BEEN COPIED TO USER \n" );
	}
	printk ("Read Function : RIGHTIR = %d \n" , gpioState ) ;

	return ( ssize_t ) ( gpioState ) ;
}
static ssize_t DeviceWrite ( struct file * file , const char __user * buf , size_t len , loff_t * off ) {
	return len ;
}
static ssize_t DeviceReadBottomLeft ( struct file * file , char __user * buf , size_t len , loff_t * off ) {
	ssize_t gpioState = gpio_get_value ( BOTTOM_LEFT ) ;
	gpioState = ! gpioState ;
	if ( copy_to_user ( buf , & gpioState , len ) > 0 ) {
		printk ( "ERROR : NOT ALL THE BYTES HAS BEEN COPIED TO USER \n" );
	}
	printk ("Read Function : BOTTOM_LEFTIR = %d \n" , gpioState ) ;

	return 0 ;
}
static ssize_t DeviceReadBottomRight ( struct file * file , char __user * buf , size_t len , loff_t * off ) {
	ssize_t gpioState = gpio_get_value ( BOTTOM_RIGHT ) ;
	gpioState = ! gpioState ;
	if ( copy_to_user ( buf , & gpioState , len ) > 0 ) {
		printk ( "ERROR : NOT ALL THE BYTES HAS BEEN COPIED TO USER \n" );
	}
	printk ("Read Function : BOTTOM_RIGHTIR = %d \n" , gpioState ) ;

	return 0 ;
}
static int __init DeviceInit ( void ) {
	int i ;
	for ( i = 0 ; i < DEVICE_NUM ; i ++ ) {
		dev [ i ]  = MKDEV ( MAJOR ( dev [ i ] ) , MINOR ( dev [ i ] ) ) ;
		minorNumber [ i ] = MINOR ( dev [ i ] )  ;
		if ( ( alloc_chrdev_region ( & dev [ i ] , 0 , 1 , region [ i ]  ) ) < 0 )  { 
			printk  ( "ERROR : CANNOT ALLOCATE MAJOR NUMBER \n" ) ;
			gpio_free ( deviceArray [ i ] ) ;
			return -1 ;
		}
	
		printk ( "Major = %d , Minor = %d \n" , MAJOR ( dev [ i ] ) , MINOR ( dev[ i ] ) ) ;
		if ( ( devClass [ i ] = class_create ( THIS_MODULE , classArr [ i ] ) ) == NULL ) {
			printk ( "ERROR : CANNOT CREATE THE STRUCT CLASS" ) ;
			class_destroy ( devClass [ i ] ) ;
			return -1 ;
		}
		cdev_init ( & myCharDevice [ i ] , & fOpStruct [ i ] ) ;
	
		if ( cdev_add ( & myCharDevice [ i ] , dev [ i ] , 1 ) < 0 ) {
			printk ( "ERROR : CANNOT ADD THE DEVICE" ) ;
			cdev_del ( & myCharDevice [ i ] ) ;
			return -1 ;
		}


		if ( ( device_create ( devClass [ i ] , NULL , dev [ i ] , NULL , devnames [ i ] ) ) == NULL ) {
			printk ( "ERROR : CANNOT CREATE THE DEVICE \n" );
		device_destroy ( devClass [ i ] , dev [ i ] ) ; 
		return -1 ;
		}

		if ( ! ( gpio_is_valid ( deviceArray [ i ] ) ) ) {
			printk ( "ERROR : GPIO %d IS NOT VALID\n" , deviceArray [ i ] ) ;
			device_destroy ( devClass [ i ] , dev [ i ] ) ;
			return -1 ;
		}
		
		if ( gpio_request ( deviceArray [ i ] , names [ i ] ) < 0 ) {
			printk ( "ERROR : CANNOT REQUEST GPIO %d\n" , deviceArray [ i ] ) ;
			gpio_free( deviceArray [ i ] );
			return -1 ;
		}
		gpio_direction_input ( deviceArray [ i ] ) ;
		gpio_export ( deviceArray [ i ] , false ) ;
	}	


	printk ( "Device Driver Insert : Success\n" ) ;
	return 0 ;
}

static void __exit DeviceExit ( void ) {
	int i ;
	for( i = 0 ; i < DEVICE_NUM ; i ++ ) {
		gpio_free ( deviceArray [ i ] ) ;
		device_destroy ( devClass [ i ] , dev [ i ] ) ;
		class_destroy ( devClass [ i ] ) ;
		cdev_del ( & myCharDevice [ i ] ) ;
		unregister_chrdev_region ( dev [ i ] , 1 ) ;
		printk ( "Device Driver Remove : Success\n" ) ;
	}
}

module_init ( DeviceInit ) ;
module_exit ( DeviceExit ) ;

MODULE_LICENSE ( "CraftX" ) ;



