#!/bin/bash

source ../../conf-exec.sh

SEND=ncat

while true
do
	DIST=$(./sonic)
	echo "SEND: ${DIST}"
	echo ${DIST} | ${SEND} ${POD_POD_ADDR} ${UP_PORT}
done
