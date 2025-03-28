package main

import (
	"bufio"
	"encoding/binary"
	"fmt"
	"io"
	"log"
	"os"
	"syscall"
	"time"
	"unsafe"

	"github.com/d2r2/go-i2c"
)

const (
	AVOID_DIST   uint32 = 100
	IR0                 = "/dev/car/ir0"
	IR1                 = "/dev/car/ir1"
	SR04                = "/dev/car/sr04"
	CMD_LEFT            = 3
	CMD_RIGHT           = 4
	CMD_FORWARD         = 5
	CMD_BACKWARD        = 6
	CMD_STOP            = 7
	CMD_GARBAGE         = 8
)

var devName = []string{SR04, IR0, IR1}
var devFile = map[string]*os.File{SR04: nil, IR0: nil, IR1: nil}
var devFd = map[string]uintptr{SR04: 0, IR0: 0, IR1: 0}
var devByte = map[string][]byte{SR04: nil, IR0: nil, IR1: nil}
var devReader = map[string]*bufio.Reader{SR04: nil, IR0: nil, IR1: nil}
var devValue = map[string]uint32{SR04: 0, IR0: 0, IR1: 0}

type ioctlInfo struct {
	size uint32
	buf  [128]byte
}

var info ioctlInfo

func ioctlMotor(fd uintptr, cmd int) {
	syscall.Syscall(syscall.SYS_IOCTL, fd, uintptr(cmd), uintptr(unsafe.Pointer(&info)))
}

func main() {
	// Open I2C connection
	I2C, err := i2c.NewI2C(0x16, 1)
	if err != nil {
		log.Fatalf("Failed to initialize I2C: %v", err)
	}
	defer I2C.Close()

	for _, name := range devName {
		devByte[name] = make([]byte, 5)
		fmt.Println(name + ":")
		devFile[name], _ = os.OpenFile(name, os.O_RDWR, 0775)
		fmt.Println("opened")
		devReader[name] = bufio.NewReader(devFile[name])

		_, _ = devReader[name].Read(devByte[name])

		devByte[name] = checkErr(devByte[name])
		devValue[name] = binary.LittleEndian.Uint32(devByte[name])

		fmt.Println(devValue[name])
	}

	ch := make(chan string, 1)
	go run(ch)

	for {
		res := <-ch
		fmt.Println(res)
	}
}

func run(ch chan string) {
	motorFile, _ := os.OpenFile("/dev/car/motor_tun", os.O_RDWR, 0775)
	motor := motorFile.Fd()

	var btString string

	for _, name := range devName {
		devFile[name], _ = os.OpenFile(name, os.O_RDONLY, 0775)
		devFd[name] = devFile[name].Fd()
		devReader[name] = bufio.NewReader(devFile[name])
		devByte[name], _ = io.ReadAll(devReader[name])
		devByte[name] = checkErr(devByte[name])
		devValue[name] = binary.LittleEndian.Uint32(devByte[name])
	}

	for {
		if devValue[SR04] < AVOID_DIST {
			if devValue[IR0] == 1 && devValue[IR1] == 1 {
				ioctlMotor(motor, CMD_BACKWARD)
				time.Sleep(800 * time.Millisecond)
				ioctlMotor(motor, CMD_STOP)
				btString = `BACKWARD`
			} else if devValue[IR0] == 1 {
				for devValue[IR0] == 1 || devValue[IR1] == 1 {
					ioctlMotor(motor, CMD_LEFT)
					updateSensorValues()
				}
				btString = `LEFT`
				ioctlMotor(motor, CMD_STOP)
			} else if devValue[IR1] == 1 {
				for devValue[IR0] == 1 || devValue[IR1] == 1 {
					ioctlMotor(motor, CMD_RIGHT)
					updateSensorValues()
				}
				btString = `RIGHT`
				ioctlMotor(motor, CMD_STOP)
			} else {
				ioctlMotor(motor, CMD_FORWARD)
				time.Sleep(700 * time.Millisecond)
				ioctlMotor(motor, CMD_STOP)
				btString = `FORWARD`
			}
		}
		ch <- btString
	}
}

func updateSensorValues() {
	devByte[IR0], _ = io.ReadAll(devReader[IR0])
	devByte[IR0] = checkErr(devByte[IR0])
	devValue[IR0] = binary.LittleEndian.Uint32(devByte[IR0])
	devByte[IR1], _ = io.ReadAll(devReader[IR1])
	devByte[IR1] = checkErr(devByte[IR1])
	devValue[IR1] = binary.LittleEndian.Uint32(devByte[IR1])
}

func checkErr(byteSlice []byte) []byte {
	if len(byteSlice) != 4 {
		for i := 0; i < 4; i++ {
			byteSlice = append(byteSlice, 0)
		}
	}
	return byteSlice
}
