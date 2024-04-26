// SPI MFRC522 Slave Driver

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/spi/spi.h>

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Alex Melnick and Alfonso Meraz");
MODULE_DESCRIPTION("Linux driver for MFRC522");

static final bool DEBUG = true;

static int __init mfrc522_spi_init(void);
static void __exit mfrc522_spi_exit(void);

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
    int result;
    
    // Add the slave device to the SPI controller
    master = spi_busnum_to_master(spi_device_info.bus_num);
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
    mfrc522_spi_device->bits_per_word = 8;
    result = spi_setup(mfrc522_spi_device);
    if (result) {
        printk(KERN_ALERT "SPI slave setup failed.\n");
        spi_unregister_device(mfrc522_spi_device);
        return -ENODEV;
    } else if (DEBUG) {
        printk(KERN_INFO "SPI slave setup successful.\n");
    }

    // Initialize the MFRC522
    // TODO - Implement this function

    // Perform a self-test
    // TODO - Implement this function
    if (success && DEBUG) {
        printk(KERN_INFO "MFRC522 self-test successful.\n");
    } else {
        printk(KERN_ALERT "MFRC522 self-test failed.\n");
    } 

    printk(KERN_INFO "MFRC522 SPI driver initialized.\n");
    return 0;
}

static void __exit mfrc522_spi_exit(void)
{
    // Deinitialize the MFRC522
    // TODO - Implement this function

    // Unregister the SPI slave device
    spi_unregister_device(mfrc522_spi_device);
    printk(KERN_INFO "MFRC522 SPI driver deinitialized.\n");
}

module_init(mfrc522_spi_init);
module_exit(mfrc522_spi_exit);