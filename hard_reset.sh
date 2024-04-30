#!/bin/bash

# Save the current directory
original_dir=$(pwd)

# Navigate to the GPIO system directory
cd /sys/class/gpio

# Export GPIO20 to userspace
echo 20 > export

# Set GPIO20 as an output
echo out > gpio20/direction

# Set GPIO20's value to 0 (LOW)
echo 0 > gpio20/value

# Wait for 1 second
sleep 1

# Set GPIO20's value to 1 (HIGH)
echo 1 > gpio20/value

# Print completion message
echo "Hard reset complete"

# Return to the original directory
cd "$original_dir"