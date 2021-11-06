#!/bin/bash

source /home/pi/remote-car/conf-exec.sh

echo "sensorRelayInPod begins."
/home/pi/remote-car/car/controller/sensorRelayInPod $K8S_CAR_ADDR &

while true
do
	sleep 1
done
