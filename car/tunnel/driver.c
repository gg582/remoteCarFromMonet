#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/string.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include "buffer.h"  // Include the provided buffer library


// Device names and regions
char *devnames[DEVICE_NUM] = { "car/left_ir", "car/right_ir", "car/sr04", "car/motor" };
char *region[DEVICE_NUM] = { "LEFT_IR_REGION", "RIGHT_IR_REGION", "SR04_REGION" , "MOTOR_REGION" };
char *classArr[DEVICE_NUM] = { "irClass1", "irClass2", "srClass", "motorClass" };

dev_t dev[DEVICE_NUM];  // Store device numbers

static struct class *devClass[DEVICE_NUM];  // Class structure for each device
static struct cdev myCharDevice[DEVICE_NUM];  // Character device structure

// Buffer and list structure for each device
list *deviceLists[DEVICE_NUM];  // List for each device buffer
node *deviceBuffers[DEVICE_NUM];  // Device-specific buffers

// Function declarations for initialization, exit, file operations, and buffer management
static int __init DeviceInit(void);
static void __exit DeviceExit(void);
static int DeviceOpen(struct inode *inode, struct file *file);
static int DeviceRelease(struct inode *inode, struct file *file);
static ssize_t DeviceRead(struct file *file, char __user *buf, size_t len, loff_t *off);
static ssize_t DeviceWrite(struct file *file, const char __user *buf, size_t len, loff_t *off);

// File operations structure for each device
static struct file_operations fOpStruct[DEVICE_NUM] = {
    {
        .owner = THIS_MODULE,
        .read = DeviceRead,   
        .write = DeviceWrite, 
        .open = DeviceOpen,   
        .release = DeviceRelease
    },
    {
        .owner = THIS_MODULE,
        .read = DeviceRead,
        .write = DeviceWrite,
        .open = DeviceOpen,
        .release = DeviceRelease
    },
    {
        .owner = THIS_MODULE,
        .read = DeviceRead,
        .write = DeviceWrite,
        .open = DeviceOpen,
        .release = DeviceRelease
    },
    {
        .owner = THIS_MODULE,
        .read = DeviceRead,
        .write = DeviceWrite,
        .open = DeviceOpen,
        .release = DeviceRelease
    }

};

/*----------------------------------------------
  Function: DeviceOpen
  Purpose: Handles opening of a device.
  Description: When a device is opened, this function is called. It logs the event and can be expanded to include locking or resource initialization.
  Returns: 0 if successful, otherwise error code.
----------------------------------------------*/
static int DeviceOpen(struct inode *inode, struct file *file) {
    printk("Device File Opened\n");
    return 0;
}

/*----------------------------------------------
  Function: DeviceRelease
  Purpose: Handles closing of a device.
  Description: When a device is released (closed), this function is invoked. It can be expanded to include cleanup or resource deallocation.
  Returns: 0 if successful, otherwise error code.
----------------------------------------------*/
static int DeviceRelease(struct inode *inode, struct file *file) {
    printk("Device File Closed\n");
    return 0;
}

/*----------------------------------------------
  Function: DeviceRead
  Purpose: Handles reading data from the device.
  Description: This function is called when a read operation is performed on the device. It dequeues a node from the device-specific buffer list and copies its data to the user-space buffer.
  Returns: The number of bytes read if successful, otherwise an error code.
----------------------------------------------*/
static ssize_t DeviceRead(struct file *file, char __user *buf, size_t len, loff_t *off) {
    dev_t t =  file->f_path.dentry->d_inode->i_rdev;
    int deviceIndex = MAJOR(dev[0]) - MAJOR(t);
    printk("Device Read mode Opened\n");
    if (deviceLists[deviceIndex] == NULL) {
        printk("ERROR: deviceLists[%d] allocation failed\n", deviceIndex);
        return -ENOMEM;
    }

    if(size(deviceLists[deviceIndex]) == 0)  {
        printk("ERROR: List is empty\n");
        return -1;  // Return error if the list is empty
    }
    node * currentNode = dequeue(deviceLists[deviceIndex]);

    printk("Read data is %s", currentNode->key);


    // Copy data from the node to user space
    if (copy_to_user(buf, currentNode->key, PREFIX)) {
        printk("ERROR: Failed to copy data to user space\n");
        return -EFAULT;
    }

    kfree(currentNode);

    // Free the dequeued node after reading
    
    return PREFIX;  // Return the length of the data read
}

/*----------------------------------------------
  Function: DeviceWrite
  Purpose: Handles writing data to the device.
  Description: This function is called when a write operation is performed on the device. It copies data from the user-space buffer to the kernel buffer and enqueues it into the device-specific list.
  Returns: The number of bytes written if successful, otherwise an error code.
----------------------------------------------*/
static ssize_t DeviceWrite(struct file *file, const char __user *buf, size_t len, loff_t *off) {
    dev_t t =  file->f_path.dentry->d_inode->i_rdev;
    int deviceIndex = MAJOR(dev[0]) - MAJOR(t);
    // Copy data from user space to kernel space
    char * kernelBuf = kmalloc(PREFIX, GFP_KERNEL);
    memset(kernelBuf,0,PREFIX);
    printk("Device Write mode Opened\n");
    if (copy_from_user(kernelBuf, buf, len)) {
        printk("ERROR: Failed to copy data from user space\n");
        kfree(kernelBuf);
        return -EFAULT;  // Return error if copy fails
    }

    printk("Write data is %s\n", kernelBuf);

    if (deviceLists[deviceIndex] == NULL) {
        printk("ERROR: deviceLists[%d] allocation failed\n", deviceIndex);
        return -ENOMEM;
    }

    if (enqueue(deviceLists[deviceIndex], kernelBuf) < 0) {
        printk("ERROR: Enqueue failed\n");
        return -ENOMEM;  // Return error if enqueue fails
    }
    return len;  // Return the length of the data written
}

/*----------------------------------------------
  Function: DeviceInit
  Purpose: Initializes the devices.
  Description: This function is called when the module is loaded. It allocates device numbers, creates device classes, initializes character devices, and sets up the device-specific buffer lists.
  Returns: 0 on success, negative error code on failure.
----------------------------------------------*/
static int __init DeviceInit(void) {
    int i;

    for (i = 0; i < DEVICE_NUM; i++) {
        // Allocate device number dynamically (using 0 for major number)
        dev[i] = MKDEV(0, i);
        if ((alloc_chrdev_region(&dev[i], 0, 1, region[i])) < 0) {
            printk("ERROR: Cannot allocate major number\n");
            return -1;
        }

        printk("device index %d, Major = %d, Minor = %d\n", i,  MAJOR(dev[i]), MINOR(dev[i]));  // Log major and minor numbers

        // Create class for each device
        if ((devClass[i] = class_create(THIS_MODULE, classArr[i])) == NULL) {
            printk("ERROR: Cannot create class\n");
            class_destroy(devClass[i]);
            return -1;
        }

        // Initialize and add the character device
        cdev_init(&myCharDevice[i], &fOpStruct[i]);
        if (cdev_add(&myCharDevice[i], dev[i], 1) < 0) {
            printk("ERROR: Cannot add the device\n");
            cdev_del(&myCharDevice[i]);
            return -1;
        }

        // Create the device file
        if (device_create(devClass[i], NULL, dev[i], NULL, devnames[i]) == NULL) {
            printk("ERROR: Cannot create device file\n");
            device_destroy(devClass[i], dev[i]);
            return -1;
        }

        // Initialize the list for the device
        deviceLists[i] = kmalloc(sizeof(list), GFP_KERNEL);
        if (!deviceLists[i]) {
            printk("ERROR: Memory allocation failed for device list\n");
            return -ENOMEM;
        }

        init_list(deviceLists[i]);  // Initialize the list for the device
    }

    printk("Device Driver Inserted: Success\n");
    return 0;
}

/*----------------------------------------------
  Function: DeviceExit
  Purpose: Cleans up the devices.
  Description: This function is called when the module is unloaded. It removes device files, destroys device classes, deletes character devices, and frees the device-specific buffer lists.
  Returns: None.
----------------------------------------------*/
static void __exit DeviceExit(void) {
    int i;

    for (i = 0; i < DEVICE_NUM; i++) {
        device_destroy(devClass[i], dev[i]);  // Destroy the device file
        class_destroy(devClass[i]);  // Destroy the class
        cdev_del(&myCharDevice[i]);  // Delete the character device
        unregister_chrdev_region(dev[i], 1);  // Unregister the device number

        // Free the list and its nodes
        free_list(deviceLists[i]);

        printk("Device Driver Removed: Success\n");
    }
}

module_init(DeviceInit);  // Call the initialization function
module_exit(DeviceExit);  // Call the exit function

MODULE_LICENSE("GPL");  // Declare the license as GPL

