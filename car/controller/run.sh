#!/bin/bash

echo "In real car, RealCarRunner begins."
/home/pi/remote-car/car/controller/RealCarRunner &

echo "In real car, sendSensorValue begins."
/home/pi/remote-car/car/controller/sendSensorValueToVirtCar &
