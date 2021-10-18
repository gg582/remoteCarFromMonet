package main

import (
		"os"
		"time"
		"log"
		"bufio"
		"net"
		"fmt"
	   )

const PORT		= ":10102"
const PROTOCOL	= "tcp"
const DEV_NAME  = "/dev/car/motor_tun"


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

		connWriter , err = net.Dial ( PROTOCOL , arguments [ 1 ] + PORT )

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


	var cmdBytes []byte

	motorFile , err := os.OpenFile ( DEV_NAME , os.O_RDWR , 0775 )

	handleError ( err )

	motorReader := bufio.NewReader ( motorFile ) 

	cmdBytes = make ( []byte , 5 ) 

	for {

		cmdBytes , err = motorReader.ReadBytes ( byte ( '\n' ) )

		if err != nil {
			log.Println ( err ) 
		}

		if len ( cmdBytes ) == 0 {

			continue

		}

		fmt.Println ("Received cmd:", string ( cmdBytes ) )

		if err != nil {
			log.Println ( err )
		}


		_ , err = connWriter.Write ( cmdBytes )

		if err != nil {
			println ( "Connection Broken" )
		}
	


	}
	motorFile.Close ()
}

