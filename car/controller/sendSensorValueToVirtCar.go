package main

import (
	"net"
	"os"
	"encoding/binary"
	"time"
	"encoding/json"
	"bufio"
	"log"
)

type Cartype struct {
	Sr04Val uint32
	IrLeftVal uint32
	IrRightVal uint32
	TimeStamp  uint32

}

const (
	SR04_DEV = "/dev/car/sr04"
	IR0_DEV = "/dev/car/left_ir"
	IR1_DEV = "/dev/car/right_ir"
)
var	devName []string = []string{ SR04_DEV , IR0_DEV , IR1_DEV }

var	devFile map[string](*os.File) = map[string](*os.File) { SR04_DEV : nil , IR0_DEV : nil , IR1_DEV : nil }

var	devByte map[string]([]byte) = map[string]([]byte) { SR04_DEV : make ( []byte , 4 ) , IR0_DEV : make ( []byte , 4 ) , IR1_DEV : make ( []byte , 4 ) }

var	devReader map[string](*bufio.Reader) = map[string](*bufio.Reader ) { SR04_DEV :  nil, IR0_DEV : nil , IR1_DEV : nil }


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

		devFile [ name ], err = os.OpenFile ( name , os.O_RDONLY , 0775 )
		defer devFile [ name ].Close ()

		if err != nil {

			log.Println ( err )

		}
		

		devReader [ name ] = bufio.NewReader ( devFile [ name ] )
	}

	var car Cartype

	for {

		for i , name := range devName {
			_ , err = devReader [ name ].Read (devByte [ name ] )

			if err != nil && len ( devByte [ name ] ) < 4 {
				for i := 0 ; i < 4 ; i ++ {

					 devByte [ name ] = append (  devByte [ name ] , 0 )

				}

			}

		switch ( i ) {

			case 0 :

				for {

					_ , err = devReader [ name ].Read ( devByte [ name ] )

					if err != nil || len ( devByte [ name ] ) < 4 {

						for r := 0 ; r < 4 ; r ++ {

							 devByte [ name ] = append (  devByte [ name ] , 0 )

						}

					}



					car.Sr04Val = binary.LittleEndian.Uint32 ( devByte [ name ] )

					for x := 1 ; x <= 2 ; x ++ {
						_ , err = devReader [ devName [ x ] ].Read ( devByte [ devName [ x ] ] )
						if err != nil || len ( devByte [ devName [ x ] ] ) < 4 {

							for r := 0 ; r < 4 ; r ++ {

								 devByte [ devName [ x ] ] = append (  devByte [ devName  [ x ]] , 0 )

							}

						}

					}
					if car.Sr04Val != 0 {
						break
					}

				}
				car.IrLeftVal = binary.LittleEndian.Uint32 ( devByte [ devName [ 0 ] ] )
				car.IrRightVal = binary.LittleEndian.Uint32 ( devByte [ devName [ 1 ] ] )

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

		timeStr := time.Now ()
		
		TStamp := timeStr.UnixNano ()

		car.TimeStamp = uint32 ( TStamp )

		byte1 , err := json.Marshal ( & car )

		if err != nil {

			println ( "marshal error" )

		}

		byte1 = append ( byte1 , byte ( '\n' ) )

		log.Printf ( "sr04:%d ir_left:%d ir_right:%d \n" , car.Sr04Val , car.IrLeftVal , car.IrRightVal )

		_ , err = uport.Write ( byte1 )



		log.Println ( string ( byte1 ) , len ( byte1 ) )

		message := make ( []byte , 8 )

		_ , _ = uport.Read ( message )
		
		log.Println ( string ( message ) )



	}
}

