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
#define SPEED 9600 // Speed of SPI bus (default 9.6 kBd)

static int __init mfrc522_spi_init(void);
static void __exit mfrc522_spi_exit(void);

static int mfrc522_spi_write_then_read(struct spi_device *spi, const void *txbuf, unsigned n_tx, void *rxbuf, unsigned n_rx);
static int mfrc522_spi_write_byte(struct spi_device *spi, uint8_t address, uint8_t data);
static int mfrc522_spi_write_data(struct spi_device *spi, uint8_t address, uint8_t *data, uint8_t length);
static int mfrc522_spi_read_byte(struct spi_device *spi, uint8_t address, uint8_t *data);
static int mfrc522_spi_read_data(struct spi_device *spi, uint8_t address, uint8_t *data, uint8_t length);
//static int mfrc522_send_command(struct spi_device *spi, uint8_t RcvOff, uint8_t PowerDown, uint8_t Command);
static int mfrc522_self_test(struct spi_device *spi);

static int mfrc522_hard_reset(void);
static int mfrc522_read_version(struct spi_device *spi);

static struct spi_device *mfrc522_spi_device;

struct spi_board_info spi_device_info = { 
    .modalias = "mfrc522-driver",   // Name of our SPI device driver
    .max_speed_hz = SPEED,           // Speed of SPI bus (9.6 kBd) - Hope this is right
    .mode = SPI_MODE_0,             // ? SPI mode (not 100% sure this is the correct mode)
    .bus_num = 1,                   // ? SPI bus number (not 100% sure this is the correct number)
    .chip_select = 0,               // SPI chip select - we have only one device connected
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
    mfrc522_hard_reset(); // Reset the MFRC522
    // version = mfrc522_read_version(mfrc522_spi_device); // Read the version of the MFRC522
    // if (DEBUG) { printk(KERN_INFO "MFRC522 version: %x (expecting 0x92)\n", version); }
    // // TODO - Configure the MFRC522
    // // TODO - Enable the antenna

    // Perform a self-test
    mfrc522_self_test(mfrc522_spi_device);

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
        return result;
    } else if (DEBUG) {
        printk(KERN_INFO "SPI transaction successful.\n");
    }

    return 0;
}

static int mfrc522_spi_write_byte(struct spi_device *spi, uint8_t address, uint8_t data)
{
    // Write a byte to the MFRC522
    int result;
    char txbuf[2] = {address << 1, data}; // Buffer to store the address and data
    char rxbuf[1] = {0};              // Buffer to store the response

    if (DEBUG) { printk(KERN_INFO "Writing 0x%x to address 0x%x.\n", data, address); }

    // Write the byte
    result = mfrc522_spi_write_then_read(spi, txbuf, 2, rxbuf, 1);
    if (result) {
        printk(KERN_WARNING "Failed to write 0x%x to address 0x%x.\n", data, address);
        return -ENODEV;
    } else if (DEBUG) {
        printk(KERN_INFO "Wrote 0x%x to address 0x%x.\n", data, address);
    }

    return 0;
}

static int mfrc522_spi_write_data(struct spi_device *spi, uint8_t address, uint8_t *data, uint8_t length) {
    // Write data to the MFRC522
    int result;
    char *txbuf = kmalloc(length + 1, GFP_KERNEL); // Buffer to store the address and data
    char rxbuf[1] = {0};                           // Buffer to store the response

    if (DEBUG) { printk(KERN_INFO "Writing data to address 0x%x.\n", address); }

    // Prepare the buffer
    txbuf[0] = address << 1; // Set the address
    memcpy(txbuf + 1, data, length); // Copy the data to the buffer

    // Write the data
    result = mfrc522_spi_write_then_read(spi, txbuf, length + 1, rxbuf, 1);
    if (result) {
        printk(KERN_WARNING "Failed to write data to address 0x%x.\n", address);
        return -ENODEV;
    } else if (DEBUG) {
        printk(KERN_INFO "Wrote data to address 0x%x.\n", address);
    }

    kfree(txbuf); // Free the buffer

    return 0;

}

static int mfrc522_spi_read_byte(struct spi_device *spi, uint8_t address, uint8_t *data)
{
    // Read a byte from the MFRC522
    int result;
    char txbuf[1] = {address << 1}; // Buffer to store the address
    char rxbuf[1] = {0};       // Buffer to store the response

    if (DEBUG) { printk(KERN_INFO "Reading from address 0x%x.\n", address); }

    // Read the byte
    result = mfrc522_spi_write_then_read(spi, txbuf, 1, rxbuf, 1);
    if (result) {
        printk(KERN_WARNING "Failed to read from address 0x%x.\n", address);
        return -ENODEV;
    } else if (DEBUG) {
        printk(KERN_INFO "Read 0x%x from address 0x%x.\n", rxbuf[0], address);
    }

    *data = rxbuf[0]; // Store the data in the pointer

    return 0;
}

static int mfrc522_spi_read_data(struct spi_device *spi, uint8_t address, uint8_t *data, uint8_t length) {
    // Read data from the MFRC522
    int result;
    char txbuf[1] = {address << 1}; // Buffer to store the address
    char *rxbuf = kmalloc(length, GFP_KERNEL); // Buffer to store the response

    if (DEBUG) { printk(KERN_INFO "Reading data from address 0x%x.\n", address); }

    // Read the data
    result = mfrc522_spi_write_then_read(spi, txbuf, 1, rxbuf, length);
    if (result) {
        printk(KERN_WARNING "Failed to read data from address 0x%x.\n", address);
        return -ENODEV;
    } else if (DEBUG) {
        printk(KERN_INFO "Read data from address 0x%x.\n", address);
    }

    memcpy(data, rxbuf, length); // Copy the data to the pointer

    kfree(rxbuf); // Free the buffer

    return 0;

}

/**
 * @brief Send a command to the MFRC522 command register 
 * @param spi Pointer to the SPI device structure
 * @param RcvOff Analogue part of the receiver is switched off
 * @param PowerDown High: Soft power-down mode entered; Low: Wake up procedure begins
 * @param Command Command to send
*/
static int mfrc522_send_command(struct spi_device *spi, uint8_t RcvOff, uint8_t PowerDown, uint8_t Command)
{
    // From MFRC522 datasheet 9.3.1.2 CommandReg register (page 38)
    // Prepare the command
    // Bit  7   6     5     4     3     2     1     0
    //      0   0 RcvOff PowerDown   Command[3:0] 

    uint8_t commandReg = 0x01; // Command register
    uint8_t data = (RcvOff << 6) | (PowerDown << 5) | Command; // Command byte

    if (DEBUG) { printk(KERN_INFO "Sending command 0x%x.\n", data); }

    // Send the command
    return mfrc522_spi_write_byte(spi, commandReg, data);
}

static int mfrc522_hard_reset(void)
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
    msleep(200); // Wait for 200 ms - cannot find on the datasheet so leaving low for a long period

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

static int mfrc522_self_test(struct spi_device *spi) {
    /*
        Procedure to perform a self-test on the MFRC522:
        1. Perform a soft reset.
        2. Clear the internal buffer by writing 25 bytes of 00h and implement the Config command.
        3. Enable the self test by writing 09h to the AutoTestReg register.
        4. Write 00h to the FIFO buffer.
        5. Start the self test with the CalcCRC command.
        6. The self test is initiated.
        7. When the self test has completed, the FIFO buffer contains the following 64 bytes:
            00h, EBh, 66h, BAh, 57h, BFh, 23h, 95h,
            D0h, E3h, 0Dh, 3Dh, 27h, 89h, 5Ch, DEh,
            9Dh, 3Bh, A7h, 00h, 21h, 5Bh, 89h, 82h, 
            51h, 3Ah, EBh, 02h, 0Ch, A5h, 00h, 49h, 
            7Ch, 84h, 4Dh, B3h, CCh, D2h, 1Bh, 81h,
            5Dh, 48h, 76h, D5h, 71h, 061h, 21h, A9h,
            86h, 96h, 83h, 38h, CFh, 9Dh, 5Bh, 6Dh, 
            DCh, 15h, BAh, 3Eh, 7Dh, 95h, 03Bh, 2Fh
    */

    // int result, 
    int i;
    uint8_t *temp;               // Temporary byte to read from the FIFO buffer
    uint8_t zeros[25] = {0}; // 25 bytes of 00h
    uint8_t result[64] = {0}; // Buffer to store the result initialized to 0 
    uint8_t expected_result[64] = {
        0x00, 0xEB, 0x66, 0xBA, 0x57, 0xBF, 0x23, 0x95,
        0xD0, 0xE3, 0x0D, 0x3D, 0x27, 0x89, 0x5C, 0xDE,
        0x9D, 0x3B, 0xA7, 0x00, 0x21, 0x5B, 0x89, 0x82,
        0x51, 0x3A, 0xEB, 0x02, 0x0C, 0xA5, 0x00, 0x49,
        0x7C, 0x84, 0x4D, 0xB3, 0xCC, 0xD2, 0x1B, 0x81,
        0x5D, 0x48, 0x76, 0xD5, 0x71, 0x61, 0x21, 0xA9,
        0x86, 0x96, 0x83, 0x38, 0xCF, 0x9D, 0x5B, 0x6D,
        0xDC, 0x15, 0xBA, 0x3E, 0x7D, 0x95, 0x3B, 0x2F
    };

    uint8_t FIFOLevelReg = 0x0A; // FIFO level register
    uint8_t FIFODataReg  = 0x09;  // FIFO data register
    uint8_t AutoTestReg  = 0x36;  // Auto test register

    

    if (DEBUG) { printk(KERN_INFO "Performing a self-test on the MFRC522.\n"); }

   // 1. Perform a soft reset
    mfrc522_send_command(spi, 0, 0, 0b1111); // Soft reset
    msleep(150);                             // Wait for 150 ms

    if (DEBUG) { printk(KERN_INFO "Soft reset complete.\n"); }

    // 2. Clear the internal buffer by writing 25 bytes of 00h and implement the Config command
    mfrc522_spi_write_byte(spi, FIFOLevelReg, 0x80); // Flush the FIFO buffer
    mfrc522_spi_write_data(spi, FIFODataReg, zeros, 25); // Clear the internal buffer
    mfrc522_send_command(spi, 0, 0, 0b0001); // Mem command

    // 3. Enable the self test by writing 09h to the AutoTestReg register
    mfrc522_spi_write_byte(spi, AutoTestReg, 0x09); // Enable the self test

    // 4. Write 00h to the FIFO buffer
    mfrc522_spi_write_byte(spi, FIFODataReg, 0x00); // Write 00h to the FIFO buffer

    // 5. Start the self test with the CalcCRC command
    mfrc522_send_command(spi, 0, 0, 0b0100); // CalcCRC command

    // 6. The self test is initiated
    for (i = 0; i < 64; i++) { // CRC is done after 64 bytes
        mfrc522_spi_read_byte(spi, FIFOLevelReg, temp); // Read the data from the FIFO buffer
    }
    mfrc522_send_command(spi, 0, 0, 0b0000); // Idle command to stop the CRC

    // 7. Read the data from the FIFO buffer
    mfrc522_spi_read_data(spi, FIFODataReg, result, 64); // Read the data from the FIFO buffer
    mfrc522_spi_write_byte(spi, AutoTestReg, 0x00); // Disable the self test

    // Compare the result with the expected result
    for (i = 0; i < 64; i++) {
        printk(KERN_INFO "Result: 0x%02x, Expected: 0x%02x\n", result[i], expected_result[i]);
        if (result[i] != expected_result[i]) {
            printk(KERN_WARNING "Self-test failed.\n");
            return -ENODEV;
        }
    }

    printk(KERN_INFO "Self-test successful.\n");
    return 0;
}

module_init(mfrc522_spi_init);
module_exit(mfrc522_spi_exit);