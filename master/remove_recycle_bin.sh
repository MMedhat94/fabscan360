#!/bin/bash

# Path to the desktop-items-0.conf file
conf_file="/etc/xdg/pcmanfm/LXDE-pi/desktop-items-0.conf"

# Check if the desktop-items-0.conf file exists
if [ -f "$conf_file" ]; then
    # Change the show_trash setting from 1 to 0
    sed -i 's/^show_trash=1/show_trash=0/' "$conf_file"
    echo "show_trash setting changed to 0."
else
    echo "Desktop items configuration file not found: $conf_file"
    exit 1
fi
