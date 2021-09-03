#!/bin/bash

SEND=ncat
SONIC="/dev/car/sr04"
RELAY="192.168.0.63"
PORT=10102

while true
do
	DIST=$(./sonic)
	echo "SEND: ${DIST}"
	echo ${DIST} | ${SEND} ${RELAY} ${PORT}
done
