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

#define I2C_BUS_AVAILABLE   ( 2        )            // I2C Bus
#define SLAVE_DEVICE_NAME   ( "pn532"  )            // Device and Driver Name
#define PN532_SLAVE_ADDR    ( 0xdb     )            // PN532 NFC Tag Reader/Writer Slave Address
                                                    //? Not sure if this is the correct address

#define GPIO_RST 20
#define GPIO_IRQ 44

static int major = 61;
static struct file_operations fops;

static struct i2c_client *pn532_client;
static struct i2c_adapter *pn532_adapter;

static int pn532_probe(struct i2c_client *client, const struct i2c_device_id *id);
static int pn532_remove(struct i2c_client *client);
static int pn532_setup(struct i2c_client *client);
static int pn532_Write(struct i2c_client *client, unsigned char *buf, unsigned int len);
static int pn532_Read(struct i2c_client *client, unsigned char *out_buf, unsigned int len);
static int pn532_self_test(struct i2c_client *client);
static int pn532_get_version(struct i2c_client *client);

static int hard_reset(void);

static int __init pn532_driver_init(void);
static void __exit pn532_driver_exit(void);

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
    .remove = pn532_remove,
    .id_table = pn532_id
};

static struct i2c_board_info pn532_board_info = {
    I2C_BOARD_INFO(SLAVE_DEVICE_NAME, PN532_SLAVE_ADDR)
};

static int pn532_probe(struct i2c_client *client, const struct i2c_device_id *id) {
    
    int err;
    int result;
    char slave_addr_reg = 0xdb;

    if (client == NULL) {
        printk(KERN_WARNING "PN532 probe: client is NULL\n");
        return -ENODEV;
    } else if (DEBUG) {
        printk(KERN_INFO "PN532 probe: client is not NULL\n");
    }

    printk(KERN_INFO "PN532 (%s) Probed at I2C address 0x%02x on adapter %d\n", client->name, client->addr, client->adapter->nr);

    // Initialize the PN532
    // result = pn532_setup(client);
    // if (result < 0) {
    //     printk(KERN_ERR "PN532 setup failed\n");
    //     return result;
    // } else if (DEBUG) {
    //     printk(KERN_INFO "PN532 setup successful\n");
    // }

    // Get the PN532 firmware version
    result = pn532_Read(client, &slave_addr_reg, 1);
    printk(KERN_INFO "PN532 Slave Address: 0x%02x\n", slave_addr_reg);

    //printk(KERN_INFO "MFRC522 test read/write successful, data: 0x%02x\n", err);

    printk(KERN_INFO "PN532 device initialized successfully\n");
    return 0;
}

static int pn532_remove(struct i2c_client *client) {
    // Free the adapter
    kfree(client->adapter);

    printk(KERN_INFO "PN532 (%s) Removed\n", client->name);
    return 0;
}

static int pn532_Write(struct i2c_client *client, unsigned char *buf, unsigned int len) {
    int result = i2c_master_send(client, buf, len);
    if (result < 0) {
        printk(KERN_ERR "PN532 write failed\n");
        return result;
    }

    return 0;
}

static int pn532_Read(struct i2c_client *client, unsigned char *out_buf, unsigned int len) {
    int result = i2c_master_recv(client, out_buf, len);
    if (result < 0) {
        printk(KERN_ERR "PN532 read failed\n");
        return result;
    }

    return 0;
}

static int hard_reset(void)
{
    // Reset the PN532 using the GPIO
    int result;

    if (DEBUG) { printk(KERN_INFO "Resetting the PN532.\n"); }

    // Set up the GPIO
    result = gpio_request(GPIO_RST, "RST");  // Request the GPIO
    if (result < 0) {
        printk(KERN_WARNING "pn532: unable to request GPIO %d\n", GPIO_RST);
        return result;
    } else if (DEBUG) {
        printk(KERN_INFO "pn532: requested GPIO %d\n", GPIO_RST);
    }
    result = gpio_direction_output(GPIO_RST, 1); // Set the GPIO as an output
    if (result < 0) {
        printk(KERN_WARNING "pn532: unable to set GPIO %d as output\n", GPIO_RST);
        return result;
    } else if (DEBUG) {
        printk(KERN_INFO "pn532: set GPIO %d as output\n", GPIO_RST);
    }

    // Reset the pn532
    gpio_set_value(GPIO_RST, 0); // Set the GPIO low

    // Wait for a short period
    msleep(200); // Wait for 200 ms - cannot find on the datasheet so leaving low for a long period

    // Release the reset
    gpio_set_value(GPIO_RST, 1);

    // Free the GPIO
    gpio_free(GPIO_RST);

    return 0;
}

static int pn532_get_version(struct i2c_client *client) {
    uint8_t VersionReg[2] = {0x27, 0x63}; // Get Firmware Version command
    uint8_t RegData = 0;
    int result;

    result = pn532_Write(client, VersionReg, 2);
    result = pn532_Read(client, &RegData, 1);

    printk(KERN_INFO "PN532 Firmware Version: 0x%02x\n", RegData);

    return RegData;
}

static int __init pn532_driver_init(void) {
    int result;

    printk(KERN_INFO "Initializing the PN532 module\n");

    // Register the character device
    result = register_chrdev(major, SLAVE_DEVICE_NAME, &fops);
    if (result < 0) {
        printk(KERN_WARNING "Cannot register the PN532 device\n");
        return result;
    } else if (DEBUG) {
        printk(KERN_INFO "Registered the PN532 device\n");
    }

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

static void __exit pn532_driver_exit(void) {
    i2c_unregister_device(pn532_client);
    i2c_del_driver(&pn532_driver);
    printk(KERN_INFO "PN532 driver removed\n");
}

module_init(pn532_driver_init);
module_exit(pn532_driver_exit);