package main

import (
	"encoding/json"
	"fmt"
	"log"
	"net"
	"os"
	"syscall"
	"time"
)

const PORT = ":10102"
const PROTOCOL = "tcp"
const DEV_NAME = "/dev/car/motor_tun"
const LOG = "/home/pi/remote-car/motor_tun_delay.log"

type motorRelay struct {
	MotorBytes string

	TimeStamp int64
}

func main() {

	arguments := os.Args

	if len(arguments) < 2 {
		fmt.Println("Please provide IP")
		return
	}

	var connWriter net.Conn
	var err error

	for {
		fmt.Println("TCP connection trial(motor): ", arguments[1]+PORT)
		connWriter, err = net.Dial(PROTOCOL, arguments[1]+PORT)

		time.Sleep(time.Second)
		if err != nil {
			continue
		} else {
			fmt.Println("Success(motor): TCP connection establishment")
			break
		}

	}

	run(connWriter)
}

func handleError(err error) {
	if err != nil {
		fmt.Println(err)
	}
}

func run(connWriter net.Conn) {

	var cmdBytes motorRelay

	motorFile, err := syscall.Open(DEV_NAME, syscall.O_RDWR, 0775)
	motorLog, err := os.OpenFile(LOG, os.O_RDONLY, 0775)

	cmdBytes.TimeStamp = int64(time.Now().Unix() + time.Now().UnixNano())

	if err != nil {
		log.Println("cannot write log now")
	}

	byt := make([]byte, 5)

	for {

		length, err := syscall.Read(motorFile, byt)

		cmdBytes.MotorBytes = string(byt)

		_, _ = syscall.Write(motorFile, []byte{0, 0, 0, 0})

		if (length == 0) && (err != nil) {

			continue

		} else {

			fmt.Println("Command from carcon:", cmdBytes.MotorBytes)
		}

		if err != nil {
			fmt.Println(err)
		}

		cmd := make([]byte, 100)

		var oldTS int64
		fmt.Fscanf(motorLog, "/dev/car/motor: %d\n", &oldTS)
		cmdBytes.TimeStamp = int64(time.Now().UnixNano() - time.Now().Unix()*1000000)
		fmt.Printf("motor_delay(tunnel) : %d\n", cmdBytes.TimeStamp-oldTS)
		cmdBytes.TimeStamp = int64(time.Now().UnixNano())
		cmd, err = json.Marshal(cmdBytes)
		cmd = append(cmd, byte('\n'))

		if err != nil {
			fmt.Println(err)
		}

		_, err = connWriter.Write(cmd)

		if err != nil {
			fmt.Println("Connection Broken")
		}

	}

}
