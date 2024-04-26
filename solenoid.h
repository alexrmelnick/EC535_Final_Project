#ifndef SOLENOID_H
#define SOLENOID_H

int initialize_solenoid(int gpio_pin);
void cleanup_solenoid(int gpio_pin);
void activate_solenoid(int gpio_pin);
void deactivate_solenoid(int gpio_pin);

#endif // SOLENOID_H
