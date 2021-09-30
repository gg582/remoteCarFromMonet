package main

import (
		"os"
		"log"
		"bufio"
		"net"
	   )
func main () {



	con , err := net.Listen ( "tcp" , ":10102" )
	handleError ( err ) 
	connWriter , err := con.Accept ()
	ch := make ( chan []byte )

	go run ( connWriter,  ch )
	go func () {
		for {
			byte1 := make ( []byte , 5 )
			byte1 = <- ch
			if byte1 == nil {
				continue
			}
			println ( string ( byte1 ) )
		}
	} ()
	for {
	}
}

func handleError ( err error ) {
	if err != nil {
		log.Println ( err )
	}
}	
func run ( connWriter net.Conn , ch chan []byte ) {

	byte1 := make ( []byte , 5 )

	for {

		motorFile , err := os.OpenFile ( "/dev/car/motor_tun" , os.O_RDWR , 0775 )

		motor := bufio.NewReader ( motorFile ) 

		handleError ( err )

		byte1 , _ = motor.ReadBytes ( byte ( '\n' )  )

		motorFile.Close ()

		_ , _= connWriter.Write ( byte1 )

		println ( string ( byte1 ) )

		ch <- byte1

	}

}
