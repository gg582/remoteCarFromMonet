# Introduction

This provides an environment to control a car at long distance.
This is an excerpt from Professor Jin Sung-geun 's laboratory.

In this repository, there is no others' code, but only the codes written by Lee Yunjin except early pure bash prototypes(which are unused after constructing flow chart: e.g. reader.sh).

Except those few files, all bash scripts, C, and Golang scripts are solely written by Lee Yunjin.

There were a few more contributors from other parts of this project(which are all lost).

Front-end, Higher level management: Kim Koung-Bin(김경빈), Abduolimov Alisherbek(@alisherfw)

Structure Engineering is mainly from Kim Koung-Bin, s
maller ideas were from Lee Yunjin and Abduolimov Alisherbek, Akhmad Kholmurodov(e.g: Device Driver Structures, TCP Flowchart at actual implementation, Integrated management)

**Almost all of the sources are implemented by Lee Yunjin, but ideas are from whole laboratory.**

*Here's a thing:*

- Main Structure: Kim Koung-bin, Jin Sung-geun 

- TCP Flowchart: Lee Yunjin, Kim Koung-bin, Jin Sung-geun

- k8s Deploy Structure: Jin Sung-geun

- Detailed k8s Structure: Lee Yunjin

- Detailed Device Driver Structure: Lee Yunjin

- Internal Implementation: Lee Yunjin, Jin Sung-geun, Kim Koung-bin

- Front-end Implementation: Kim Koung-bin, Abduolimov Alisherbek, Akhmad Kholmurodov

- Detailed Software Integration: Lee Yunjin, Akhmad Kholmurodov, Abduolimov Alisherbek


## Basic Network Structure
- Physical Car sends device status 

- Virtual car gets information and runs simulation

- Virtual car gives simulated result into physical car

- Physical car executes those simulated results


Motor Driver implementation: IOCTL to I2C driver
Tunneling device driver is doubly linked list
Others are normal gpio chardev.


 # Project tree

.
 * [driver-real-car](./driver-real-car)
   * [driver](./driver-real-car/driver)
 * [config](./config)
   * [k8s-deploy](./config/k8s-deploy)
 * [exec](./exec)
   * [cluster_pod_man](./exec/cluster_pod_man)
   * [k8s_car_man](./exec/k8s_car_man)
 * [pod](./pod)
   * [driver](./pod/driver)
   * [tunnel](./pod/tunnel)
 * [car](./car)
     * [common](./car/common)
     * [driver](./car/driver)
     * [sonic](./car/sonic)
     * [controller](./car/controller)
     * [real-car](./car/real-car)
     * [virt-cartun](./car/virt-cartun)
     * [runner](./car/runner)
     * [virt-carcon](./car/virt-carcon)
     * [tunnel](./car/tunnel)

## config directory

It is used for K8S Pod deployment.

## car directory

It includes device driver for a car and a control program.

The control program controls the car while receiving commands transmitted at a long distance.
In tunneling device, there is a program catching a command from its tunnel while relaying the commands to remote car.

### car/common directory
Common headers
### car/controller
Go language, and shell script demos for emergency controls(manual act control)
Miscellaneous scripts for controlling car, originally this scripts were called by shelljs(from missing front-end application)
### car/driver
Linux Device Drivers for actual hardware devices
### car/real-car
Go language, and shell script demos for relaying information
### car/runner
Virtual car application with simple driving algorithm with rough object avoidance
### car/sonic
Ultrasonic driver test demo
### car/virt-carcon
Simple bash script at virtual car pod(container name: con)
con Container does VIRTUAL CONTROL
### car/virt-cartun
Simple bash script that triggers tunnel
### car/tunnel
Tunneling device driver

## driver-real-car
Early idea of Linux Device Drivers for physical car
Simple bash scripts for tunneling transmitted information
## exec
pure-shell prototypes and some emergency controls
## pod
early, and abstract ideas of this project


