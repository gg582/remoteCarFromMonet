#!/bin/bash

echo "In real car, sendSensorValue begins."

/home/pi/remote-car/car/real-car/sendSensorValueToVirtCar &

while true
do
	sleep 1
done
