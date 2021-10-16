#!/bin/bash

module="car"
mode="664"
names=(${module}/left_ir ${module}/right_ir ${module}/sr04)
motor="${module}/motor"
output="_tun"

let len=${#names[@]}-1
let nr_devs=${len}+1

