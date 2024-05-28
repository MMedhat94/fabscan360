#!/bin/bash

# Line to add to sshd_config
line="ClientAliveInterval 120"
commented_line="#ClientAliveInterval 120"
# Uncomment if already commented
sed -i "s/^$commented_line/$line/" /etc/ssh/sshd_config
# Add if not present (commented or uncommented)
grep -q "$line" /etc/ssh/sshd_config || echo "$line" >> /etc/ssh/sshd_config
# Print a message
echo "Line '$line' enabled in /etc/ssh/sshd_config (if not already present)."

# Line to add to sshd_config
line="ClientAliveCountMax 720"
commented_line="#ClientAliveCountMax 720"
# Uncomment if already commented
sed -i "s/^$commented_line/$line/" /etc/ssh/sshd_config
# Add if not present (commented or uncommented)
grep -q "$line" /etc/ssh/sshd_config || echo "$line" >> /etc/ssh/sshd_config
# Print a message
echo "Line '$line' enabled in /etc/ssh/sshd_config (if not already present)."

# Check if access token and username are provided
if [ $# -ne 3 ]; then
    echo "Usage: $0 <Wifi_password> <username> <access_token>"
    exit 1
fi

# Download Git and Wifi needed modules
sudo apt-get install git -y
sudo apt-get install hostapd -y
sudo apt-get install dnsmasq -y
sudo apt-get install iptables -y
sudo apt-get install nftables -y
sudo apt install libcurl4-openssl-dev -y
sudo apt install libgtk-3-dev  -y
sudo apt-get install accountsservice -y
# Update package lists
#sudo apt-get update

# Upgrade packages without prompting (use with caution)
#sudo DEBIAN_FRONTEND=noninteractive apt-get upgrade -y

# Modify /etc/rc.local (inserting before "exit 0")
sed -i'' '$i sudo iw wlan0 set power_save off' /etc/rc.local

# Print a success message
echo "Script completed. Remember to review /etc/rc.local before reboot."

# Assign command line arguments to variables
access_token=$3
username=$2
wifi="$1"
# Set up the git configuration with the access token
git config --global credential.helper store
git config --global credential.helper "cache --timeout=3600"
git config --global user.name "$username"
git config --global user.email "$username@gmail.com"

# Clone the private repository using the access token
git clone https://$username:$access_token@github.com/$username/rpb_camera.git

# Reset git configuration after cloning
git config --global --unset credential.helper
git config --global --unset user.name
git config --global --unset user.email

sudo chmod -R 777 ./rpb_camera

# Set the time zone for accurate date to be used in logging
timedatectl set-timezone Europe/Helsinki

#Sets the Wifi country to Finland
raspi-config nonint do_wifi_country FI

# Remove the cursor from the screen
sudo sed -i -- "s/#xserver-command=X/xserver-command=X -nocursor/" /etc/lightdm/lightdm.conf

#Remove the task bar
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

#Remove the recycle bin icon
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

# Disable Wifi autoconnect
if grep -q "ssid=" /etc/wpa_supplicant/wpa_supplicant.conf; then
    # If the network block exists, add disabled=1 to it
    sudo sed -i '/ssid=/a \ \tdisabled=1' /etc/wpa_supplicant/wpa_supplicant.conf
fi

#Set wifi password from the first argument
file_path="./rpb_camera/master/hotspot/config/hostapd.conf"
sed -i "s/wpa_passphrase=[0-9]*/wpa_passphrase=$wifi/" "$file_path"

# Start the hotspot
./rpb_camera/master/hotspot/autohotspot-setup.sh 3


cd rpb_camera/master/FSM
make
sudo cp ./app.service /etc/systemd/system/
sudo systemctl enable app.service
sudo systemctl start app.service
