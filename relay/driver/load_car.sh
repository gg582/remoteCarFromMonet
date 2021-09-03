#!/bin/bash

source ./common.sh

mkdir -p /dev/${module}

if grep -q '^staff:' /etc/group; then
    group="staff"
else
    group="wheel"
fi

/sbin/insmod ./$module.ko $* || exit 1

# retrieve major number
major=$(awk "\$2==\"$module\" {print \$1}" /proc/devices)

#
# The exits of the tunnels are prepared
#
# Remove all devices
#
# Remove all devices for generating new devices
#

for i in `seq 0 ${len}` 
	do rm -f /dev/${names[i]}
done

for i in `seq 0 ${len}`
do
	let minor=$(expr ${len}+${i}+1)
	mknod /dev/${names[i]} c $major ${minor}
	chgrp $group /dev/${names[i]}
	chmod $mode  /dev/${names[i]}
done

let minor=$(expr ${nr_devs}+${nr_devs})

mknod /dev/${motor} c $major $minor

#
# The entrances of the tunnels are prepared
#
# Remove all devices for generating new devices
#

for i in `seq 0 ${len}` 
	do rm -f /dev/${names[i]}${output}
done

for i in `seq 0 ${len}`
do
	ln -s /dev/${names[i]} /dev/${names[i]}${output}
done

ln -s /dev/${motor} /dev/${motor}${output}

