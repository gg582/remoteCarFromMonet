#!/bin/bash

source /home/pi/remote-car/conf-exec.sh

echo "motorRelayInPod begins."
/home/pi/remote-car/car/controller/motorRelayInPod $K8S_CAR_ADDR  &

while true
do
	sleep 1
done
