#!/bin/bash

# Line to add to sshd_config
#line="AllowTcpForwarding  = yes"
#commented_line="#AllowTcpForwarding  = yes"
# Uncomment if already commented
#sed -i "s/^$commented_line/$line/" /etc/ssh/sshd_config
# Add if not present (commented or uncommented)
#grep -q "$line" /etc/ssh/sshd_config || echo "$line" >> /etc/ssh/sshd_config
# Print a message
#echo "Line '$line' enabled in /etc/ssh/sshd_config (if not already present)."

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
if [ $# -ne 2 ]; then
    echo "Usage: $0 <access_token> <username>"
    exit 1
fi

# Download Git
sudo apt-get install git -y
# Update package lists
#sudo apt-get update

# Upgrade packages without prompting (use with caution)
#sudo DEBIAN_FRONTEND=noninteractive apt-get upgrade -y

# Modify /etc/rc.local (inserting before "exit 0")
sed -i'' '$i sudo iw wlan0 set power_save off' /etc/rc.local

# Print a success message
echo "Script completed. Remember to review /etc/rc.local before reboot."

# Assign command line arguments to variables
access_token=$2
username=$1

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
cd rpb_camera/slave/app
make
sudo cp ./app.service /etc/systemd/system/
sudo systemctl enable app.service
sudo systemctl start app.service