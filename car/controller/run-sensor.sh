#!/bin/bash

echo "In real car, sendSensorValue begins."

/home/pi/remote-car/car/controller/sendSensorValueToVirtCar &

while true
do
	sleep 1
done
