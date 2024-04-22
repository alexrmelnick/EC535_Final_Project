// This kernal module is used to encode the NFC tag data onto the NFC tag

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/gpio.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/spi/spi.h>

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Alex Melnick and Alfonso Meraz");
MODULE_DESCRIPTION("Linux driver for encoding NFC tags");

static int major = 61;

#define GPIO_RST 68
#define GPIO_IRQ 44

static bool DEBUG = true;

static ssize_t NFC_tag_read(struct file *file, char *buffer, size_t length, loff_t *offset);
static ssize_t NFC_tag_write(struct file *file, const char *buffer, size_t length, loff_t *offset);
static int NFC_tag_open(struct inode *inode, struct file *file);

static int mfrc522_spi_init(void);
static int mfrc522_spi_write_then_read(struct spi_device *spi, const void *txbuf, unsigned n_tx, void *rxbuf, unsigned n_rx);

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .read = NFC_tag_read,
    .write = NFC_tag_write,
    .open = NFC_tag_open
};

static struct spi_device *MFRC522_spi_device;

static int mfrc522_spi_init(void)
{
    struct spi_master *master;
    struct spi_board_info spi_device_info = { // https://linuxtv.org/downloads/v4l-dvb-internals/device-drivers/API-struct-spi-board-info.html
        .modalias = "mfrc522",
        .max_speed_hz = 1000000,  // Speed of SPI bus (1MHz)
        .mode = SPI_MODE_0,
        .bus_num = 1,            // SPI bus number (FIXME: not 100% sure this is the correct number)
        .chip_select = 0,        // SPI chip select - we have only one device connected
        .platform_data = NULL,   // FIXME: can leave as NULL for simple configurations - not sure if this qualifies
    };

    master = spi_busnum_to_master(spi_device_info.bus_num);
    if (!master) {
        printk(KERN_ALERT "SPI Master not found.\n");
        return -ENODEV;
    }

    mfrc522_spi_device = spi_new_device(master, &spi_device_info);
    if (!mfrc522_spi_device) {
        printk(KERN_ALERT "Failed to create SPI device.\n");
        spi_master_put(master);
        return -ENODEV;
    }

    mfrc522_spi_device->bits_per_word = 8;
    if (spi_setup(mfrc522_spi_device)) {
        printk(KERN_ALERT "SPI setup failed.\n");
        spi_unregister_device(mfrc522_spi_device);
        return -ENODEV;
    }

    return 0;
}

static int mfrc522_spi_write_then_read(struct spi_device *spi, const void *txbuf, unsigned n_tx, void *rxbuf, unsigned n_rx)
{
    /* 
        *struct spi_device spi: Pointer to the SPI device structure; represents the SPI slave device.
        *const void txbuf: Pointer to the buffer containing the data to be transmitted.
        unsigned n_tx: Number of bytes to transmit.
        *void rxbuf: Pointer to the buffer where the received data will be stored.
        unsigned n_rx: Number of bytes expected to be received.
    */

    struct spi_transfer t[2] = {0}; // Array of two transfer structures (one for transmit, one for receive)
    struct spi_message m;           // SPI message object

    spi_message_init(&m);           // Initialize the SPI message
    memset(t, 0, sizeof(t));        // Clear the transfer structures

    t[0].tx_buf = txbuf;            // Set the transmit buffer
    t[0].len = n_tx;                // Set the length of the transmit buffer
    spi_message_add_tail(&t[0], &m);// Add the first transfer to the message

    t[1].rx_buf = rxbuf;            // Set the receive buffer
    t[1].len = n_rx;                // Set the length of the receive buffer
    spi_message_add_tail(&t[1], &m);// Add the second transfer to the message

    return spi_sync(spi, &m);       // Execute the SPI transaction
}

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

    // Set up the GPIO
    result = gpio_request(GPIO_RST, "RST");
    if (result < 0) {
        printk(KERN_WARNING "NFC_tag: unable to request GPIO %d\n", GPIO_RST);
        return result;
    } else if (DEBUG) {
        printk(KERN_INFO "NFC_tag: requested GPIO %d\n", GPIO_RST);
    }
    result = gpio_direction_output(GPIO_RST, 1);
    if (result < 0) {
        printk(KERN_WARNING "NFC_tag: unable to set GPIO %d as output\n", GPIO_RST);
        return result;
    } else if (DEBUG) {
        printk(KERN_INFO "NFC_tag: set GPIO %d as output\n", GPIO_RST);
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

    // Initialize the SPI
    result = mfrc522_spi_init();
    if (result < 0) {
        printk(KERN_WARNING "NFC_tag: unable to initialize SPI\n");
        return result;
    } else if (DEBUG) {
        printk(KERN_INFO "NFC_tag: initialized SPI\n");
    }

    return 0;
}

static void __exit NFC_tag_exit(void) {
    if (DEBUG) printk(KERN_INFO "NFC_tag: Exiting the NFC_tag module\n");

    // Unregister the device
    unregister_chrdev(major, "NFC_tag");

    // Free the GPIO
    gpio_free(GPIO_ANTENNA_RST);
}

module_init(NFC_tag_init);
module_exit(NFC_tag_exit);