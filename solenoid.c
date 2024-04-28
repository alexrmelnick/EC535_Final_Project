/**
 * This module is used to operate the solenoid lock
*/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/timer.h>
#include <linux/init.h>
#include <linux/gpio.h> // For GPIO functions
#include <linux/interrupt.h>
#include <linux/kthread.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/types.h>
#include <linux/uaccess.h>

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Alfonso Meraz & Alex Melnick");
MODULE_DESCRIPTION("Linux driver for traffic light.");

static int major = 61;  //? May change later

#define SOLENOID_GPIO 26
#define DEVICE_NAME "solenoid"

const static bool DEBUG = true;

static ssize_t solenoid_read(struct file *file, char *buffer, size_t length, loff_t *offset);
static ssize_t solenoid_write(struct file *file, const char *buffer, size_t length, loff_t *offset);
static int solenoid_open(struct inode *inode, struct file *file);
static int solenoid_release(struct inode *inode, struct file *file);

static int __init solenoid_init(void);
static void __exit solenoid_exit(void);

static const struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = solenoid_open,
    .release = solenoid_release,
    .read = solenoid_read,
    .write = solenoid_write
};

static bool locked = false; 

static int __init solenoid_init(void) {
    int result;

    printk(KERN_INFO "Initializing the Solenoid module\n");

    // Register the device
    result = register_chrdev(major, "solenoid", &fops);
    if (result < 0) {
        printk(KERN_WARNING "Cannot get major number %d\n", major);
        return result;
    } else if (DEBUG) {
        printk(KERN_INFO "Registered correctly with major number %d\n", major);
    }

    // Requesting the GPIO
    result = gpio_request(SOLENOID_GPIO, "solenoid_gpio");
    if (result) {
        printk(KERN_ALERT "Cannot request GPIO %d: %d\n", SOLENOID_GPIO, result);
        return result;
    } else if (DEBUG) {
        printk(KERN_INFO "Requested GPIO %d\n", SOLENOID_GPIO);
    }

    // Setting the GPIO as output
    result = gpio_direction_output(SOLENOID_GPIO, 0);
    if (result) {
        printk(KERN_ALERT "Cannot set GPIO %d as output: %d\n", SOLENOID_GPIO, result);
        return result;
    } else if (DEBUG) {
        printk(KERN_INFO "Set GPIO %d as output\n", SOLENOID_GPIO);
    }

    // Set the GPIO to 0
    gpio_set_value(SOLENOID_GPIO, 0);

    return 0;


}

static ssize_t solenoid_write(struct file *filep, const char *buffer, size_t len, loff_t *offset){
   char message[256] = {0};
   if (len > 255) len = 255;
   if (copy_from_user(message, buffer, len)) {
       return -EFAULT;
   }

   if (strncmp(message, "on", 2) == 0) {
        if (locked) {
            printk(KERN_INFO "%s: Solenoid is already locked\n", DEVICE_NAME);
            return len;
        } else {
            gpio_set_value(SOLENOID_GPIO, 1);
            locked = true;
            printk(KERN_INFO "%s: Solenoid turned ON\n", DEVICE_NAME);
        }
   } else if (strncmp(message, "off", 3) == 0) {
        if (!locked) {
            printk(KERN_INFO "%s: Solenoid is already unlocked\n", DEVICE_NAME);
            return len;
        } else {
            gpio_set_value(SOLENOID_GPIO, 0);
            locked = false;
            printk(KERN_INFO "%s: Solenoid turned OFF\n", DEVICE_NAME);
        }
   }

   return len;
}

static ssize_t solenoid_read(struct file *filep, char *buffer, size_t len, loff_t *offset){
   char message[256] = {0};
   int message_len = 0;

   if (locked) {
       message_len = sprintf(message, "locked\n");
   } else {
       message_len = sprintf(message, "unlocked\n");
   }

   if (copy_to_user(buffer, message, message_len)) {
       return -EFAULT;
   }

   return message_len;
}

static void __exit solenoid_exit(void) {
    printk(KERN_INFO "Exiting the Solenoid module\n");

    // Unregister the device
    unregister_chrdev(major, DEVICE_NAME);

    // Free the GPIO
    gpio_free(SOLENOID_GPIO);
}

static int solenoid_open(struct inode *inodep, struct file *filep){
   printk(KERN_INFO "%s: Device has been opened\n", DEVICE_NAME);
   return 0;
}

static int solenoid_release(struct inode *inodep, struct file *filep){
   printk(KERN_INFO "%s: Device successfully closed\n", DEVICE_NAME);
   return 0;
}

module_init(solenoid_init);
module_exit(solenoid_exit);
