// This kernal module is used to encode the NFC tag data onto the NFC tag

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/gpio.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/device.h>

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Alex Melnick and Alfonso Meraz");
MODULE_DESCRIPTION("Linux driver for encoding NFC tags");

static int major = 61;

#define GPIO_ANTENA_RST 68

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