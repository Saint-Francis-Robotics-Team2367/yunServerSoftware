#/bin/sh
echo "::sysinit:/etc/init.d/rcS S boot
::shutdown:/etc/init.d/rcS K shutdown
#ttyATH0::askfirst:/bin/ash --login
" > /etc/inittab

opkg update
opkg install python pyserial unzip
mkdir /home
mkdir /home/robotics
cd /home/robotics
rm *.zip
wget https://github.com/Saint-Francis-Robotics-Team2367/yunServerSoftware/archive/master.zip --no-check-certificate
unzip master.zip

echo "# Put your custom commands here that should be executed once
# the system init finished. By default this file does nothing.
wifi-live-or-reset
boot-complete-notify
# Uncomment the following line in order to reset the microntroller
# right after linux becomes ready
#reset-mcu
# Uncomment the following line in order to disable kernel console
# debug messages, thus having a silent and clean serial communication
# with the microcontroller
#echo 0 > /proc/sys/kernel/printk
python /home/robotics/yunServerSoftware-master/src/server.py
exit 0" > /etc/rc.local
