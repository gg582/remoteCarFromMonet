#!/bin/bash

source ../../conf-exec.sh

SONIC="/dev/car/sr04_tun"

if [[ $# -eq 0 ]]
then
	PORT=${UP_PORT}
else
	PORT=$1
fi


echo ""
echo "Welcome. This an ingress of a sonar sensor tunnel."
echo "We open a port of ${PORT}."
echo ""

while true
do
	DIST=$(ncat -lp ${PORT})
	echo ${DIST} | ./toss ${SONIC}
	echo "INGRESS: ${DIST}"
done
