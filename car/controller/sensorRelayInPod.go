package main

import (
		"net"
		"log"
		"bytes"
		"os"
		"encoding/binary"
		"encoding/json"
		"bufio"
		"time"
	   )

type Cartype struct {
		Sr04Val uint32
		Ir0Val  uint32
		Ir1Val  uint32
}

const (
	DEV_SR04 = "/dev/car/sr04_tun"
	DEV_IR0  = "/dev/car/ir0_tun"
	DEV_IR1  = "/dev/car/ir1_tun"
)

var devNames []string = []string { DEV_SR04, DEV_IR0, DEV_IR1 }

var devFds		map[string]*os.File		 = map[string]*os.File {	 DEV_SR04: nil, DEV_IR0: nil, DEV_IR1: nil, }
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

		time.Sleep ( time.Second )

		if err == nil {
			println ( "TCP connection established" )
			break
		} else {
			println ( "Waiting for TCP connection establishment" )
		}
	}

	println ( "TCP connected" )


	for _, name := range devNames {
			devFds [name] = openFile ( name )
			defer devFds[name].Close ()
			devWriters [ name ] = bufio.NewWriter ( devFds [ name ] )
	}

	ch := make ( chan Cartype , 1 )

	chBool := make ( chan bool , 1 )
	go getSensorValue ( conn , ch , chBool )

	go setSensorValue ( ch , chBool )
	for {
		time.Sleep ( time.Second * 100 )
	}
}

func setSensorValue (ch chan Cartype , c chan bool) {

	var car Cartype

	for {

		flag := <- c
		println ( flag )
		if flag == false {
			continue
		}
		car = <-ch

		log.Printf ( "/dev/car/sr04 --> %d\n" , car.Sr04Val )
		log.Printf ( "/dev/car/ir0 --> %d\n" , car.Ir0Val )
		log.Printf ( "/dev/car/ir1 --> %d\n" , car.Ir1Val )

		devValue [DEV_SR04] = car.Sr04Val
		devValue [DEV_IR0 ] = car.Ir0Val
		devValue [DEV_IR1 ] = car.Ir1Val

		for _, name := range devNames {
			err := binary.Write (  devBuffer [name] , binary.LittleEndian , devValue [name])
			manageError ( err )
			_ , err = devWriters[name].Write( devBuffer [name].Bytes () )
			manageError ( err )
		}
	}
}

func openFile ( name string) *os.File {
	fd , err :=			os.OpenFile (  name, os.O_RDWR , 0775 )
	manageError ( err )
	return fd;
}

func getSensorValue ( conn net.Conn , ch chan Cartype , c chan bool ) {

	var car Cartype

	for {

		conReader :=  bufio.NewReader ( conn )

		buf , err := conReader.ReadBytes ( byte ( '\n' ) )
		manageError ( err ) 
		if err != nil {
			c <- false
			continue
		}

		bufSlice := bytes.Split ( buf , []byte (  "\x00" ) )
		for _ , i := range bufSlice {
			println ( string ( buf ) )
			err = json.Unmarshal ( i , & car ) 
			manageError ( err ) 
			ch <- car
			if err != nil {
				c <- false
					break
			}
		}
		c <- true
	}
}

func manageError ( err error ) {
	if ( err != nil ) {
		log.Println( err )
	}
}


