#!/bin/bash

source ../../conf-exec.sh

while true
do
	echo "Waiting command ..."

	cmd=$(ncat -lp ${DOWN_PORT})

	case $cmd in
	"STOP")
		echo "CMD: stop"
		./stop
		;;
	"LEFT")
		echo "CMD: left"
		./left
		;;
	"RGHT")
		echo "CMD: right"
		./right
		;;
	"FWRD")
		echo "CMD: forward"
		./forward
		;;
	"BWRD")
		echo "CMD: backward"
		./backward
		;;
	"TERM")
		echo "CMD: terminate"
		break
		;;
	*) 
		echo "CMD: invalid"
		;;
	esac
done

echo "Good bye"

