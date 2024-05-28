#!/bin/bash

# Define the rule
RULE='SUBSYSTEM=="usb", ATTR{idVendor}=="16d0", ATTR{idProduct}=="1557", MODE="0666"'

# Check if the file already exists
RULE_FILE="/etc/udev/rules.d/99-com.rules"
if [ -f "$RULE_FILE" ]; then
    # Append the rule to the existing file
    echo "$RULE" | sudo tee -a "$RULE_FILE" > /dev/null
    echo "Rule appended to $RULE_FILE"
else
    # Create a new file with the rule
    echo "$RULE" | sudo tee "$RULE_FILE" > /dev/null
    echo "Rule added to $RULE_FILE"
fi

# Reload udev rules
sudo udevadm control --reload-rules
sudo udevadm trigger

echo "Udev rules reloaded"

modprobe -r cdc_acm
