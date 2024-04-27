// SPI MFRC522 Slave Driver
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/spi/spi.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/fs.h>

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Alex Melnick and Alfonso Meraz");
MODULE_DESCRIPTION("Linux driver for MFRC522");

const static bool DEBUG = true;
static int major = 61; //* FOR TESTING PURPOSES

//* FOR TESTING PURPOSES
static struct file_operations fops;




#define GPIO_RST 68

static int __init mfrc522_spi_init(void);
static void __exit mfrc522_spi_exit(void);
static int mfrc522_spi_write_then_read(struct spi_device *spi, const void *txbuf, unsigned n_tx, void *rxbuf, unsigned n_rx);

static int mfrc522_reset(void);
static int mfrc522_read_version(struct spi_device *spi);

static struct spi_device *mfrc522_spi_device;
struct spi_board_info spi_device_info = { 
    .modalias = "mfrc522-driver",   // Name of our SPI device driver
    .max_speed_hz = 1000000, // ? Speed of SPI bus (1MHz) - Hope this is right
    .mode = SPI_MODE_0,      // ? SPI mode (not 100% sure this is the correct mode)
    .bus_num = 1,            // ? SPI bus number (not 100% sure this is the correct number)
    .chip_select = 0,        // SPI chip select - we have only one device connected
};

static int __init mfrc522_spi_init(void)
{
    // Get the SPI master driver
    struct spi_master *master;
    int result, version;
    
    //* FOR TESTING PURPOSES
    register_chrdev(major, "spi_mfrc522_driver", &fops); // Register the device


    master = spi_busnum_to_master(spi_device_info.bus_num);


    if(DEBUG) printk(KERN_INFO "Initializing MFRC522 SPI driver.\n");

    // Add the slave device to the SPI controller    
    if (!master) {
        printk(KERN_ALERT "SPI Master not found.\n");
        return -ENODEV;
    } else if (DEBUG) {
        printk(KERN_INFO "SPI Master found.\n");
    }

    mfrc522_spi_device = spi_new_device(master, &spi_device_info);
    if (!mfrc522_spi_device) {
        printk(KERN_ALERT "Failed to create SPI slave.\n");
        spi_master_put(master); // Decrement the reference count of the master in case of failure since we won't be using it
        return -ENODEV;
    } else if (DEBUG) {
        printk(KERN_INFO "SPI slave created.\n");
    }


    // Configure the SPI interface to take effect on the bus
    mfrc522_spi_device->bits_per_word = 8; // 8 bits per word (byte)
    result = spi_setup(mfrc522_spi_device);
    if (result) {
        printk(KERN_ALERT "SPI slave setup failed.\n");
        spi_unregister_device(mfrc522_spi_device);
        return -ENODEV;
    } else if (DEBUG) {
        printk(KERN_INFO "SPI slave setup successful.\n");
    }

    // Initialize the MFRC522
    mfrc522_reset(); // Reset the MFRC522
    version = mfrc522_read_version(mfrc522_spi_device); // Read the version of the MFRC522
    if (DEBUG) { printk(KERN_INFO "MFRC522 version: %x (expecting 0x92)\n", version); }
    // TODO - Configure the MFRC522
    // TODO - Enable the antenna

    // Perform a self-test
    // TODO - Implement this function
    // if (success && DEBUG) {
    //     printk(KERN_INFO "MFRC522 self-test successful.\n");
    // } else {
    //     printk(KERN_ALERT "MFRC522 self-test failed.\n");
    // } 

    printk(KERN_INFO "MFRC522 SPI driver initialized.\n");
    return 0;
}

static void __exit mfrc522_spi_exit(void)
{
    //* FOR TESTING PURPOSES
    unregister_chrdev(major, "spi_mfrc522_driver"); // Unregister the device

   
    // Deinitialize the MFRC522
    // TODO - Implement this function
    if (DEBUG) { printk(KERN_INFO "MFRC522 deinitialized.\n");}

    // Unregister the SPI slave device
    spi_unregister_device(mfrc522_spi_device);
    printk(KERN_INFO "MFRC522 SPI driver deinitialized.\n");
}

static int mfrc522_spi_write_then_read(struct spi_device *spi, const void *txbuf, unsigned n_tx, void *rxbuf, unsigned n_rx)
{   /* 
    ptr *struct spi_device spi: Pointer to the SPI device structure; represents the SPI slave device.
    ptr *const void txbuf: Pointer to the buffer containing the data to be transmitted.
    unsigned n_tx: Number of bytes to transmit.
    ptr *void rxbuf: Pointer to the buffer where the received data will be stored.
    unsigned n_rx: Number of bytes expected to be received.
    */

    struct spi_transfer t[2] = {0}; // Array of two transfer structures (one for transmit, one for receive)
    struct spi_message m;           // SPI message object
    int result;

    if (DEBUG) { printk(KERN_INFO "SPI writing 0x%x then reading.\n", *(int *)txbuf); }

    spi_message_init(&m);           // Initialize the SPI message
    memset(t, 0, sizeof(t));        // Clear the transfer structures

    t[0].tx_buf = txbuf;            // Set the transmit buffer
    t[0].len = n_tx;                // Set the length of the transmit buffer
    spi_message_add_tail(&t[0], &m);// Add the first transfer to the message

    t[1].rx_buf = rxbuf;            // Set the receive buffer
    t[1].len = n_rx;                // Set the length of the receive buffer
    spi_message_add_tail(&t[1], &m);// Add the second transfer to the message

    result = spi_sync(spi, &m);      // Execute the SPI transaction
    if (result) {
        printk(KERN_WARNING "SPI transaction failed.\n");
        return -ENODEV;
    } else if (DEBUG) {
        printk(KERN_INFO "SPI transaction successful.\n");
    }

    return 0;
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

static int mfrc522_read_version(struct spi_device *spi)
{
    //* Read the version of the MFRC522 - should be 0x92
    int result;
    char txbuf[1] = {0x37}; // Command to read the version
    char rxbuf[1] = {0};    // Buffer to store the version

    if (DEBUG) { printk(KERN_INFO "Reading the version of the MFRC522.\n"); }

    // Send the command to read the version
    result = mfrc522_spi_write_then_read(spi, txbuf, 1, rxbuf, 1);
    if (result) {
        printk(KERN_WARNING "Failed to read the version of the MFRC522.\n");
        return -ENODEV;
    } else if (DEBUG) {
        printk(KERN_INFO "Version of the MFRC522: %d\n", rxbuf[0]);
    }

    if (rxbuf[0] != 0x92) {
        printk(KERN_WARNING "Incorrect version of the MFRC522.\n");
        return -ENODEV;
    }

    return (int)rxbuf[0]; // Return the version
}

module_init(mfrc522_spi_init);
module_exit(mfrc522_spi_exit);