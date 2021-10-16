package main

import (
	"net"
	"os"
	"encoding/binary"
	"encoding/json"
	"reflect"
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

	var car , carPrev Cartype

	for {

		carPrev = car

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
				car.Ir0Val = binary.LittleEndian.Uint32 ( devByte [ devName [ 0 ] ] )
				car.Ir1Val = binary.LittleEndian.Uint32 ( devByte [ devName [ 1 ] ] )

			case 1 :

				car.Ir0Val = binary.LittleEndian.Uint32 ( devByte [ name ] )

				if car.Ir0Val > 1 {

					car.Ir0Val = 1 

				}

			case 2 :

				car.Ir1Val = binary.LittleEndian.Uint32 ( devByte [ name ] )

				if car.Ir1Val > 1 {

					car.Ir1Val = 1 

				}
			}


		}
		if reflect.DeepEqual ( carPrev , car ) == true {
			continue
		}


		byte1 , err := json.Marshal ( & car )

		if err != nil {

			println ( "marshal error" )

		}

		byte1 = append ( byte1 , byte ( '\n' ) )

		log.Printf ( "sr04:%d ir0:%d ir1:%d \n" , car.Sr04Val , car.Ir0Val , car.Ir1Val )

		_ , err = uport.Write ( byte1 )



		log.Println ( string ( byte1 ) , len ( byte1 ) )

		message := make ( []byte , 8 )

		_ , err = uport.Read ( message )
		
		log.Println ( string ( message ) )



	}
}

