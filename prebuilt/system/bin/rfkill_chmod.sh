#!/bin/sh

for i in 0 1 2 3 4 5  
do
	if [ -e "/sys/class/rfkill/rfkill$i/type" ]
	then
		if [ `cat /sys/class/rfkill/rfkill$i/type` == "wwan" ]
		then
			chown 1000:1001 /sys/class/rfkill/rfkill$i/state
			chmod 660 /sys/class/rfkill/rfkill$i/state
		fi
	fi
done
if [ -e "/sys/bus/usb/devices/1-1/power/control" ]
then
	chown 1000:1001 /sys/bus/usb/devices/1-1/power/control
	chmod 664 /sys/bus/usb/devices/1-1/power/control
fi
