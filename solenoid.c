#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>          // Required for GPIO functions
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/uaccess.h>       // Required for copy_to_user function
#include "solenoid.h"

#define SOLENOID_GPIO_PIN 50    // Define the GPIO pin number for the solenoid

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alfonso Meraz and Alex Melnick");
MODULE_DESCRIPTION("Kernel module for controlling a solenoid lock");

// Function prototypes
static int solenoid_open(struct inode *inode, struct file *file);
static int solenoid_release(struct inode *inode, struct file *file);
static ssize_t solenoid_write(struct file *file, const char __user *buf, size_t len, loff_t *ppos);

// File operations structure
static struct file_operations solenoid_fops = {
    .owner = THIS_MODULE,
    .open = solenoid_open,
    .release = solenoid_release,
    .write = solenoid_write,
};

static int major = 61;              // Major number for device file
static struct class *solenoid_class;

// Initialize the solenoid module
static int __init solenoid_init(void) {
    int result;

    // Registering device
    major = register_chrdev(major, "solenoid", &solenoid_fops);
    if (major < 0) {
        printk(KERN_ALERT "Solenoid module registration failed: %d\n", major);
        return major;
    }

    // Creating class
    solenoid_class = class_create(THIS_MODULE, "solenoid_class");
    if (IS_ERR(solenoid_class)) {
        unregister_chrdev(major, "solenoid");
        printk(KERN_ALERT "Failed to create class.\n");
        return PTR_ERR(solenoid_class);
    }

    // Creating device
    device_create(solenoid_class, NULL, MKDEV(major, 0), NULL, "solenoid%d", 0);

    // Request GPIO
    result = gpio_request(SOLENOID_GPIO_PIN, "solenoid_gpio");
    if (result) {
        printk(KERN_ERR "Unable to request GPIOs: %d\n", result);
        return result;
    }

    // Set GPIO as output and initialize it to high (lock)
    gpio_direction_output(SOLENOID_GPIO_PIN, 1);

    printk(KERN_INFO "Solenoid module has been loaded successfully\n");
    return 0;
}

// Clean up the solenoid module
static void __exit solenoid_exit(void) {
    gpio_set_value(SOLENOID_GPIO_PIN, 1);  // Ensure solenoid is locked
    gpio_free(SOLENOID_GPIO_PIN);          // Free the GPIO pin

    device_destroy(solenoid_class, MKDEV(major, 0));
    class_unregister(solenoid_class);
    class_destroy(solenoid_class);
    unregister_chrdev(major, "solenoid");

    printk(KERN_INFO "Solenoid module has been unloaded\n");
}

// Open function for solenoid device file
static int solenoid_open(struct inode *inode, struct file *file) {
    return 0;
}

// Release function for solenoid device file
static int solenoid_release(struct inode *inode, struct file *file) {
    return 0;
}

// Write function for solenoid device file
static ssize_t solenoid_write(struct file *file, const char __user *buf, size_t len, loff_t *ppos) {
    char cmd;

    if (copy_from_user(&cmd, buf, 1)) {
        return -EFAULT;
    }

    switch (cmd) {
        case '0': // Lock the solenoid
            gpio_set_value(SOLENOID_GPIO_PIN, 1);
            break;
        case '1': // Unlock the solenoid
            gpio_set_value(SOLENOID_GPIO_PIN, 0);
            break;
        default:
            printk(KERN_INFO "Solenoid command not recognized\n");
            return -EINVAL;  // Invalid argument error
    }

    return len;
}

int initialize_solenoid(int gpio_pin) {
    int result;
    result = gpio_request(gpio_pin, "solenoid_gpio");
    if (result) {
        printk(KERN_ERR "Unable to request GPIO %d for solenoid: %d\n", gpio_pin, result);
        return result;
    }

    gpio_direction_output(gpio_pin, 1);  // Set the GPIO as a high output, locking the solenoid
    return 0;
}

void cleanup_solenoid(int gpio_pin) {
    gpio_set_value(gpio_pin, 1);  // Ensure solenoid is locked
    gpio_free(gpio_pin);          // Release the GPIO
}

void activate_solenoid(int gpio_pin) {
    gpio_set_value(gpio_pin, 0);  // Set GPIO low to unlock
    printk(KERN_INFO "Solenoid Unlocked\n");
}

void deactivate_solenoid(int gpio_pin) {
    gpio_set_value(gpio_pin, 1);  // Set GPIO high to lock
    printk(KERN_INFO "Solenoid Locked\n");
}

module_init(solenoid_init);
module_exit(solenoid_exit);

