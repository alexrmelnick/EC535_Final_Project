#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/init.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/delay.h>

#define SOLENOID_GPIO_PIN 50 // Need to change GPIO pin number for solenoid
#define NUM_TOKENS_REQUIRED 2  // Number of tokens required to unlock

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alfonso Meraz");
MODULE_DESCRIPTION("Controller Module for NFC and Solenoid Lock Interaction");

static bool tokens_detected[3] = {false, false, false};  // Token presence states


static int __init init_controller_module(void) {
    printk(KERN_INFO "Initializing Controller Module\n");
    if (initialize_nfc() != 0) {
        printk(KERN_ALERT "Failed to initialize NFC module\n");
        return -EIO;
    }
    if (initialize_solenoid(SOLENOID_GPIO_PIN) != 0) {
        printk(KERN_ALERT "Failed to initialize solenoid lock\n");
        cleanup_nfc();
        return -EIO;
    }
    return 0;
}

static void __exit cleanup_controller_module(void) {
    printk(KERN_INFO "Cleaning up Controller Module\n");
    cleanup_nfc();
    cleanup_solenoid(SOLENOID_GPIO_PIN);
    
}

int initialize_nfc() {
    // Setup NFC hardware interfaces, configure registers, etc.
    return 0;
}

void cleanup_nfc() {
    // Disable NFC, free resources
}

int initialize_solenoid(int gpio_pin) {
    int result;
    result = gpio_request(gpio_pin, "solenoid_gpio");
    if (result) {
        printk(KERN_ERR "Unable to request GPIO %d for solenoid: %d\n", gpio_pin, result);
        return result;
    }

    gpio_direction_output(gpio_pin, 1);  // Set the GPIO as a high output, locking the solenoid
    return 0;
}

void cleanup_solenoid(int gpio_pin) {
    gpio_set_value(gpio_pin, 1);  // Ensure solenoid is locked
    gpio_free(gpio_pin);          // Release the GPIO
}

void activate_solenoid(int gpio_pin) {
    gpio_set_value(gpio_pin, 0);  // Set GPIO low to unlock
    printk(KERN_INFO "Solenoid Unlocked\n");
}

void deactivate_solenoid(int gpio_pin) {
    gpio_set_value(gpio_pin, 1);  // Set GPIO high to lock
    printk(KERN_INFO "Solenoid Locked\n");
}


void check_token_proximity(void) {
    int i, count = 0;
    for (i = 0; i < 3; i++) {
        if (tokens_detected[i]) {
            count++;
        }
    }

    if (count >= NUM_TOKENS_REQUIRED) {
        activate_solenoid(SOLENOID_GPIO_PIN);
    } else {
        deactivate_solenoid(SOLENOID_GPIO_PIN);
    }
}

// Example of an interrupt handler for NFC detection
// This is a placeholder and needs to be adapted based on actual hardware specs
irqreturn_t nfc_irq_handler(int irq, void *dev_id) {
    static unsigned long last_jiffies = 0;

    // Debounce handling (200 ms)
    if (time_before(jiffies, last_jiffies + msecs_to_jiffies(200))) {
        return IRQ_HANDLED;
    }
    last_jiffies = jiffies;

    // Example token processing logic
    read_nfc_data();  // This function would update `tokens_detected` array
    check_token_proximity();

    return IRQ_HANDLED;
}

module_init(init_controller_module);
module_exit(cleanup_controller_module);


