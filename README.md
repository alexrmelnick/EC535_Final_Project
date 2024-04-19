# EC535_Final_Project

Project Summary:
The project intends to program a beagleboard kernel module to interact with an antenna, NFC tokens, and
a solenoid lock. The antenna should respond/recognize different tokens and interact with the solenoid
when the 2 of the 3 NFC tokens are in close proximity.

Materials:
- Beagleboard (1)
- Antenna Module (NXP MFRC52202HN1) (1)
- NFC Tokens (S50) (3) 
- Linear Solenoid (uxcell a14092600ux0438) (1)

Modules neccessary:
- Main kernel module
- Programming module for tags
- Detector module for antenna to tag
- Expand detector module to include detecting different tags
- Expand to detect different tags simultaneously
- Module to operate solenoid (Lock)
- Userspace program (potentially)

# Pinout
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
