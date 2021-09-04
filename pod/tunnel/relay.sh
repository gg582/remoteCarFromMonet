#!/bin/bash

source ../../conf-exec.sh

./eg_tun.sh ${K8S_CAR_ADDR} ${DOWN_PORT} &
./in_tun.sh ${UP_PORT} &
