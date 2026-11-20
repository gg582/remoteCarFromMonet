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
#include <linux/mutex.h>
#include <linux/err.h>
#include <linux/version.h>
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
static struct mutex deviceLocks[DEVICE_NUM];  // Protects each device list

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
    int i;
    for (i = 0; i < DEVICE_NUM; i++) {
        if (inode->i_rdev == dev[i]) {
            file->private_data = (void *)(long)i;
            printk("Device File Opened\n");
            return 0;
        }
    }
    return -ENODEV;
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
    int deviceIndex = (int)(long)file->private_data;
    node *currentNode;
    size_t copyLen;
    ssize_t ret;

    printk("Device Read mode Opened\n");
    if (deviceLists[deviceIndex] == NULL) {
        printk("ERROR: deviceLists[%d] allocation failed\n", deviceIndex);
        return -ENOMEM;
    }

    mutex_lock(&deviceLocks[deviceIndex]);
    if (size(deviceLists[deviceIndex]) == 0)  {
        mutex_unlock(&deviceLocks[deviceIndex]);
        return -EAGAIN;  // Nothing queued yet
    }
    currentNode = dequeue(deviceLists[deviceIndex]);
    mutex_unlock(&deviceLocks[deviceIndex]);

    if (currentNode == NULL) {
        return -EAGAIN;
    }

    // Never copy more than the caller's buffer or the stored data
    copyLen = min(len, currentNode->len);

    // Copy data from the node to user space
    if (copy_to_user(buf, currentNode->key, copyLen)) {
        printk("ERROR: Failed to copy data to user space\n");
        ret = -EFAULT;
    } else {
        ret = copyLen;  // Return the length of the data read
    }

    // Free the dequeued node and its data after reading
    kfree(currentNode->key);
    kfree(currentNode);
    return ret;
}

/*----------------------------------------------
  Function: DeviceWrite
  Purpose: Handles writing data to the device.
  Description: This function is called when a write operation is performed on the device. It copies data from the user-space buffer to the kernel buffer and enqueues it into the device-specific list.
  Returns: The number of bytes written if successful, otherwise an error code.
----------------------------------------------*/
static ssize_t DeviceWrite(struct file *file, const char __user *buf, size_t len, loff_t *off) {
    int deviceIndex = (int)(long)file->private_data;
    char *kernelBuf;

    printk("Device Write mode Opened\n");
    if (deviceLists[deviceIndex] == NULL) {
        printk("ERROR: deviceLists[%d] allocation failed\n", deviceIndex);
        return -ENOMEM;
    }

    // Keep one byte for the terminating NUL; longer writes are truncated
    if (len > PREFIX - 1) {
        len = PREFIX - 1;
    }

    // Copy data from user space to kernel space
    kernelBuf = kzalloc(PREFIX, GFP_KERNEL);
    if (kernelBuf == NULL) {
        return -ENOMEM;
    }
    if (copy_from_user(kernelBuf, buf, len)) {
        printk("ERROR: Failed to copy data from user space\n");
        kfree(kernelBuf);
        return -EFAULT;  // Return error if copy fails
    }

    mutex_lock(&deviceLocks[deviceIndex]);
    // Drop the oldest entry instead of growing without bound
    if (size(deviceLists[deviceIndex]) >= LST_MAX) {
        node *old = dequeue(deviceLists[deviceIndex]);
        if (old) {
            kfree(old->key);
            kfree(old);
        }
    }
    if (enqueue(deviceLists[deviceIndex], kernelBuf) != 0) {
        mutex_unlock(&deviceLocks[deviceIndex]);
        printk("ERROR: Enqueue failed\n");
        kfree(kernelBuf);
        return -ENOMEM;  // Return error if enqueue fails
    }
    deviceLists[deviceIndex]->rear->prev->len = len;
    mutex_unlock(&deviceLocks[deviceIndex]);

    return len;  // Return the length of the data written
}

/*----------------------------------------------
  Function: DeviceInit
  Purpose: Initializes the devices.
  Description: This function is called when the module is loaded. It allocates device numbers, creates device classes, initializes character devices, and sets up the device-specific buffer lists.
  Returns: 0 on success, negative error code on failure.
----------------------------------------------*/
static void DeviceCleanup(int count) {
    int i;

    for (i = count - 1; i >= 0; i--) {
        device_destroy(devClass[i], dev[i]);  // Destroy the device file
        class_destroy(devClass[i]);  // Destroy the class
        cdev_del(&myCharDevice[i]);  // Delete the character device
        unregister_chrdev_region(dev[i], 1);  // Unregister the device number

        // Free the list and its nodes
        if (deviceLists[i]) {
            free_list(deviceLists[i]);
            kfree(deviceLists[i]);
            deviceLists[i] = NULL;
        }
    }
}

static int __init DeviceInit(void) {
    int i;
    int ret;
    struct device *devFile;

    for (i = 0; i < DEVICE_NUM; i++) {
        mutex_init(&deviceLocks[i]);

        // Allocate device number dynamically
        ret = alloc_chrdev_region(&dev[i], 0, 1, region[i]);
        if (ret < 0) {
            printk("ERROR: Cannot allocate major number\n");
            goto fail;
        }

        printk("device index %d, Major = %d, Minor = %d\n", i,  MAJOR(dev[i]), MINOR(dev[i]));  // Log major and minor numbers

        // Create class for each device
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 4, 0)
        devClass[i] = class_create(classArr[i]);
#else
        devClass[i] = class_create(THIS_MODULE, classArr[i]);
#endif
        if (IS_ERR(devClass[i])) {
            printk("ERROR: Cannot create class\n");
            ret = PTR_ERR(devClass[i]);
            goto fail_region;
        }

        // Initialize the list for the device before it becomes reachable
        deviceLists[i] = kmalloc(sizeof(list), GFP_KERNEL);
        if (!deviceLists[i]) {
            printk("ERROR: Memory allocation failed for device list\n");
            ret = -ENOMEM;
            goto fail_class;
        }
        init_list(deviceLists[i]);  // Initialize the list for the device
        if (deviceLists[i]->front == NULL) {
            ret = -ENOMEM;
            goto fail_list;
        }

        // Initialize and add the character device
        cdev_init(&myCharDevice[i], &fOpStruct[i]);
        ret = cdev_add(&myCharDevice[i], dev[i], 1);
        if (ret < 0) {
            printk("ERROR: Cannot add the device\n");
            goto fail_list;
        }

        // Create the device file
        devFile = device_create(devClass[i], NULL, dev[i], NULL, devnames[i]);
        if (IS_ERR(devFile)) {
            printk("ERROR: Cannot create device file\n");
            ret = PTR_ERR(devFile);
            goto fail_cdev;
        }
    }

    printk("Device Driver Inserted: Success\n");
    return 0;

fail_cdev:
    cdev_del(&myCharDevice[i]);
fail_list:
    free_list(deviceLists[i]);
    kfree(deviceLists[i]);
    deviceLists[i] = NULL;
fail_class:
    class_destroy(devClass[i]);
fail_region:
    unregister_chrdev_region(dev[i], 1);
fail:
    DeviceCleanup(i);
    return ret;
}

/*----------------------------------------------
  Function: DeviceExit
  Purpose: Cleans up the devices.
  Description: This function is called when the module is unloaded. It removes device files, destroys device classes, deletes character devices, and frees the device-specific buffer lists.
  Returns: None.
----------------------------------------------*/
static void __exit DeviceExit(void) {
    DeviceCleanup(DEVICE_NUM);
    printk("Device Driver Removed: Success\n");
}

module_init(DeviceInit);  // Call the initialization function
module_exit(DeviceExit);  // Call the exit function

MODULE_LICENSE("GPL");  // Declare the license as GPL

