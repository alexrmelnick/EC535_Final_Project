// This kernal module is used to encode the NFC tag data onto the NFC tag

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/gpio.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/spi/spi.h>
#include <linux/dekay.h>

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Alex Melnick and Alfonso Meraz");
MODULE_DESCRIPTION("Linux driver for encoding NFC tags");

static int major = 61;

#define GPIO_IRQ 44

static bool DEBUG = true;

static ssize_t NFC_tag_read(struct file *file, char *buffer, size_t length, loff_t *offset);
static ssize_t NFC_tag_write(struct file *file, const char *buffer, size_t length, loff_t *offset);
static int NFC_tag_open(struct inode *inode, struct file *file);

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .read = NFC_tag_read,
    .write = NFC_tag_write,
    .open = NFC_tag_open
};

static int __init NFC_tag_init(void) {
    int result;
    
    if (DEBUG) printk(KERN_INFO "NFC_tag: Initializing the NFC_tag module\n");

    // Register the device
    result = register_chrdev(major, "NFC_tag", &fops);
    if (result < 0) {
        printk(KERN_WARNING "NFC_tag: can't get major %d\n", major);
        return result;
    } else if (DEBUG) {
        printk(KERN_INFO "NFC_tag: registered correctly with major number %d\n", major);
    }

    

    result = gpio_request(GPIO_IRQ, "IRQ");
    if (result < 0) {
        printk(KERN_WARNING "NFC_tag: unable to request GPIO %d\n", GPIO_IRQ);
        return result;
    } else if (DEBUG) {
        printk(KERN_INFO "NFC_tag: requested GPIO %d\n", GPIO_IRQ);
    }
    result = gpio_direction_input(GPIO_IRQ);
    if (result < 0) {
        printk(KERN_WARNING "NFC_tag: unable to set GPIO %d as input\n", GPIO_IRQ);
        return result;
    } else if (DEBUG) {
        printk(KERN_INFO "NFC_tag: set GPIO %d as input\n", GPIO_IRQ);
    }
}

static void __exit NFC_tag_exit(void) {
    if (DEBUG) printk(KERN_INFO "NFC_tag: Exiting the NFC_tag module\n");

    // Unregister the device
    unregister_chrdev(major, "NFC_tag");

   
}