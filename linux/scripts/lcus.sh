#! /bin/sh

# $Id$

serdev="/dev/serial/relay"
n="$1"
op="$2"

[  -c $serdev  ] || { echo "No serial device found!";  exit 1; }

realpath  $serdev | grep -q ttyUSB || {
	sudo sh -c "udevadm control --reload && udevadm trigger;"
	rm -f /tmp/.relayA /tmp/.relayB
	sleep 3
	realpath  $serdev | grep -q ttyUSB ||  { echo "Serial device setup failed!";  exit 1; }
}

echo -n "Used relay on "; realpath  $serdev | grep  ttyUSB 
[ $# -lt 1 ] && {

	echo "Channels:  A:$(cat /tmp/.relayA 2>/dev/null) B:$(cat /tmp/.relayB 2>/dev/null)"
	exit 1
}

nv=""
echo "0 A a" | grep -q $n && nv=A
echo "1 B b" | grep -q $n && nv=B
[ $nv ] || exit 1

last=$(cat /tmp/.relay$nv 2>/dev/null)
ov=""
[ $# -ge 2 ] && {
	echo "ON on On U u 1" | grep -q $op && ov=ON
	echo "OF of  Of D d 0" | grep -q $op && ov=OFF
}	
[ "$ov" = "" ] && {
	echo "Channel: $nv state: ${last}"
	exit 0;
}

echo "Channel: $nv ${last} => ${ov}"
echo ${ov} > /tmp/.relay${nv}

A1="A0 01 01 A2"
A0="A0 01 00 A1"
B1="A0 02 01 A3"
B0="A0 02 00 A2"

[ "$nv" = "A" ] && [ "$ov" = "ON" ] && VAL=$A1
[ "$nv" = "A" ] && [ "$ov" = "OFF" ] && VAL=$A0
[ "$nv" = "B" ] && [ "$ov" = "ON" ] && VAL=$B1
[ "$nv" = "B" ] && [ "$ov" = "OFF" ] && VAL=$B0

do_cmd() { echo $VAL | xxd -r -p >/dev/serial/relay;  sleep 1;  echo $VAL | xxd -r -p >/dev/serial/relay; }
do_cmd &

# UDEV rule:
# SUBSYSTEMS=="usb", ATTRS{idProduct}=="7523", ATTRS{idVendor}=="1a86",  ATTRS{bmAttributes}=="80", ATTRS{bDeviceClass}=="ff",  SYMLINK+="serial/relay"

