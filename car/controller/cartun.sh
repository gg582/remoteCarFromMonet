#!/bin/bash
source /home/pi/remote-car/conf-exec.sh ;/home/pi/remote-car/car/controller/sensorRelayInPod $K8S_CAR_ADDR &/home/pi/remote-car/car/controller/motorRelayInPod $K8S_CAR_ADDR 
