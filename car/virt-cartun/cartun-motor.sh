#!/bin/bash

echo "motorRelayInPod begins."
/home/pi/remote-car/car/virt-cartun/motorRelayInPod $K8S_CAR_ADDR  &

while true
do
	sleep 1
done
