# EC535_Final_Project

## Project Summary:
The project intends to program a beagleboard kernel module to interact with an antenna, NFC tokens, and
a solenoid lock. The antenna should respond/recognize different tokens and interact with the solenoid
when the 2 of the 3 NFC tokens are in close proximity.

## Project Video:
https://youtu.be/ctBJeuU8Y2I

## Materials:
- Beaglebone Black (1x)
- Antenna Module (NXP MFRC52202HN1) (1x)
- NFC Tokens (S50) (3x) 
- Linear Solenoid (uxcell a14092600ux0438) (1x) 

## Modules necessary:
- Main kernel module
- Programming module for tags
- Detector module for antenna to tag
- Expand detector module to include detecting different tags
- Expand to detect different tags simultaneously
- Module to operate solenoid (Lock)
- Userspace program (potentially)

## Pinout
| BeagleBone Black   | MFRC522 | PN532     | Solenoid |
|--------------------|---------|-----------|----------|
| P9_17 (SPIO_CSO)   | SDA     |           |          |
| P9_22 (SPIO_SCLK)  | SCK     |           |          |
| P9_18 (SPIO_D1)    | MOSI    |           |          |
| P9_21 (SPIO_DO)    | MISO    |           |          |
| P8_12 (GPIO_44)    | IRQ     |           |          |
| P9_45 (DGND)       | GND     |           |          |
| P8_10 (GPIO_68)    | RST     |           |          |
| P9_03 (VDD 3.3 V)  | 3.3V    |           |          |
|--------------------|---------|-----------|----------|
|                    |         | SCK       |          |
|                    |         | M         |          |
| P9_20 (I2C_SDA)    |         | MO/SDA/TX |          |
| P9_19 (I2C2_SLC)   |         | NSS/SCL   |          |
| P8_12 (GPIO_44)    |         | IRO       |          |
| P8_10 (GPIO_68)    |         | RST       |          |
| P9_45 (DGND)       |         | GND       |          |
| P9_06 (VDD 5V)     |         | 5V        |          |
|--------------------|---------|-----------|----------|
| P9_05 (VDD 5V)     |         |           | Power    |
| P9_46 (DGND)       |         |           | Ground   |
| P8_14 (GPIO_26)    |         |           | Gate     |

## Setup
### PN532
- SET0 -> H
- SET1 -> L

### Solenoid
- Connect solenoid to across the diode
- Connect the GPIO to the transistor gate in series with the resistor
- Connect 5V and GND to breadboard

## Further Reading
- https://www.kernel.org/doc/html/v4.9/driver-api/spi.html
- https://linuxtv.org/downloads/v4l-dvb-internals/device-drivers/API-struct-spi-board-info.html 
- https://youtu.be/Yh3TLXihUps?si=wRjqsa_WysqMT6_W - Solenoid wiring tutorial
- https://embetronicx.com/tutorials/linux/device-drivers/linux-kernel-spi-device-driver-tutorial/ 
- https://embetronicx.com/tutorials/tech_devices/spi-serial-peripheral-interface-protocol-basics/

- https://embetronicx.com/tutorials/tech_devices/i2c_1/ 
- https://embetronicx.com/tutorials/tech_devices/i2c_2/
- https://embetronicx.com/tutorials/linux/device-drivers/i2c-linux-device-driver-using-raspberry-pi/ 
- https://embetronicx.com/tutorials/linux/device-drivers/i2c-bus-driver-dummy-linux-device-driver-using-raspberry-pi/
- https://docs.kernel.org/i2c/writing-clients.html 
- https://unboxnbeyond.wordpress.com/2020/06/12/i2c-communication-in-beagleboneblack/
- https://onion.io/2bt-digging-into-i2cget-and-i2cset/
