#!/bin/bash

SONIC="/dev/car/sr04_tun"

if [[ $# -eq 0 ]]
then
	"in_tun.sh <port number>"
	exit 0
fi

PORT=$1

echo ""
echo "Welcome. This an ingress of a sonar sensor tunnel."
echo ""

while true
do
	DIST=$(ncat -lp ${PORT})
	echo ${DIST} > ${SONIC}
	echo "RECV: ${DIST}"
done
