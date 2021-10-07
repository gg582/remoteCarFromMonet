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
	ch := make ( chan []byte )

	go run ( connWriter,  ch )
	for {
			byte1 := make ( []byte , 5 )
			byte1 = <-ch
			if byte1 == nil {
				continue
			}
			println ( string ( byte1 ) )
		} 
}

func handleError ( err error ) {
	if err != nil {
		log.Println ( err )
	}
}

func run ( connWriter net.Conn , ch chan []byte ) {

	byte1 := make ( []byte , 5 )

	motorFile , err := os.OpenFile ( "/dev/car/motor_tun" , os.O_RDWR , 0775 )
	for {


		motor := bufio.NewReader ( motorFile )

		handleError ( err )

		byte1 , _ = motor.ReadBytes ( byte ( '\n' )  )

		_ , _= connWriter.Write ( byte1 )

		println ( string ( byte1 ) )

		ch <- byte1

	}
	motorFile.Close ()
}

