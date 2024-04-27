/**
 * @file i2c_driver.c
 * @brief This file contains the I2C driver implementation for the MFRC522 NFC tag reader and writer
 * This is implemented on the BeagleBone Black using the Linux I2C driver
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
MODULE_DESCRIPTION("A simple Linux kernel module for interacting with the MFRC522 NFC Reader over I2C.");

static const bool DEBUG = true;

#define I2C_BUS_AVAILABLE   (          0 )              // I2C Bus
#define SLAVE_DEVICE_NAME   ( "mfrc522"  )              // Device and Driver Name
#define MFRC522_SLAVE_ADDR  (       0x28 )              // MFRC522 NFC Tag Reader/Writer Slave Address

#define GPIO_RST 68

static int major = 61;
static struct file_operations fops;


static struct i2c_client *mfrc522_client;
static struct i2c_adapter *mfrc522_adapter;

//static int I2C_Write(unsigned char *buf, unsigned int len);
//static int I2C_Read(unsigned char *out_buf, unsigned int len);

static int __init mfrc522_init(void);
static void __exit mfrc522_exit(void);

static int mfrc522_remove(struct i2c_client *client);
static int mfrc522_probe(struct i2c_client *client, const struct i2c_device_id *id);
static int mfrc522_reset(void);

static const struct i2c_device_id mfrc522_id[] = {
    { SLAVE_DEVICE_NAME, 0 },
    { }
};

MODULE_DEVICE_TABLE(i2c, mfrc522_id);

static struct i2c_driver mfrc522_driver = {
    .driver = {
        .name = SLAVE_DEVICE_NAME,
        .owner = THIS_MODULE,
    },
    //* I think we need to add more to this struct with other methods for the board itself
    .probe = mfrc522_probe,
    .remove = mfrc522_remove,
    .id_table = mfrc522_id,
};

static struct i2c_board_info mfrc522_info = {
    I2C_BOARD_INFO(SLAVE_DEVICE_NAME, MFRC522_SLAVE_ADDR)
};

// Probe function called when a matching I2C device is found
static int mfrc522_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    int err;
    char test_data = 0x55; // Test data to write

    printk(KERN_INFO "Probing MFRC522 at I2C address 0x%02x\n", client->addr);

    // Attempt to write test data to a register
    err = i2c_smbus_write_byte_data(client, 0x01, test_data);
    if (err) {
        printk(KERN_ERR "MFRC522 write failed\n");
        return err;
    }

    // Attempt to read back the test data
    err = i2c_smbus_read_byte_data(client, 0x01);
    if (err < 0) {
        printk(KERN_ERR "MFRC522 read failed\n");
        return err;
    }
    
    printk(KERN_INFO "MFRC522 test read/write successful, data: 0x%02x\n", err);
    return 0;
}


// Remove function called when the device is removed or the driver is unloaded
static int mfrc522_remove(struct i2c_client *client)
{
    printk(KERN_INFO "MFRC522 (%s) Removed from I2C address 0x%02x\n", client->name, client->addr);
    return 0;
}

// Module Initialization
static int __init mfrc522_init(void) {
    int result;
   
    result = register_chrdev(major, SLAVE_DEVICE_NAME, &fops); // Register the device
    if (result < 0) {
        printk(KERN_WARNING "MFRC522: can't get major %d\n", major);
        return result;
    } else if (DEBUG) {
        printk(KERN_INFO "MFRC522: registered correctly with major number %d\n", major);
    }

    mfrc522_reset(); // Reset the MFRC522
   
    mfrc522_adapter = i2c_get_adapter(I2C_BUS_AVAILABLE);
    if (mfrc522_adapter == NULL) {
        printk(KERN_WARNING "MFRC522: unable to get I2C adapter\n");
        return -ENODEV;
    } else if (DEBUG) {
        printk(KERN_INFO "MFRC522: got I2C adapter\n");
    }

    mfrc522_client = i2c_new_device(mfrc522_adapter, &mfrc522_info);
    if (mfrc522_client != NULL) {
        i2c_put_adapter(mfrc522_adapter);
        printk(KERN_INFO "MFRC522 driver added\n");
    } else {
        printk(KERN_WARNING "MFRC522: unable to create I2C device\n");
        return -ENODEV;
    }

    i2c_put_adapter(mfrc522_adapter); // Always release the adapter after use

    i2c_add_driver(&mfrc522_driver);


    // TODO - add self-test

    return 0;
}

// Module Cleanup
static void __exit mfrc522_exit(void) {
    unregister_chrdev(major, SLAVE_DEVICE_NAME); // Unregister the device
    i2c_unregister_device(mfrc522_client); // Unregister the I2C device
    printk(KERN_INFO "MFRC522 driver removed\n");
}

static int mfrc522_reset(void)
{
    // Reset the MFRC522
    int result;

    if (DEBUG) { printk(KERN_INFO "Resetting the MFRC522.\n"); }

    // Set up the GPIO
    result = gpio_request(GPIO_RST, "RST");  // Request the GPIO
    if (result < 0) {
        printk(KERN_WARNING "NFC_tag: unable to request GPIO %d\n", GPIO_RST);
        return result;
    } else if (DEBUG) {
        printk(KERN_INFO "NFC_tag: requested GPIO %d\n", GPIO_RST);
    }
    result = gpio_direction_output(GPIO_RST, 1); // Set the GPIO as an output
    if (result < 0) {
        printk(KERN_WARNING "NFC_tag: unable to set GPIO %d as output\n", GPIO_RST);
        return result;
    } else if (DEBUG) {
        printk(KERN_INFO "NFC_tag: set GPIO %d as output\n", GPIO_RST);
    }

    // Reset the MFRC522
    gpio_set_value(GPIO_RST, 0); // Set the GPIO low

    // Wait for a short period
    msleep(100); // Wait for 100 ms - cannot find on the datasheet so leaving low for a long period

    // Release the reset
    gpio_set_value(GPIO_RST, 1);

    // Free the GPIO
    gpio_free(GPIO_RST);

    return 0;
}

module_init(mfrc522_init);
module_exit(mfrc522_exit);