package main

import (
		"net"
		"fmt"
		"bytes"
		"time"
		"syscall"
		"encoding/binary"
		"encoding/json"
		"bufio"
		"os"
	   )

type Cartype struct {
		Sr04Val	    uint32
		IrLeftVal   uint32
		IrRightVal  uint32
		TimeStamp   uint32
}

const (
	DEV_SR04 = "/dev/car/sr04_tun"
	DEV_IR_LEFT  = "/dev/car/left_ir_tun"
	DEV_IR_RIGHT  = "/dev/car/right_ir_tun"
	PORT = ":10101"
)

var devNames []string = []string { DEV_SR04, DEV_IR_LEFT, DEV_IR_RIGHT }

var devFds	map[string]int	 = map[string]int { DEV_SR04: 0, DEV_IR_LEFT: 0, DEV_IR_RIGHT: 0 }

var devBuffer	map[string]*bytes.Buffer = map[string]*bytes.Buffer{	DEV_SR04: new (bytes.Buffer),
									DEV_IR_LEFT: new (bytes.Buffer),
									DEV_IR_RIGHT: new (bytes.Buffer),
								   }

var devValue    map[string]uint32 = map[string]uint32{ DEV_SR04: 0, DEV_IR_LEFT: 0, DEV_IR_RIGHT: 0, }

func main () {

	arguments := os.Args

	if len ( arguments ) < 2 {

		fmt.Println ( "Please provide IP" )

		return
   }

	var conn net.Conn
	var err error

	for {

		fmt.Println ( "TCP connection trial(sensor): ", arguments [ 1 ] + PORT )
		conn , err = net.Dial ( "tcp" , arguments [ 1 ] + ":10101"  )


		if err != nil {
			continue
		} else {
			fmt.Println ( "Success(sensor): TCP connection establishment" )
			break
		}

		time.Sleep ( time.Second )
	}

	for _, name := range devNames {
		devFds [name] = openFile ( name )
	}

	setSensorValue (conn )
}

func setSensorValue (conn net.Conn ) {

	for {
		car := getSensorValue ( conn )

		fmt.Printf ( "/dev/car/sr04 --> %d\n" , car.Sr04Val )
		fmt.Printf ( "/dev/car/ir_left --> %d\n" , car.IrLeftVal )
		fmt.Printf ( "/dev/car/ir_right --> %d\n" , car.IrRightVal )

		devValue [DEV_SR04] = car.Sr04Val
		devValue [DEV_IR_LEFT ] = car.IrLeftVal
		devValue [DEV_IR_RIGHT ] = car.IrRightVal

		oldTS := car.TimeStamp
		NewTSNow := time.Now ()

		NewTS := uint32 ( NewTSNow.UnixNano () )

		fmt.Printf ( "Delay : %v (sec) \n " , float64(( NewTS - oldTS ) / uint32 (100000000)) )

		for _, name := range devNames {
			err := binary.Write (  devBuffer [name] , binary.LittleEndian , devValue [name])
			manageError ( err )
			_ , err = syscall.Write ( devFds[name] , devBuffer [name].Bytes () )
			devBuffer [ name ].Reset ()
			manageError ( err )
		}
	}
}

func openFile ( name string) int {
	fd , err := syscall.Open (  name, syscall.O_RDWR , 0775 )
	manageError ( err )
	return fd
}

func getSensorValue ( conn net.Conn ) Cartype {

	var car Cartype

	conReader := bufio.NewReader ( conn )

	buf , _ := conReader.ReadBytes ( byte ( '\n' ) )

	fmt.Println ( string ( buf ) )

	bytes.Trim ( buf , "\n" )

	json.Unmarshal ( buf , &car )

	return car
}

func manageError ( err error ) {
	if ( err != nil ) {
		fmt.Println( err )
	}
}


