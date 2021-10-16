package main

import (
		"os"
		"time"
		"log"
		"bufio"
		"net"
	   )
func main () {
	
	arguments := os.Args

	if len ( arguments ) < 2 {

		log.Println ( "Please provide IP" ) 
		
		return

	}

	var connWriter net.Conn
	var err error
	for {
		time.Sleep ( time.Second )
		connWriter , err = net.Dial ( "tcp" , arguments [ 1 ] + ":10102" )
			if err != nil {
				println ( "waiting for TCP Connection Establishment" )
			} else {
				break
			}
	}

	run ( connWriter )
}

func handleError ( err error ) {
	if err != nil {
		log.Println ( err )
	}
}

func run ( connWriter net.Conn ) {

	byte1 := make ( []byte , 5 )

	motorFile , err := os.OpenFile ( "/dev/car/motor_tun" , os.O_RDWR , 0775 )
	for {


		motor := bufio.NewReader ( motorFile )

		handleError ( err )

		byte1 , err = motor.ReadBytes ( byte ( '\n' )  )
		
		if err != nil {
			log.Fatal ( err )
		}

		_ , _= connWriter.Write ( byte1 )

		println ( string ( byte1 ) )

	}
	motorFile.Close ()
}

