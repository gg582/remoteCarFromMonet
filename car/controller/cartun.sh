#!/bin/bash
source ../../conf-exec.sh ; ./sensorRelayInPod $K8S_CAR_ADDR & ./motorRelayInPod $K8S_CAR_ADDR
