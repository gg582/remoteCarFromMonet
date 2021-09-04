#!/bin/bash

source ../../conf-exec.sh

./eg_tun.sh ${K8S_CAR_ADDR} ${DOWN_PORT} > /dev/null &
./in_tun.sh ${UP_PORT} > /dev/null &
