package main

import (
		"net"
		"fmt"
		"bytes"
		"os"
		"log"
		"encoding/binary"
		"encoding/json"
	   )

type cartype struct {
	sr04Val uint32
		ir0Val uint32
		ir1Val uint32
}


func main () {

arguments := os.Args

			   if len ( arguments ) < 2 {

				   fmt.Println ( "Please provide IP and port" )

					   return
			   }


connection := arguments [ 1 ] + ":" + arguments [ 2 ] 

				var conn net.Conn
				var err error
				for {

					conn , err = net.Dial ( "tcp" , connection )

						println ( "Waiting for TCP Reply" )

						if err == nil {


							break
						}

				}

			println ( "TCP connected" )

				var car cartype

				go getSensorValue ( conn , car )
				for {

					fmt.Printf ( "/dev/car/sr04 --> %d" , car.sr04Val )
						fmt.Printf ( "/dev/car/ir0 --> %d" , car.ir0Val )
						fmt.Printf ( "/dev/car/ir1 --> %d" , car.ir1Val )

						sr04File , err :=			os.OpenFile (  "/dev/car/sr04", os.O_RDWR , 0775 )
						handleError ( err )
						ir0File	, err :=			os.OpenFile (  "/dev/car/ir0" , os.O_RDWR , 0775 ) 
						handleError ( err )
						ir1File	, err :=			os.OpenFile (  "/dev/car/ir1" , os.O_RDWR , 0775 ) 
						handleError ( err )


						sr04Buffer := new( bytes.Buffer  )
						ir0Buffer := new ( bytes.Buffer  )
						ir1Buffer := new ( bytes.Buffer  )
						err = binary.Write ( sr04Buffer , binary.LittleEndian , car.sr04Val 	)
						
						handleError ( err )
						
						err = binary.Write ( ir0Buffer , binary.LittleEndian , car.ir0Val 		)
						
						handleError ( err )

						err = binary.Write ( ir1Buffer , binary.LittleEndian , car.ir1Val 		)
						
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

}

func getSensorValue ( conn net.Conn , car cartype ) {

buf := make ( []byte , 512 )
	
	conn.Read ( buf )
	
	err := json.Unmarshal ( buf , & car )
	handleError ( err )
	
}

func handleError ( err error ) {
	if ( err != nil ) {
		log.Fatal ( err ) 
	}
}
