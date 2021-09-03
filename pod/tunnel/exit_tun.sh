#!/bin/bash

SEND=ncat

MOTOR_TUNNEL="/dev/car/motor_tun"

TARGET=$1
PORT=$2

STOP=stop
LEFT=left
RGHT=right
FWRD=forward
BWRD=backward
TERM=terminate

echo ""
echo "Welcome. This an exit of a tunnel for a remote car."
echo "The remote car has an IP: ${TARGET}"
echo ""

while true
do
	echo "Waiting command..."

	CMD=`./catd`
	
	echo "Waiting command... ${CMD}"

	case ${CMD} in
	"STOP")
		echo "CMD: stop"
		echo "${SEND} ${TARGET} ${PORT} < ${STOP}"
		${SEND} ${TARGET} ${PORT} < ${STOP}
		;;
	"LEFT")
		echo "CMD: left"
		${SEND} ${TARGET} ${PORT} < ${LEFT}
		;;
	"RGHT")
		echo "CMD: right"
		${SEND} ${TARGET} ${PORT} < ${RGHT}
		;;
	"FWRD")
		echo "CMD: forward"
		${SEND} ${TARGET} ${PORT} < ${FWRD}
		;;
	"BWRD")
		echo "CMD: backward"
		${SEND} ${TARGET} ${PORT} < ${BWRD}
		;;
	"TERM")
		${SEND} ${TARGET} ${PORT} < ${TERM}
		break
		;;
	*)
		echo "CMD: invalid"
	esac
done

echo "Good bye"
