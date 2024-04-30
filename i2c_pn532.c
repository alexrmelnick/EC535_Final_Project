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
                                                    //* must be 2 since i2cdetect fails when reset is high but succeeds when reset is low                             
#define SLAVE_DEVICE_NAME   ( "pn532"  )            // Device and Driver Name
#define PN532_SLAVE_ADDR    ( 0x24     )            // PN532 NFC Tag Reader/Writer Slave Address
                                                    //* must be 0x24 since i2cget fails when reset is high but succeeds when reset is low

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
//static int pn532_send_command(struct i2c_client *client, const u8 *command, size_t command_len);
//static int pn532_read_response(struct i2c_client *client, u8 *response, size_t response_len);

static int hard_reset(void);

static int __init pn532_driver_init(void);
static void __exit pn532_driver_exit(void);

static struct i2c_board_info info = {
        I2C_BOARD_INFO("pn532", PN532_SLAVE_ADDR)
};

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

// // Function to send a simple command to the PN532
// static int pn532_send_command(struct i2c_client *client, const u8 *command, size_t command_len) {
//     int ret;

//     ret = i2c_master_send(client, command, command_len);
//     if (ret < 0) {
//         printk(KERN_ERR "Failed to send command to PN532\n");
//     } else {
//         printk(KERN_INFO "Sent command to PN532 successfully\n");
//     }
//     return ret;
// }

// // Function to read data from the PN532
// static int pn532_read_response(struct i2c_client *client, u8 *response, size_t response_len) {
//     int ret;

//     ret = i2c_master_recv(client, response, response_len);
//     if (ret < 0) {
//         printk(KERN_ERR "Failed to read response from PN532\n");
//     } else {
//         printk(KERN_INFO "Read response from PN532 successfully\n");
//     }
//     return ret;
// }

// // Probe function that will be called when the I2C device with matching ID is found
// static int pn532_probe(struct i2c_client *client, const struct i2c_device_id *id) {
//     u8 get_firmware_version_cmd[] = {0x02, 0x02, 0x00, 0xD4, 0x02, 0x2A}; // Sample command
//     u8 response[12];

//     printk(KERN_INFO "PN532 [%s] probed at I2C address 0x%02x\n", client->name, client->addr);

//     // Send a command to the PN532
//     pn532_send_command(client, get_firmware_version_cmd, sizeof(get_firmware_version_cmd));
//     msleep(100); // Wait for the PN532 to process the command

//     // Read the response from the PN532
//     pn532_read_response(client, response, sizeof(response));

//     return 0;
// }

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
    // Write data to the PN532
    int result;
    u8 general_Call = (PN532_SLAVE_ADDR << 1) | 0x01; // General Call address

    /**
     * First set PN532 to Slave Receiver mode (section 8.3.2.4)
     * "To initiate the Slave receiver mode, I2CADR must be loaded with 
     * the 7-bit Slave address to which the I2C interface will respond when addressed by a Master. 
     * Also the least significant bit of I2CADR should be set to logic 1 if the interface should 
     * respond to the general call address (00h)"
    */ 
    result = i2c_master_send(client, &general_Call, 1);

    /**
     * "The control register, I2CCON, should be initialized with ENS1 and AA set to logic 1
     *  and STA, STO, and SI set to logic 0 in order to enter the Slave receiver mode. 
     * Setting the AA bit will enable the logic to acknowledge its own Slave address 
     * or the general call address and ENS1 will enable the interface."
    */


    return 0;
}

static int pn532_Read(struct i2c_client *client, unsigned char *out_buf, unsigned int len) {
    // Read data from the PN532

    // First set PN532 to Slave Transmitter mode (section 8.3.2.5)



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

    // Register the I2C device
    pn532_adapter = i2c_get_adapter(I2C_BUS_AVAILABLE);
    if (!pn532_adapter) {
        printk(KERN_WARNING "Cannot get the I2C adapter\n");
        return -ENODEV;
    } else if (DEBUG) {
        printk(KERN_INFO "Got the I2C adapter\n");
    }

    // Create the I2C device
    pn532_client = i2c_new_device(pn532_adapter, &info);
    if (!pn532_client) {
        i2c_put_adapter(pn532_adapter); // Release the adapter if the device cannot be created
        printk(KERN_WARNING "Cannot register the I2C device\n");
        return -ENODEV;
    } else if (DEBUG) {
        printk(KERN_INFO "Registered the I2C device\n");
    }

    // Register the I2C driver
    // result = i2c_add_driver(&pn532_driver);
    // if (result < 0) {
    //     printk(KERN_WARNING "Cannot register the PN532 driver\n");
    //     return result;
    // } else if (DEBUG) {
    //     printk(KERN_INFO "Registered the PN532 driver\n");
    // }

    printk(KERN_INFO "PN532 device is initialized\n");
    return 0;
}

static void __exit pn532_driver_exit(void) {
    if(pn532_client) {
        i2c_unregister_device(pn532_client);
        printk(KERN_INFO "PN532 device is unregistered\n");
    }
    if (pn532_adapter){
        i2c_put_adapter(pn532_adapter);
    }
}

module_init(pn532_driver_init);
module_exit(pn532_driver_exit);