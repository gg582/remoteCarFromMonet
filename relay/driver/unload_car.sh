#!/bin/bash

source ./common.sh

# invoke rmmod with all arguments we got
rmmod ${module}
# Remove stale nodes
for i in `seq 0 ${len}`
	do rm -f /dev/${names[i]} /dev/${names[i]}${output} 
done

sudo rm -rf /dev/${module}
