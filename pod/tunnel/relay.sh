#!/bin/bash

source ../../conf-exec.sh

bg ./eg_tun.sh ${CAR_ADDR} ${DOWN_PORT}
bg ./in_tun.sh ${UP_PORT}
