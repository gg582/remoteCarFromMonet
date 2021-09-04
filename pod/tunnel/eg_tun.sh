#!/bin/bash

SEND=ncat

MOTOR_TUNNEL="/dev/car/motor_tun"

TARGET=$1
PORT=$2

STOP=stop
LEFT=left
RGHT=right
FWRD=forward
BKWD=backward
TERM=terminate

echo ""
echo "Welcome. This an exit of a tunnel for a remote car."
echo "The remote car has an IP: ${TARGET}"
echo ""

while true
do
	CMD=`./catd`
	
	case ${CMD} in
	"STOP")
		echo "EGRESS: stop"
		${SEND} ${TARGET} ${PORT} < ${STOP}
		;;
	"LEFT")
		echo "EGRESS: left"
		${SEND} ${TARGET} ${PORT} < ${LEFT}
		;;
	"RGHT")
		echo "EGRESS: right"
		${SEND} ${TARGET} ${PORT} < ${RGHT}
		;;
	"FWRD")
		echo "EGRESS: forward"
		${SEND} ${TARGET} ${PORT} < ${FWRD}
		;;
	"BKWD")
		echo "EGRESS: backward"
		${SEND} ${TARGET} ${PORT} < ${BKWD}
		;;
	"TERM")
		${SEND} ${TARGET} ${PORT} < ${TERM}
		break
		;;
	*)
		echo "Invalid command"
	esac
done

echo "EGRESS is terminated. Good bye"
echo "Good bye!!!"

