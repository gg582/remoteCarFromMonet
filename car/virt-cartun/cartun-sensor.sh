#!/bin/bash

echo "sensorRelayInPod begins."
/home/pi/remote-car/car/virt-cartun/sensorRelayInPod $K8S_CAR_ADDR &

while true
do
	sleep 1
done
