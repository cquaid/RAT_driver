#!/bin/bash

# This script generates a udev rule file and places it in the proper directory

vid=$1
pid=$2
execpath=$3

command="KERNEL==\"mouse?\", ATTRS{idProduct}==\"$pid\", ATTRS{idVendor}==\"$vid\", RUN:=\"$execpath\""


if [ ! -e "$execpath" ]; then
	echo "$execpath does not exist"
	exit
fi

if [ -d "/etc/udev/rules.d/" ]; then
	path="/etc/udev/rules.d/"
elif [ -d "/lib/udev/rules.d/" ]; then
	path="/lib/udev/rules.d"
else
	echo "failed to find udev rules.d"
	exit
fi


fname="${path}/81-local-$vid-$pid.rules"

if [ -e "$fname" ]; then
	echo "$fname exists"
	exit
fi

echo "writing '$command' to $fname"
echo $command >> $fname
