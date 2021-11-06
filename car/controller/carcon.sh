#!/bin/bash
source /home/pi/remote-car/conf-exec.sh

echo "In carcon, runner begins."

/home/pi/remote-car/car/runner/runner &
