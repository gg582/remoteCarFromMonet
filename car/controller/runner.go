package main

import (

	"io"
	"time"
	"os"
	"encoding/binary"
	"log"
	"bufio"
	"github.com/d2r2/go-i2c"
)


const (

		AVOID_DIST uint32 = 100

		IR0 = "/dev/car/ir0"
		IR1 = "/dev/car/ir1"
		SR04 = "/dev/car/sr04"

		)
	var devName []string = []string { SR04 , IR0 , IR1 }
	var devFile map[string](*os.File ) = map[string](*os.File) { SR04 : nil , IR0 : nil , IR1 : nil }
	var devFd map[string]uintptr = map[string]uintptr { SR04 : 0 , IR0 : 0 , IR1 : 0 }

	var devByte map[string]([]byte)	   = map[string]([]byte)   { SR04 : nil , IR0 : nil , IR1 : nil }
	var devReader map[string](*bufio.Reader) = map[string](*bufio.Reader) { SR04 : nil , IR0 : nil , IR1 : nil }
	var devValue map[string]uint32	   = map[string]uint32	   { SR04 : 0 , IR0 : 0 , IR1 : 0 }

func main () {


	I2C , _ := i2c.NewI2C ( 0x16 , 1 )


	for _ , name := range devName {
		
		devByte [ name ] = make ( []byte , 5 ) 

		print ( name+":" )

		devFile [ name ] , _ = os.OpenFile ( name , os.O_RDWR , 0775 )

		println ( "opened" )

		devReader [ name ] = bufio.NewReader ( devFile [ name ] )

		println ( "bufio" )

		_ , _ = devReader [ name ].Read ( devByte [ name ] ) 

		println ( "read" )

		devByte [ name ] = checkErr ( devByte [ name ] )

		println ( "err" ) 

		devValue [ name ]    = binary.LittleEndian.Uint32 ( devByte [ name ] )
		
		println (  devValue [ name ] ) 

	}

	for {
		for _ , name := range devName {
			_ , _ = devReader [ name ].Read ( devByte [ name ] ) 

			println ( "read" )

			devByte [ name ] = checkErr ( devByte [ name ] )

			println ( "err" ) 

			devValue [ name ]    = binary.LittleEndian.Uint32 ( devByte [ name ] )
			
			println (  devValue [ name ] ) 

		}

		if devValue [ SR04 ] < AVOID_DIST {

			if ( devValue [ IR0 ] == 1 ) && ( devValue [ IR1 ] == 1 ) {

				I2C.WriteBytes ( []byte { 0x01,0 , 0x4F , 0 , 0x4F } ) 
				time.Sleep ( 1300 * time.Millisecond )


			} else if devValue [ IR1 ] == 1 {
				for ( devValue [ IR0 ] == 1 ) || ( devValue [ IR1 ] == 1 ) {

					I2C.WriteBytes ( []byte { 0x01,0 , 0x3F , 1 , 0x4F } ) 
					devByte [ IR0 ] , _ = io.ReadAll ( devReader [ IR0 ] )
					devByte [ IR0 ] = checkErr ( devByte [ IR0 ])
					devValue [ IR0 ] = binary.LittleEndian.Uint32 ( devByte [ IR0 ] )
					devByte [ IR1 ] , _ = io.ReadAll ( devReader [ IR1 ] )
					devByte [ IR1 ] = checkErr ( devByte [ IR1 ] )
					devValue [ IR1 ] = binary.LittleEndian.Uint32 ( devByte [ IR1 ] )
				}

			} else if devValue [ IR0 ] == 1{

				for ( devValue [ IR0 ] == 1 ) || ( devValue [ IR1 ] == 1 ) {

					I2C.WriteBytes ( []byte { 0x01,1 , 0x4F , 0 , 0x3F } ) 
					devByte [ IR0 ] , _ = io.ReadAll ( devReader [ IR0 ] )
					devByte [ IR0 ] = checkErr ( devByte [ IR0 ])
					devValue [ IR0 ] = binary.LittleEndian.Uint32 ( devByte [ IR0 ] )
					devByte [ IR1 ] , _ = io.ReadAll ( devReader [ IR1 ] )
					devByte [ IR1 ] = checkErr ( devByte [ IR1 ] )
					devValue [ IR1 ] = binary.LittleEndian.Uint32 ( devByte [ IR1 ] )

				}

			} else {
				if ( devValue [ IR0 ] == 1 ) && ( devValue [ IR1 ] == 1 ) {

					I2C.WriteBytes ( []byte { 0x01,0 , 0x4F , 0 , 0x4F } ) 
					time.Sleep ( 800 * time.Millisecond )

				} else if devValue [ IR1 ] == 1 {
					for ( devValue [ IR0 ] == 1 ) || ( devValue [ IR1 ] == 1 ) {

						I2C.WriteBytes ( []byte { 0x01,0 , 0x3F , 1 , 0x4F } ) 
						devByte [ IR0 ] , _ = io.ReadAll ( devReader [ IR0 ] )
						devByte [ IR0 ] = checkErr ( devByte [ IR0 ])
						devValue [ IR0 ] = binary.LittleEndian.Uint32 ( devByte [ IR0 ] )
						devByte [ IR1 ] , _ = io.ReadAll ( devReader [ IR1 ] )
						devByte [ IR1 ] = checkErr ( devByte [ IR1 ] )
						devValue [ IR1 ] = binary.LittleEndian.Uint32 ( devByte [ IR1 ] )
					}

	
				} else if devValue [ IR0 ] == 1{
	
					for ( devValue [ IR0 ] == 1 ) || ( devValue [ IR1 ] == 1 ) {

						I2C.WriteBytes ( []byte { 0x01,1 , 0x4F , 0 , 0x3F } ) 
						devByte [ IR0 ] , _ = io.ReadAll ( devReader [ IR0 ] )
						devByte [ IR0 ] = checkErr ( devByte [ IR0 ])
						devValue [ IR0 ] = binary.LittleEndian.Uint32 ( devByte [ IR0 ] )
						devByte [ IR1 ] , _ = io.ReadAll ( devReader [ IR1 ] )
						devByte [ IR1 ] = checkErr ( devByte [ IR1 ] )
						devValue [ IR1 ] = binary.LittleEndian.Uint32 ( devByte [ IR1 ] )

					}



				} else {

					I2C.WriteBytes ( []byte { 0x01,1 , 0x4F , 1 , 0x4F } ) 


				}
			}

			log.Println( "Step 4")

		}

	}

}

func checkErr ( byteSlice []byte ) []byte {

		if len ( byteSlice ) != 4 {


			for i := 0 ; i < 4 ; i ++ {

				byteSlice = append ( byteSlice , 0 )

			}

		}
		return byteSlice
}

