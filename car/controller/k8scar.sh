#!/bin/bash
source /home/pi/remote-car/conf-exec.sh

echo "In real car, runner begins."

/home/pi/remote-car/car/runner/runner &

while true
do
	sleep 1
done
