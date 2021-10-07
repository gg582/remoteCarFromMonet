package main

import (
	"net"
	"fmt"
	"os"
	"time"
	"encoding/binary"
	"encoding/json"
	"bufio"
	"log"
)

type Cartype struct {
	Sr04Val uint32
	Ir0Val uint32
	Ir1Val uint32
}

const (
	SR04_DEV = "/dev/car/sr04"
	IR0_DEV = "/dev/car/ir0"
	IR1_DEV = "/dev/car/ir1"
)
var	devName []string = []string{ SR04_DEV , IR0_DEV , IR1_DEV }

var	devFile map[string](*os.File) = map[string](*os.File) { SR04_DEV : nil , IR0_DEV : nil , IR1_DEV : nil }

var	devByte map[string]([]byte) = map[string]([]byte) { SR04_DEV :  nil, IR0_DEV : nil , IR1_DEV : nil }

var	devReader map[string](*bufio.Reader) = map[string](*bufio.Reader ) { SR04_DEV :  nil, IR0_DEV : nil , IR1_DEV : nil }


func main () {

	upload , err := net.Listen ( "tcp" , ":10101" )
	
	if err != nil {
		panic(err)
	}

	
	uport , err := upload.Accept ()
	ch := make ( chan bool )
	if err != nil {
		panic( err )
	}
	
	
	go READ ( uport , ch )
	for {
		if <-ch == true {
			println ( "send : OK" )
		} else {
			println ( "send : failed" )

		}
	}
}

	
func READ ( uport net.Conn , ch chan bool ) {


	var err error 


	for _ , name := range devName {

		devFile [ name ], err = os.OpenFile ( name , os.O_RDONLY, 0775 )

		if err != nil {

			log.Println ( err )

		}

		devReader [ name ] = bufio.NewReader ( devFile [ name ] )

	}

	for {

		var car Cartype
		for i , name := range devName {
			devByte [name]  = []byte { 0 , 0 , 0 , 0 , 0 }
			devByte [ name ] = make ( []byte , 5 ) ;

			_ , _ = devReader [ name ].Read( devByte [ name ] )

			if len ( devByte [ name ] ) < 4 {

					for i := 0 ; i < 4 ; i ++ {

						 devByte [ name ] = append (  devByte [ name ] , 0 )

				}

			}

			switch ( i ) {

				case 0 :

					car.Sr04Val = binary.LittleEndian.Uint32 ( devByte [ name ] )

					break
				case 1 :

					car.Ir0Val = binary.LittleEndian.Uint32 ( devByte [ name ] )

					if car.Ir0Val > 1 {

						car.Ir0Val = 1 

					}

					break

				case 2 :

					car.Ir1Val = binary.LittleEndian.Uint32 ( devByte [ name ] )

					if car.Ir1Val > 1 {

						car.Ir1Val = 1 

					}

					break

			}

		}
		byte1 , err := json.Marshal ( & car )

		if err != nil {

			println ( "marshal error" )
		}
		byte1 = append ( byte1 , byte ( '\n' ) )
		fmt.Printf ( "sr04:%d ir0:%d ir1:%d \n" , car.Sr04Val , car.Ir0Val , car.Ir1Val )
		if ( car.Sr04Val == 0 ) {
			log.Println ( "UltraSonic error" ) 
		}

		_ , err = uport.Write ( byte1 )

		fmt.Println ( string ( byte1 ) , len ( byte1 ) )

		if err != nil {
			log.Println ( err )
			ch <- false

		} else {

			fmt.Println ( "success" )
			ch <- true

		}
		time.Sleep ( time.Millisecond * 10 )
	}
}

