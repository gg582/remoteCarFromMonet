package main

import (
		"net"
		"log"
		"bytes"
		"os"
		"encoding/binary"
		"encoding/json"
		"bufio"
	   )

type Cartype struct {
		Sr04Val uint32
		Ir0Val uint32
		Ir1Val uint32
}


func main () {

		arguments := os.Args

	   if len ( arguments ) < 2 {

		   log.Println ( "Please provide IP and port" )

			   return
	   }



		var conn net.Conn
		var err error
		for {

			conn , err = net.Dial ( "tcp" , arguments [ 1 ] + ":10101"  )

				println ( "Waiting for TCP Reply" )

				if err == nil {


					break
				}

		}

	println ( "TCP connected" )
	go func () {
		var car Cartype
		

		for {

				car = getSensorValue ( conn , car )
				log.Printf ( "/dev/car/sr04 --> %d\n" , car.Sr04Val )
				log.Printf ( "/dev/car/ir0 --> %d\n" , car.Ir0Val )
				log.Printf ( "/dev/car/ir1 --> %d\n" , car.Ir1Val )

				sr04File , err :=			os.OpenFile (  "/dev/car/sr04", os.O_RDWR , 0775 )
				handleError ( err )
				ir0File	, err :=			os.OpenFile (  "/dev/car/ir0" , os.O_RDWR , 0775 ) 
				handleError ( err )
				ir1File	, err :=			os.OpenFile (  "/dev/car/ir1" , os.O_RDWR , 0775 ) 
				handleError ( err )


				sr04Buffer := new( bytes.Buffer  )
				ir0Buffer := new ( bytes.Buffer  )
				ir1Buffer := new ( bytes.Buffer  )
				err = binary.Write ( sr04Buffer , binary.LittleEndian , car.Sr04Val 	)
				
				handleError ( err )
				
				err = binary.Write ( ir0Buffer , binary.LittleEndian , car.Ir0Val 		)
				
				handleError ( err )

				err = binary.Write ( ir1Buffer , binary.LittleEndian , car.Ir1Val 		)
				
				handleError ( err )
				
				_ , err = sr04File.Write ( sr04Buffer.Bytes () )
				
				handleError ( err )
				
				_ , err = ir0File.Write ( ir0Buffer.Bytes () )
				
				handleError ( err )
				
				_ , err = ir1File.Write ( ir1Buffer.Bytes () )
				
				handleError ( err )
				
				sr04File.Close ()
				ir0File.Close ()
				ir1File.Close ()
		}
	} ()
	for {
	}

}

func getSensorValue ( conn net.Conn , car Cartype ) Cartype {

	conReader := bufio.NewReader ( conn ) 
	buf , err := conReader.ReadSlice ( '\n' )
	buf = bytes.Trim ( buf , "\x00\n" )
	buf = bytes.TrimSpace ( buf )
	println ( string ( buf ) )
	err = json.Unmarshal ( buf , &car )
	handleError ( err )
	return car
	
}

func handleError ( err error ) {
	if ( err != nil ) {
		log.Fatal ( err ) 
	}
}

