package main

import (
	"net"
	"encoding/binary"
	"time"
	"encoding/json"
	"syscall"
	"log"
)

type Cartype struct {
	Sr04Val uint32
	IrLeftVal uint32
	IrRightVal uint32

}

const (
	SR04_DEV = "/dev/car/sr04"
	IR0_DEV = "/dev/car/left_ir"
	IR1_DEV = "/dev/car/right_ir"
)
var	devName []string = []string{ SR04_DEV , IR0_DEV , IR1_DEV }

var	devFd map[string]int = map[string]int { SR04_DEV : 0 , IR0_DEV : 0 , IR1_DEV : 0 }

var	devByte map[string]([]byte) = map[string]([]byte) { SR04_DEV : make ( []byte , 4 ) , IR0_DEV : make ( []byte , 4 ) , IR1_DEV : make ( []byte , 4 ) }



func main () {

	upload , err := net.Listen ( "tcp" , ":10101" )

	if err != nil {
		panic(err)
	}


	uport , err := upload.Accept ()
	if err != nil {
		panic( err )
	}

	READ ( uport )

}


func READ ( uport net.Conn ) {

	var err error

	for _ , name := range devName {

		devFd [ name ], err = syscall.Open ( name , syscall.O_RDONLY , 0775 )

		if err != nil {

			log.Println ( err )

		}
		

	}

	var car Cartype

	for {

		time.Sleep ( time.Millisecond * 300 ) 

		for i , name := range devName {
			_ , err = syscall.Read ( devFd [ name ] , devByte [ name ] )

			if err != nil && len ( devByte [ name ] ) < 4 {
				for i := 0 ; i < 4 ; i ++ {

					 devByte [ name ] = append (  devByte [ name ] , 0 )

				}

			}

		switch ( i ) {

			case 0 :

				car.Sr04Val = binary.LittleEndian.Uint32 ( devByte [ name ] )

			case 1 :

				car.IrLeftVal = binary.LittleEndian.Uint32 ( devByte [ name ] )

				if car.IrLeftVal > 1 {

					car.IrLeftVal = 1 

				}

			case 2 :

				car.IrRightVal = binary.LittleEndian.Uint32 ( devByte [ name ] )

				if car.IrRightVal > 1 {

					car.IrRightVal = 1 

				}
			}


		}


		byte1 , err := json.Marshal ( & car )

		if err != nil {

			println ( "marshal error" )

		}

		byte1 = append ( byte1 , byte ( '\n' ) )

		log.Printf ( "sr04:%d ir_left:%d ir_right:%d \n" , car.Sr04Val , car.IrLeftVal , car.IrRightVal )

		_ , err = uport.Write ( byte1 )



	}
}

