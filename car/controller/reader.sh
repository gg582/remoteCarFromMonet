#!/bin/bash

source ../../conf-exec.sh

SEND=ncat

echo "Welcome. This is a real car."
echo "The virtual car has an IP: ${POD_CAR_ADDR} and its port : ${UP_PORT}"

while true
do
	DIST=$(./sonic)
	echo "SEND: ${DIST}"
	echo ${DIST} | ${SEND} ${POD_CAR_ADDR} ${UP_PORT}
done
