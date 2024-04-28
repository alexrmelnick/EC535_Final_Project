/**
 * @file i2c_pn532.c
 * @brief This file contains the I2C driver implementation for the PN532 NFC tag reader and writer
*/

#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/fs.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alex & Alfonso");
MODULE_DESCRIPTION("A simple Linux kernel module for interacting with the PN532 NFC Reader over I2C.");

static const bool DEBUG = true;

#define I2C_BUS_AVAILABLE   ( 0        )            // I2C Bus
#define SLAVE_DEVICE_NAME   ( "pn532"  )            // Device and Driver Name
#define PN532_SLAVE_ADDR    ( 0x24     )            // PN532 NFC Tag Reader/Writer Slave Address
                                                    //? Not sure if this is the correct address

static int major = 61;
static struct file_operations fops;

static struct i2c_client *pn532_client;
static struct i2c_adapter *pn532_adapter;

static int __init pn532_init(void);
static void __exit pn532_exit(void);

static int pn532_remove(struct i2c_client *client);
static int pn532_probe(struct i2c_client *client, const struct i2c_device_id *id);
static int pn532_reset(void);

static const struct i2c_device_id pn532_id[] = {
    { SLAVE_DEVICE_NAME, 0 },
    { }
};

MODULE_DEVICE_TABLE(i2c, pn532_id);

static struct i2c_driver pn532_driver = {
    .driver = {
        .name = SLAVE_DEVICE_NAME,
        .owner = THIS_MODULE,
    },
    .probe = pn532_probe,
    .remove = pn532_remove
};

static struct i2c_board_info pn532_board_info = {
    I2C_BOARD_INFO(SLAVE_DEVICE_NAME, PN532_SLAVE_ADDR)
};

static int __init pn532_init(void) {
    int result;

    printk(KERN_INFO "Initializing the PN532 module\n");

    // Register the I2C device
    pn532_adapter = i2c_get_adapter(I2C_BUS_AVAILABLE);
    if (!pn532_adapter) {
        printk(KERN_WARNING "Cannot get the I2C adapter\n");
        return -ENODEV;
    } else if (DEBUG) {
        printk(KERN_INFO "Got the I2C adapter\n");
    }

    // Create the I2C device
    pn532_client = i2c_new_device(pn532_adapter, &pn532_board_info);
    if (!pn532_client) {
        i2c_put_adapter(pn532_adapter); // Release the adapter if the device cannot be created
        printk(KERN_WARNING "Cannot create the I2C device\n");
        return -ENODEV;
    } else if (DEBUG) {
        printk(KERN_INFO "Created the I2C device\n");
    }

    // Register the I2C device
    pn532_adapter = i2c_get_adapter(I2C_BUS_AVAILABLE);
    if (!pn532_adapter) {
        printk(KERN_WARNING "Cannot get the I2C adapter\n");
        return -ENODEV;
    } else if (DEBUG) {
        printk(KERN_INFO "Got the I2C adapter\n");
    }

    // Register the I2C driver
    result = i2c_add_driver(&pn532_driver);
    if (result < 0) {
        printk(KERN_WARNING "Cannot register the PN532 driver\n");
        return result;
    } else if (DEBUG) {
        printk(KERN_INFO "Registered the PN532 driver\n");
    }

    return 0;
}

static void __exit pn532_exit(void) {
    i2c_unregister_device(pn532_client);
    i2c_del_driver(&pn532_driver);
    printk(KERN_INFO "PN532 driver removed\n");
}

module_init(pn532_init);
module_exit(pn532_exit);