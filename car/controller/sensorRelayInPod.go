package main

import (
		"net"
		"log"
		"bytes"
		"fmt"
		"os"
		"encoding/binary"
		"encoding/json"
		"bufio"
		"time"
	   )

type Cartype struct {

		Sr04Val uint32

		IrLeftVal  uint32

		IrRightVal  uint32

}

const (
	DEV_SR04 = "/dev/car/sr04_tun"
	DEV_IR0  = "/dev/car/left_ir_tun"
	DEV_IR1  = "/dev/car/right_ir_tun"
)

var devNames []string = []string { DEV_SR04, DEV_IR0, DEV_IR1 }

var devFiles		map[string]*os.File		 = map[string]*os.File {	 DEV_SR04: nil, DEV_IR0: nil, DEV_IR1: nil, }
var devWriters		map[string]*bufio.Writer		 = map[string]*bufio.Writer {	 DEV_SR04: nil, DEV_IR0: nil, DEV_IR1: nil, }
var devBuffer	map[string]*bytes.Buffer = map[string]*bytes.Buffer{ DEV_SR04: new (bytes.Buffer), DEV_IR0: new (bytes.Buffer), DEV_IR1: new (bytes.Buffer), }
var devValue    map[string]uint32		 = map[string]uint32{ DEV_SR04: 0, DEV_IR0: 0, DEV_IR1: 0, }

func main () {

	var conn net.Conn
	var err error

	arguments := os.Args

	if len ( arguments ) < 2 {

		log.Println ( "Please provide IP" )

		return
   }


	for {

		conn , err = net.Dial ( "tcp" , arguments [ 1 ] + ":10101"  )

		if err == nil {
			println ( "TCP connection established" )
			break
		} else {
			println ( "Waiting for TCP connection establishment" )
		}

		time.Sleep ( time.Second )
	}

	println ( "TCP connected" )


	for _, name := range devNames {
			devFiles [name] = openFile ( name )
			defer devFiles[name].Close ()
			devWriters [ name ] = bufio.NewWriter ( devFiles [ name ] )
	}


	setSensorValue ( conn , devWriters ,devBuffer , devValue )


}

func setSensorValue (conn net.Conn , devWriters map[string]*bufio.Writer , devBuffer map[string]*bytes.Buffer , devValue map[string]uint32 ) {

	var car Cartype

	for {
		car = getSensorValue ( conn )

		fmt.Printf ( "/dev/car/sr04 --> %d\n" , car.Sr04Val )
		fmt.Printf ( "/dev/car/ir_left --> %d\n" , car.IrLeftVal )
		fmt.Printf ( "/dev/car/ir_right --> %d\n" , car.IrRightVal )

		devValue [DEV_SR04] = car.Sr04Val
		devValue [DEV_IR0 ] = car.IrLeftVal
		devValue [DEV_IR1 ] = car.IrRightVal

		writeBytes := make ( []byte , 4 )

		for _ , name := range devNames {


			binary.LittleEndian.PutUint32 ( writeBytes , devValue [ name ] )


			_ , err := devWriters[name].Write( writeBytes )
			if err != nil {

				log.Println ( err )
				break

			}
		}
		conn.Write ( []byte ( "success\n" ) ) 
	}
}

func openFile ( name string) *os.File {
	fd , err :=			os.OpenFile (  name , os.O_RDWR , 0775 )

	handleError ( err )

	return fd
}

func getSensorValue ( conn net.Conn ) Cartype {

	var car Cartype

	conReader :=  bufio.NewReader ( conn )

	buf , err := conReader.ReadBytes ( byte ( '\n' ) )


	println ( string ( buf ) )
	buf = bytes.Trim ( buf , "\n" )
	err = json.Unmarshal ( buf , & car )

	if err != nil {
		log.Println ( "Unmarshal error" )
	}


	return car

}


func manageError ( err error ) {

	if ( err != nil ) {

		log.Println( err )

	}

}

func handleError ( err error ) {

	if ( err != nil ) {

		log.Fatal( err )

	}

}

