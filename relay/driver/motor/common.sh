#!/bin/bash

module="car"
mode="664"
names=(${module}/ir0 ${module}/ir1 ${module}/ir2 ${module}/ir3 ${module}/sr04)
motor="${module}/motor"
output="_tun"

let len=${#names[@]}-1
let nr_devs=${len}+1

