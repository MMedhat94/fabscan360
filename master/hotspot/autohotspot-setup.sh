#!/bin/bash
#RaspberryConnect.com - Graeme Richards
#This installer can be shared but all references to RaspberryConnect.com in this file
#and other files used by the installer should remain in place. 

#Installer version 0.74-1 (9 Feb 2022)
#Installer for AutoHotspot, AutohotspotN scripts and Static Access Point setup.
#Autohotspot: a script that allows the Raspberry Pi to switch between Network Wifi and
#an access point either at bootup or with seperate timer without a reboot.

#This installer script will alter network settings and may overwrite existing settings if allowed.
#/etc/hostapd/hostapd.conf (backup old), /etc/dnsmasq.conf (backup old), modifies /etc/dhcpcd.conf (modifies)
#/etc/sysctl.conf (modifies), /etc/network/interfaces (backup old & removes any network entries)
#PiOS 10 Buster and older use ip tables, PiOS 11 Bullseye uses nftables. 
#If nftables are detected as installed on the older PiOS then it will be used. 

#Force Access Point or Network Wifi option will only work if either autohotspot is installed and active.


#Check for PiOS or Raspbian and version.
osver=($(cat /etc/issue))
cpath="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
opt="X"
vhostapd="N" vdnsmasq="N" autoH="N"
autoserv="N" iptble="N" nftble="N"




check_reqfiles()
{	
	fstatus=0
	cd "${cpath}/config/"
	if test -f "Checklist.md5" ;then
		if ! md5sum -c --quiet Checklist.md5 ;then
			echo "one or more of the required files in the config folder are missing or have been altered"
			echo "please download the installer again from RaspberryConnect.com"
			exit
		fi
	else
		echo "The file Checklist.md5 is missing from Config folder"
		echo "Please download the installer again"
		echo "from RaspberryConnect.com"
		exit
	fi
	
}

check_installed()
{
	#check if required software is already installed
	if dpkg -s "hostapd" | grep 'Status: install ok installed' >/dev/null 2>&1; then
		vhostapd="Y"
	fi
	if dpkg -s "dnsmasq" | grep 'Status: install ok installed' >/dev/null 2>&1; then
		vdnsmasq="Y"
	fi
	#Does an Autohotspot files exist
	if ls /usr/bin/ | grep "autohotspot*" >/dev/null 2>&1 ; then
		autoH="Y"
	fi
	if ls /etc/systemd/system/ | grep "autohotspot.service" >/dev/null 2>&1 ; then
		autoserv="Y"
	fi
	if dpkg -s "iptables" >/dev/null 2>&1 ; then
		iptble="Y"
	fi
	if dpkg -s "nftables" >/dev/null 2>&1 ; then
		nftble="Y"
	fi
}

display_HS_IP() #get ip address from current active hotspot script
{
    Aserv=($(cat /etc/systemd/system/autohotspot.service 2>/dev/null| grep "ExecStart="))  #which hotspot is active?
    if [ ${Aserv: -4} = "spot" ] >/dev/null 2>&1  ;then #Direct
		ipline=($(cat /usr/bin/autohotspot | grep "ip a add")) 
		echo "Access Point IP Address for SSH and VNC: ${ipline[3]: :-3}" 
    elif [ ${Aserv: -4} = "potN" ] >/dev/null 2>&1 ;then #Internet
		ipline=($(cat /usr/bin/autohotspotN | grep "ip a add")) 
		echo "Access Point IP Address for SSH and VNC: ${ipline[3]: :-3}"
    else #Static Hotspot default IP
		echo "Access Point IP Address for ssh and VNC: 192.168.50.10"
    fi
}

remove()
{
	if systemctl -all list-unit-files hostapd.service | grep "hostapd.service enabled" ;then
		systemctl disable hostapd >/dev/null 2>&1
	fi
	if systemctl -all list-unit-files dnsmasq.service | grep "dnsmasq.service enabled" ;then
		systemctl disable dnsmasq >/dev/null 2>&1
	fi
	auto_script #Remove Autohotspot Scripts
	#Reset DHCPCD.conf
	if [ -f "/etc/dhcpcd-RCbackup.conf" ] ;then #restore backup
		mv "/etc/dhcpcd-RCbackup.conf" "/etc/dhcpcd.conf"
	else #or remove edits if no backup
		echo "Removing config from dhcpcd.conf"
		grep -vxf "${cpath}/config/dhcpcd-remove.conf" "/etc/dhcpcd.conf" > "${cpath}/config/Ndhcpcd.conf"
		mv "${cpath}/config/Ndhcpcd.conf" "/etc/dhcpcd.conf"
	fi
	hs_routing #remove routing for Static HS
	sysctl #remove port forwarding
	interface #restore backup of interfaces fle
	auto_service #remove autohotspot.service
}

check_wificountry()
{
	#echo "Checking WiFi country"
	wpa=($(cat "/etc/wpa_supplicant/wpa_supplicant.conf" | tr -d '\r' | grep "country="))
	if [ -z ${wpa: -2} ] || [[ ${wpa: -2} == *"="* ]];then
		echo "The WiFi country has not been set. This is required for the access point setup."
		echo "Please update PiOS with the wifi country using the command 'sudo raspi-config' and choose the localisation menu"
		echo "From the desktop this can be done in the menu Preferences - Raspberry Pi Configuration - Localisation" 
		echo "Once done please try again."
		echo ""
		echo "press a key to continue"
		read
	fi
}

# This line uses the grep command to check if the contents of the file located at 
# "${cpath}/config/interfaces" are not found in the /etc/network/interfaces file. The options used are:
# -v: Inverts the matching, so it returns lines that do not match.
# -x: Treats the patterns as entire lines.
# -f: Specifies the file to use as the pattern list.
# The purpose of this line is to check if the interfaces file in the system configuration 
# directory (/etc/network/interfaces) contains network settings that are also present in the interfaces 
# file located at the path specified by ${cpath}/config/interfaces.
interface()
{
	#if interfaces file contains network settings
	#backup and remove. 
	if grep -vxf "${cpath}/config/interfaces"  "/etc/network/interfaces" ;then
		mv "/etc/network/interfaces" "/etc/network/RCbackup-interfaces"
		cp "${cpath}/config/interfaces" "/etc/network/interfaces"
	fi
	if [ "$opt" = "REM" ] ;then
		if [ -f "/etc/network/RCbackup-interfaces" ] ;then
			mv "/etc/network/RCbackup-interfaces" "/etc/network/interfaces"
		fi
	fi
}
# The dnsmasq_config() function appears to configure and manage the dnsmasq service on a Linux system.
# dnsmasq is a lightweight DNS and DHCP server commonly used for various network services.
dnsmasq_config()
{
	echo "Dnsmasq Config"
	if [ "$vdnsmasq" = "N" ]; then
		apt -q install dnsmasq
		check_installed
		if [ "$vdnsmasq" = "N" ]; then
		    echo ""
		    echo ""
			echo "dnsmasq failed to install. Check there is internet access"
			echo "and try again"
			echo "Press a key to continue"
			read
			menu
		fi
	fi
	if [ -f "/etc/dnsmasq.conf" ] ; then
		if ! grep -F "RaspberryConnect.com" "/etc/dnsmasq.conf" ;then
			#not a autohotspot file, create backup
			mv "/etc/dnsmasq.conf" "/etc/dnsmasq-RCbackup.conf"
		fi
	fi
	if [ "$opt" = "SHS" ] ;then
		cp "${cpath}/config/dnsmasqSHS.conf" "/etc/dnsmasq.conf"
	fi
	if [ "$opt" = "SHS" ]; then
		#for Static Hotspot
		echo "Unmask & Enable Dnsmasq"
		if systemctl -all list-unit-files dnsmasq.service | grep "dnsmasq.service masked" ;then
			systemctl unmask dnsmasq >/dev/null 2>&1
		fi
		if systemctl -all list-unit-files dnsmasq.service | grep "dnsmasq.service disabled" ;then
			systemctl enable dnsmasq >/dev/null 2>&1
		fi
	fi
	if [ "$opt" = "REM" ]; then
		if [ -f "/etc/dnsmasq-RCbackup.conf" ] ; then
			mv "/etc/dnsmasq-RCbackup.conf" "/etc/dnsmasq.conf"
		fi
	fi
		
}

#This function uses the sed command to search for a line in the
# /etc/sysctl.conf file that starts with #net.ipv4.ip_forward=1 (a commented out line).
# It replaces that line with net.ipv4.ip_forward=1, effectively uncommenting it.
# This setting controls IP forwarding, allowing the system to act as
# a router for forwarding network traffic.
sysctl()
{
	if [ "$opt" = "SHS" ] ;then
		sed -i -e "/#net.ipv4.ip_forward=1/c\net.ipv4.ip_forward=1" /etc/sysctl.conf
	elif [ "$opt" = "REM" ] ;then
		sed -i -e "/net.ipv4.ip_forward=1/c\#net.ipv4.ip_forward=1" /etc/sysctl.conf
	fi
}

# In summary, this function is used to manage the dhcpcd configuration file. It creates a backup of the original configuration,
# and when the option is set to "SHS," it replaces the dhcpcd.conf with a clean default configuration, removes certain configurations,
# and adds new configurations based on other files. This type of configuration management is often used to set up network-related settings,
# particularly when configuring a static hotspot on a Raspberry Pi.

dhcpcd_config()
{
	#Make backup if not done
	if [ ! "/etc/dhcpcd-RCbackup.conf" ] ;then
		mv "/etc/dhcpcd.conf" "/etc/dhcpcd-RCbackup.conf"
	fi
	if [ "$opt" = "SHS" ]; then
		#use clean dhcpcd.conf for static hotspot, backup will be restored on removal /etc/dhcpcd-RCbackup.conf 
		cp "${cpath}/config/dhcpcd-default.conf" "/etc/dhcpcd.conf"
		grep -vxf "${cpath}/config/dhcpcd-remove.conf" "/etc/dhcpcd.conf" > "${cpath}/config/Ndhcpcd.conf"
		cat "${cpath}/config/dhcpcd-SHSN.conf" >> "${cpath}/config/Ndhcpcd.conf"
		mv "${cpath}/config/Ndhcpcd.conf" "/etc/dhcpcd.conf"
	fi
}

auto_service()
{
	if [ "$opt" = "REM" ] || [ "$opt" = "SHS" ]; then
		if systemctl -all list-unit-files autohotspot.service | grep "autohotspot.service enabled" ;then
			systemctl disable autohotspot.service
		fi
		if [ -f "/etc/systemd/system/autohotspot.service" ]; then
			rm /etc/systemd/system/autohotspot.service
		fi
	fi

}

hostapd_config()
{
	echo "hostapd Config"
	echo "Hostapd Status is " $vhostapd
	if [ "$vhostapd" = "N" ]; then
		echo "Hostapd not installed- now installing"
		apt -q install hostapd
		echo "Recheck install Status"
		check_installed
		if [ "$vhostapd" = "N" ]; then
			echo ""
			echo ""
			echo "Hostapd failed to install. Check there is internet access"
			echo "and try again"
			echo "Press a key to continue"
			read
			menu
		fi
	fi
	echo "Hostapd is installed"
	if ! grep -F "RaspberryConnect.com" "/etc/hostapd/hostapd.conf" ;then
		#not a autohotspot file, create backup
		mv "/etc/hostapd/hostapd.conf" "/etc/hostapd/hostapd-RCbackup.conf"
	fi
	cp "$cpath/config/hostapd.conf" /etc/hostapd/hostapd.conf
	if [ "${osver[2]}" -lt 10 ]; then
		cp "$cpath/config/hostapd" /etc/default/hostapd
	fi
	if [ "$opt" = "SHS" ]; then
		#for Static Hotspot
		echo "Unmask and enable hostapd"
		if systemctl -all list-unit-files hostapd.service | grep "hostapd.service masked" ;then
			systemctl unmask hostapd >/dev/null 2>&1
		fi
		if systemctl -all list-unit-files hostapd.service | grep "hostapd.service disabled" ;then
			systemctl enable hostapd >/dev/null 2>&1
		fi
	elif [ "$opt" = "REM" ]; then
		if [ -f "/etc/hostapd/hostapd-RCbackup.conf" ] ; then
			mv "/etc/hostapd/hostapd-RCbackup.conf" "/etc/hostapd/hostapd.conf"
		fi
	fi
	#check country code for hostapd.conf
	wpa=($(cat "/etc/wpa_supplicant/wpa_supplicant.conf" | tr -d '\r' | grep "country="))
	hapd=($(cat "/etc/hostapd/hostapd.conf" | tr -d '\r' | grep "country_code="))
	if [[ ! ${wpa: -2} == ${hapd: -2} ]] ; then
		echo "Changing Hostapd Wifi country to " ${wpa: -2} 
		sed -i -e "/country_code=/c\country_code=${wpa: -2}" /etc/hostapd/hostapd.conf
	fi
}

# In summary, this function is responsible for configuring and managing routing settings for a Raspberry Pi hotspot,
# with options for both IPTables and nftables routing methods. It sets up and enables routing services when configuring
# the hotspot and disables and removes them when removing the hotspot configuration, based on the specified options and 
# conditions.

hs_routing()
{
	if [ "$opt" = "SHS" ]  ;then
		if [ "$iptble" = "Y" ] ; then
			if [ ! -f "/etc/systemd/system/hs-iptables.service" ];then
				cp "${cpath}/config/hs-iptables.service" "/etc/systemd/system/hs-iptables.service"
			fi
			if systemctl -all list-unit-files hs-iptables.service | grep "hs-iptables.service enabled" ;then
				systemctl daemon-reload
			fi
			if systemctl -all list-unit-files hs-iptables.service | grep "hs-iptables.service disabled" ;then
				systemctl enable hs-iptables.service
			fi
			if [ ! -f "/etc/iptables-hs" ] ;then
				cp "${cpath}/config/iptables-hs.txt" "/etc/iptables-hs"
				chmod +x "/etc/iptables-hs"
			fi
			
		elif [ "$nftble" = "Y" ] ; then
			if [ ! -d '/etc/nftables' ] ; then
				mkdir /etc/nftables
			fi
			if ! cat '/etc/nftables.conf' | grep 'nft-stat-ap.nft' ; then
				cp "${cpath}/config/nft-stat-ap.txt" "/etc/nftables/nft-stat-ap.nft"
				chmod +x "/etc/nftables/nft-stat-ap.nft"
				sed -i '$ a include "/etc/nftables/nft-stat-ap.nft"' "/etc/nftables.conf"
				if systemctl -all list-unit-files nftables.service | grep "nftables.service disabled" ;then
					systemctl enable nftables >/dev/null 2>&1
				fi
			fi	
		fi
	elif [ "$opt" = "REM" ] ; then
		if [ "$iptble" = "Y" ] ; then
			if systemctl is-active hs-iptables | grep -w "active" ;then
				systemctl disable hs-iptables.service
			fi
			if test -f "/etc/systemd/system/hs-iptables.service" ; then
				rm /etc/systemd/system/hs-iptables.service
			fi
			if test -f "/etc/iptables-hs" ; then
				rm /etc/iptables-hs
			fi
		elif [ "$nftble" = "Y" ] ; then
			sed -i '/nft-stat-ap/d' '/etc/nftables.conf'			
		fi
	fi
}

auto_script()
{
	if [ "$opt" = "REM" ] || [ "$opt" = "SHS" ] ;then
		if [ -f "/usr/bin/autohotspotN" ]; then
			rm /usr/bin/autohotspotN
		fi
		if [ -f "/usr/bin/autohotspot" ]; then
			rm /usr/bin/autohotspot
		fi		
	fi
}

go()
{
	opt="$1"
	#echo "Selected" "$opt"
	#echo "Action options"
	if [ "$opt" = "REM" ] ;then
		remove
		echo "Please reboot to complete the uninstall"
	elif [ "$opt" = "SSI" ] ;then
		setupssid
		echo "the new ssid will be used next time the autohotspot script is "
		echo "run at boot or manually otherwise use the Force to.... option"
		echo "if the hotspot is active"
	elif [ "$opt" = "HSS" ] ;then
		Hotspotssid
	else
		hostapd_config
		dnsmasq_config
		interface
		sysctl
		dhcpcd_config
		auto_service
		hs_routing
		auto_script
		echo ""
		echo "The hotspot setup will be available after a reboot"
		HSssid=($(cat "/etc/hostapd/hostapd.conf" | grep '^ssid='))
		HSpass=($(cat "/etc/hostapd/hostapd.conf" | grep '^wpa_passphrase='))
		echo "The Hotspots WiFi SSID name is: ${HSssid: 5}"
		echo "The WiFi password is: ${HSpass: 15}"
		display_HS_IP
	fi
	echo "Press any key to continue"
	read
	
}

menu()
{
	#selection menu
	clear
	echo "Raspberryconnect.com Autohotspot installation and setup"
	echo "for installation or switching between access point types"
	echo "or uninstall the access point back to standard Pi wifi"
	echo ""
	echo "Autohotspot Net = connects to a known wifi network in range,"
	echo "otherwise automatically creates a Raspberry Pi access point with network/internet access if an"
	echo "ethernet cable is connected. Uses wlan0, eth0. Pi's 3,3+,4"
	echo ""
	echo "Autohotspot NO Net = as above but connected devices to the access point"
	echo "will NOT get a network/internet connection if an ethernet cable is connected. Rpi Zero W & RPi Zero 2"
	echo ""
	echo "Permanent Access Point = permanent access point with network/internet access from eth0 for"
	echo "connected devices"
	echo ""
	echo " 3 = Install a Permanent Access Point with eth0 access for connected devices"
	echo " 4 = Uninstall Autohotspot or permanent access point"
	echo " 5 = Add a new wifi network to the Pi (SSID) or update the password for an existing one."
	echo " 7 = Change the access points SSID and password"
	echo " 8 = Exit"
	echo ""
	
	if [ $# -eq 1 ]; then
		select="$1"
	else
		echo -n "Select an Option:"
		read select
	fi
	case $select in
		3) clear ; go "SHS" ;; #Static Hotspot
		4) clear ; go "REM" ;; #Remove Autohotspot or Static Hotspot
		5) clear ; go "SSI" ;; #Change/Add Wifi Network
		7) clear ; go "HSS" ;; #Change Hotspot SSID and Password
		8) clear ; exit ;;
		*) clear; echo "Please select again";;
	esac
}

#check_reqfiles
check_installed
check_wificountry
menu #show menu