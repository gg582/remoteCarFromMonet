#!/bin/bash
source /home/pi/remote-car/conf-exec.sh ;exec /home/pi/remote-car/car/controller/sendSensorValueToVirtCar & exec /home/pi/remote-car/car/controller/RealCarRunner &
