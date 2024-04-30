#!/bin/bash

# I2C bus number
bus=2
# I2C device address
device_address=0x24

# Loop through all possible 8-bit addresses
i=0
while [ $i -le 255 ]; do
    # Format as a hexadecimal number with leading zero
    reg_address=$(printf "0x%02X" $i)
    # Run i2cget command
    result=$(i2cget -y $bus $device_address $reg_address)
    # Check result
    if [ "$result" != "0x00" ]; then
        echo "Non-zero value found: $result at register $reg_address"
        break
    fi
    i=$((i+1))
done

echo "Scan complete."
