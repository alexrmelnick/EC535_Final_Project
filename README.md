# EC535_Final_Project

## Project Summary:
The project intends to program a beagleboard kernel module to interact with an antenna, NFC tokens, and
a solenoid lock. The antenna should respond/recognize different tokens and interact with the solenoid
when the 2 of the 3 NFC tokens are in close proximity.

## Materials:
- Beagleboard (1x)
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
| MFRC522 | BeagleBoard        |
|---------|--------------------|
| SDA     | P9_17 (SPIO_CSO)   |
| SCK     | P9_22 (SPIO_SCLK)  |
| MOSI    | P9_18 (SPIO_D1)    |
| MISO    | P9_21 (SPIO_DO)    |
| IRQ     | P8_12 (GPIO_44)    |
| GND     | P9_45 (DGND)       |
| RST     | P8_10 (GPIO_68)    |
| 3.3V    | P9_03 (VDD 3.3 V)  |
|---------|--------------------|
|Solenoid |                    |
|---------|--------------------|
| 4.5V    | P9_05 (VDD 5V)     |
| GND     | P9_46 (DGND)       |
| Control | P8_14 (GPIO_26)    |

## Further Reading
- https://www.kernel.org/doc/html/v4.9/driver-api/spi.html
- https://linuxtv.org/downloads/v4l-dvb-internals/device-drivers/API-struct-spi-board-info.html 
- https://youtu.be/Yh3TLXihUps?si=wRjqsa_WysqMT6_W - Solenoid wiring tutorial
- https://embetronicx.com/tutorials/linux/device-drivers/linux-kernel-spi-device-driver-tutorial/ 
- https://embetronicx.com/tutorials/tech_devices/spi-serial-peripheral-interface-protocol-basics/

- https://embetronicx.com/tutorials/tech_devices/i2c_1/ 
- https://embetronicx.com/tutorials/tech_devices/i2c_2/
- https://embetronicx.com/tutorials/linux/device-drivers/i2c-linux-device-driver-using-raspberry-pi/ 