# 1. Introduction

This provides an environment to control a car at long distance

## Project tree

.
 * [config](./config)
   * [yaml](./config/yaml)
 * [car](./car)
   * [common](./car/common)
   * [controller](./car/controller)
   * [driver](./car/driver)
 * [relay](./relay)
     * [driver](./relay/driver)
     * [tunnel](./relay/tunnel)

### config directory

It is used for Pod deployment

### car directory

It includes device driver for a car and a control program.
The control program controls the car while receiving commands transmitted at long distance

### relay directory

It contains a device driver for command tunnel establishment and its management
In tunnel directory, there is a program catching a command from its tunnel while relaying the commands to remote car


