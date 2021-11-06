package main

import (
		"os"
		"syscall"
		"time"
		"fmt"
		"net"
	   )

const PORT	= ":10102"
const PROTOCOL	= "tcp"
const DEV_NAME  = "/dev/car/motor_tun"

func main () {

	arguments := os.Args

	if len ( arguments ) < 2 {
		fmt.Println ( "Please provide IP" )
		return
	}

	var connWriter net.Conn
	var err error

	for {
		fmt.Println ( "TCP connection trial(motor): ", arguments [ 1 ] + PORT )
		connWriter , err = net.Dial ( PROTOCOL , arguments [ 1 ] + PORT )

		if err != nil {
			continue
		} else {
			fmt.Println ( "Success(motor): TCP connection establishment")
			break
		}

		time.Sleep ( time.Second )
	}

	run ( connWriter )
}

func handleError ( err error ) {
	if err != nil {
		fmt.Println ( err )
	}
}

func run ( connWriter net.Conn ) {

	var cmdBytes []byte

	motorFile , err := syscall.Open ( DEV_NAME , os.O_RDWR , 0775 )

	handleError ( err )

	cmdBytes = make ( []byte , 5 ) 

	for {
		_ , err = syscall.Read ( motorFile , cmdBytes )

		if err != nil {
			fmt.Println ( err ) 
		}

		if len ( cmdBytes ) == 0 {
			connWriter.Write ( []byte ( "STOP\n" ) )
			continue
		}

		fmt.Println ("Command from carcon:", string ( cmdBytes ) )

		if err != nil {
			fmt.Println ( err )
		}

		_ , err = connWriter.Write ( cmdBytes )

		if err != nil {
			fmt.Println ( "Connection Broken" )
			os.Exit ( 1 ) 
		}

	}

	syscall.Close ( motorFile ) 
}

