#!/bin/bash

# Path to the autostart file
autostart_file="/etc/xdg/lxsession/LXDE-pi/autostart"

# Check if the autostart file exists
if [ -f "$autostart_file" ]; then
    # Check if the line is already commented out
    if grep -q "^#@lxpanel --profile LXDE-pi" "$autostart_file"; then
        echo "Line is already commented out."
    else
        # Comment out the line by adding "#" before it
        sed -i 's/^@lxpanel --profile LXDE-pi/#@lxpanel --profile LXDE-pi/' "$autostart_file"
        echo "Line commented out successfully."
    fi
else
    echo "Autostart file not found: $autostart_file"
    exit 1
fi
