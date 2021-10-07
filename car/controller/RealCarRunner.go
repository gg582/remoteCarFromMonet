package main
import (
	"net"
	"os"
	"log"
	"bufio"
	"bytes"
	i2c "github.com/d2r2/go-i2c"
)
func main () {

	var download net.Conn

	var res string

	ch := make ( chan string , 1 )
	downloadListener , err := net.Listen ( "tcp" , ":10102" )
	download , err = downloadListener.Accept ( )
	if err != nil {
		log.Fatal ( err )
	}

	go Run ( download , ch  )

	for {
		log.Println("response res -------> ", res)
		res = <- ch
		println ( res )
	}
	//download.Close ()

}

func Run ( download net.Conn , ch chan string ) {
	I2C , err := i2c.NewI2C ( 0x16 , 1 )

	if err != nil {

		log.Fatal ( err )

	}

	defer I2C.Close ()

	downloadNew := bufio.NewReader ( download )
	for {
		log.Println( "Step 1")

		byte1 , err := downloadNew.ReadBytes ( byte ( '\n' ) )
		if err != nil {
			log.Println ( err ) 
			continue
		}
		byte1 = bytes.Trim ( byte1 , "\n" )
		var btString string
		btString = string ( byte1 )
		log.Println ( "btString ----------->:", btString )

		log.Println( "Step 2")

		switch ( btString ) {

			case "LEFT" :
				_ , _ = I2C.WriteBytes ( []byte { 1 , 0x3F , 0 , 0x4F } )
				
				log.Println( "Step 3" )

				break

			case "RGHT" :
				_ , _ = I2C.WriteBytes ( []byte { 0 , 0x4F , 1 , 0x3F } )
				break
			case "FWRD" :

				_ , _ = I2C.WriteBytes ( []byte { 0 , 0x4F , 0 , 0x4F } )
				log.Println( "switch parameter ====>" , btString )
				break
			case "BKWD" :

				_ , _ = I2C.WriteBytes ( []byte { 1 , 0x4F , 1 , 0x4F } )
				break
			case "STOP" :

				_ , _ = I2C.WriteBytes ( []byte { 0 , 0 , 0 , 0 } )
				break
			case "TERM" :

				_ , _ = I2C.WriteBytes ( []byte { 0 , 0 , 0 , 0 } )
				os.Exit ( 1 ) 
				break

			case "GABG" :

				_ , _ = I2C.WriteBytes ( []byte { 0 , 0 , 0 , 0 } )
				break

		}

		log.Println( "Step 4")

		ch <- btString

		log.Println( "Step 5")
	}
}



