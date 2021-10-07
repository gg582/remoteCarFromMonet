package main

import (

	"io"
	"time"
	"os"
	"encoding/binary"
	"log"
	"bufio"
<<<<<<< HEAD
	"github.com/d2r2/go-i2c"
)


const (

		AVOID_DIST uint32 = 100
=======
	"bytes"
	"../runner/runner"
	"fmt"
	"io"
	"time"
	"encoding/binary"
	"syscall"
	"unsafe"
)

type ioctlInfo struct {

	size uint32
	buf [128]byte

}

const (

		AVOID_DIST=100
		CMD_LEFT = 3
		CMD_RIGHT = 4
		CMD_FORWARD = 5
		CMD_BACKWARD = 6
		CMD_STOP = 7
		CMD_GARBAGE = 8
>>>>>>> 9ee4511fe6b3c08860a7d1686e1c99b4cbaf19f3

		IR0 = "/dev/car/ir0"
		IR1 = "/dev/car/ir1"
		SR04 = "/dev/car/sr04"

<<<<<<< HEAD
		)
=======
	)
>>>>>>> 9ee4511fe6b3c08860a7d1686e1c99b4cbaf19f3
	var devName []string = []string { SR04 , IR0 , IR1 }
	var devFile map[string](*os.File ) = map[string](*os.File) { SR04 : nil , IR0 : nil , IR1 : nil }
	var devFd map[string]uintptr = map[string]uintptr { SR04 : 0 , IR0 : 0 , IR1 : 0 }

	var devByte map[string]([]byte)	   = map[string]([]byte)   { SR04 : nil , IR0 : nil , IR1 : nil }
	var devReader map[string](*bufio.Reader) = map[string](*bufio.Reader) { SR04 : nil , IR0 : nil , IR1 : nil }
	var devValue map[string]uint32	   = map[string]uint32	   { SR04 : 0 , IR0 : 0 , IR1 : 0 }

<<<<<<< HEAD
=======


var info ioctlInfo ;

func IoctlMotor ( fd uintptr , cmd int ) {

	syscall.Syscall ( syscall.SYS_IOCTL , fd , uintptr ( cmd ) , uintptr ( unsafe.Pointer ( &info ) ) )

}


func main () {



	var res string

	ch := make ( chan string , 1 )

	go Run ( ch )

	for {
		log.Println("response res -------> " , res)
		res = <- ch
		println ( res )
	}
	//download.Close ()

}

func Run ( ch chan string ) {

	motorFile , _ := os.OpenFile ( "/dev/car/motor_tun" , os.O_RDWR , 0775 )

	motor := motorFile.Fd ()

	var btString string

	for _ , name := range devName {

		devFile [ name ] , _ = os.OpenFile ( name , os.O_RDONLY , 0775 )

		devFd [ name ] = devFile [ name ].Fd ()

		devReader [ name ] = bufio.NewReader ( devFile [ name ] )

		devByte [ name ] , _ = io.ReadAll ( devReader [ name ] )

		devByte [ name ] = checkErr ( devByte [ name ] )

		devValue [ name ]    = binary.LittleEndian.Uint32 ( devByte [ name ] )

	}

	for {

		if devValue [ SR04 ] < AVOID_DIST {

			if ( devValue [ IR0 ] == 1 ) && ( devValue [ IR1 ] == 1 ) {

				IoctlMotor ( motor , CMD_BACKWARD )

				time.Sleep ( 800 * time.Millisecond )

				IoctlMotor ( motor , CMD_STOP )
				btString = `BACKWARD`

			} else if devValue [ IR0 ] == 1 {
				for ( devValue [ IR0 ] == 1 ) || ( devValue [ IR1 ] == 1 ) {

					IoctlMotor ( motor , CMD_LEFT )
					devByte [ IR0 ] , _ = io.ReadAll ( devReader [ IR0 ] )
					devByte [ IR0 ] = checkErr ( devByte [ IR0 ])
					devValue [ IR0 ] = binary.LittleEndian.Uint32 ( devByte [ IR0 ] )
					devByte [ IR1 ] , _ = io.ReadAll ( devReader [ IR1 ] )
					devByte [ IR1 ] = checkErr ( devByte [ IR1 ] )
					devValue [ IR1 ] = binary.LittleEndian.Uint32 ( devByte [ IR1 ] )
				}

				btString = `LEFT`
				IoctlMotor ( motor , CMD_STOP )

			} else if devValue [ IR1 ] == 1{

				for ( devValue [ IR0 ] == 1 ) || ( devValue [ IR1 ] == 1 ) {

					IoctlMotor ( motor , CMD_RIGHT )
					devByte [ IR0 ] , _ = io.ReadAll ( devReader [ IR0 ] )
					devByte [ IR0 ] = checkErr ( devByte [ IR0 ])
					devValue [ IR0 ] = binary.LittleEndian.Uint32 ( devByte [ IR0 ] )
					devByte [ IR1 ] , _ = io.ReadAll ( devReader [ IR1 ] )
					devByte [ IR1 ] = checkErr ( devByte [ IR1 ] )
					devValue [ IR1 ] = binary.LittleEndian.Uint32 ( devByte [ IR1 ] )

				}


				IoctlMotor ( motor , CMD_STOP )
				btString = `RIGHT`
			} else {
				if ( devValue [ IR0 ] == 1 ) && ( devValue [ IR1 ] == 1 ) {

					IoctlMotor ( motor , CMD_BACKWARD )

					time.Sleep ( 800 * time.Millisecond )

					IoctlMotor ( motor , CMD_STOP )

					btString = `BACKWARD`

				} else if devValue [ IR0 ] == 1 {
					for ( devValue [ IR0 ] == 1 ) || ( devValue [ IR1 ] == 1 ) {

						IoctlMotor ( motor , CMD_LEFT )
						devByte [ IR0 ] , _ = io.ReadAll ( devReader [ IR0 ] )
						devByte [ IR0 ] = checkErr ( devByte [ IR0 ])
						devValue [ IR0 ] = binary.LittleEndian.Uint32 ( devByte [ IR0 ] )
						devByte [ IR1 ] , _ = io.ReadAll ( devReader [ IR1 ] )
						devByte [ IR1 ] = checkErr ( devByte [ IR1 ] )
						devValue [ IR1 ] = binary.LittleEndian.Uint32 ( devByte [ IR1 ] )
					}

					btString = `LEFT`
					IoctlMotor ( motor , CMD_STOP )

				} else if devValue [ IR1 ] == 1{

					for ( devValue [ IR0 ] == 1 ) || ( devValue [ IR1 ] == 1 ) {

						IoctlMotor ( motor , CMD_RIGHT )
						devByte [ IR0 ] , _ = io.ReadAll ( devReader [ IR0 ] )
						devByte [ IR0 ] = checkErr ( devByte [ IR0 ])
						devValue [ IR0 ] = binary.LittleEndian.Uint32 ( devByte [ IR0 ] )
						devByte [ IR1 ] , _ = io.ReadAll ( devReader [ IR1 ] )
						devByte [ IR1 ] = checkErr ( devByte [ IR1 ] )
						devValue [ IR1 ] = binary.LittleEndian.Uint32 ( devByte [ IR1 ] )

					}
					btString = `RIGHT`

				} else {

					IoctlMotor ( motor , CMD_STOP )

					IoctlMotor ( motor , CMD_FORWARD )

					time.Sleep ( 700 * time.Millisecond )

					IoctlMotor ( motor , CMD_STOP )

					btString = `FORWARD`
				}
			}

			log.Println( "Step 4")

			ch <- btString

			log.Println( "Step 5")
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

>>>>>>> 9ee4511fe6b3c08860a7d1686e1c99b4cbaf19f3
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

<<<<<<< HEAD
=======
func Run ( download net.Conn , ch chan string , PATH string ) {
	motorFile := os.OpenFile ( "/dev/car/motor_tun" , os.O_RDWR , 0775 )


	motor := motorFile.Fd ()
>>>>>>> 9ee4511fe6b3c08860a7d1686e1c99b4cbaf19f3
	for {
		for _ , name := range devName {
			_ , _ = devReader [ name ].Read ( devByte [ name ] ) 

			println ( "read" )

			devByte [ name ] = checkErr ( devByte [ name ] )

			println ( "err" ) 

			devValue [ name ]    = binary.LittleEndian.Uint32 ( devByte [ name ] )
			
			println (  devValue [ name ] ) 

		}
<<<<<<< HEAD

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

=======
		byte1 = bytes.Trim ( byte1 , "\n" )
		var btString string
		btString = string ( byte1 )
		log.Println ( "btStinrg ----------->:", btString )

		log.Println( "Step 2")

		switch ( btString ) {
			case "LEFT" :
				log.Println( "Step 3" )
				IoctlMotor (  motor , CMD_LEFT  )
			case "RGHT" :
				IoctlMotor (  motor , CMD_RIGHT  )
			case "FWRD" :
				log.Println( "switch parameter ====>" , btString )
				IoctlMotor (  motor , CMD_FORWARD  )
			case "BKWD" :
				IoctlMotor (  motor , CMD_BACKWARD  )
			case "STOP" :
				IoctlMotor (  motor , CMD_STOP )
			case "TERM" :
				IoctlMotor ( motor , CMD_STOP )
				os.Exit ( 1 ) 
			case "GABG" :
				IoctlMotor ( motor , CMD_GARBAGE  )
>>>>>>> 9ee4511fe6b3c08860a7d1686e1c99b4cbaf19f3
		}

	}

}

<<<<<<< HEAD
func checkErr ( byteSlice []byte ) []byte {

		if len ( byteSlice ) != 4 {


			for i := 0 ; i < 4 ; i ++ {

				byteSlice = append ( byteSlice , 0 )

			}

		}
		return byteSlice
}
=======
>>>>>>> 9ee4511fe6b3c08860a7d1686e1c99b4cbaf19f3

