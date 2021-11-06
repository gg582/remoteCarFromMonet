#!/bin/bash

echo "In real car, RealCarRunner begins."

/home/pi/remote-car/car/controller/RealCarRunner &

while true
do
	sleep 1
done
