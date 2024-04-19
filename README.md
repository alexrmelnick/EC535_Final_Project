# EC535_Final_Project

# Parts
- 1x NXP MFRC52202HN1 (in documentation as "version 2.0")
- 3x S50 NFC tags
- 1x uxcell a14092600ux0438 Linear Solenoid
- 1x transistor (from lab)

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