package main
import (
	"net"
	"os"
	"os/exec"
	"log"
	"bufio"
	"bytes"
//	"syscall"
	"fmt"
)
func main () {

	if len ( os.Args ) < 2 {
		fmt.Println ( "Please provide IP" )
		return
	}
	PATH , err := os.Getwd ()

	if err != nil {
		log.Println ( err )
	}
	println ( PATH )

	down := os.Args [ 1 ] + ":10102" 
	var download net.Conn

	var res string

	ch := make ( chan string , 1 )
	for {
		download , err = net.Dial ( "tcp" , down )
		if err == nil {
			break
		}
	}

	go Run ( download , ch , PATH )

	for {
		log.Println("response res -------> ", res)
		res = <- ch
		println ( res )
	}
	//download.Close ()

}

func Run ( download net.Conn , ch chan string , PATH string ) {
	for {
		log.Println( "Step 1")

		downloadNew := bufio.NewReader ( download )
		byte1 , err := downloadNew.ReadBytes ( byte ( '\n' ) )
		if err != nil {
			log.Println ( err ) 
			continue
		}
		byte1 = bytes.Trim ( byte1 , "\n" )
		var btString string
		btString = string ( byte1 )
		log.Println ( "btStinrg ----------->:", btString )

		log.Println( "Step 2")

		switch ( btString ) {
			case "LEFT" :
				log.Println( "Step 3" )
				execute (  PATH+"/"+"left"  )
			case "RGHT" :
				execute (  PATH+"/"+"right"  )
			case "FWRD" :
				log.Println( "switch parameter ====>" , btString )
				execute (  PATH+"/"+"forward"  )
			case "BKWD" :
				execute (  PATH+"/"+"backward"  )
			case "STOP" :
				execute (  PATH+"/"+"stop"  )
			case "TERM" :
				execute (  PATH+"/"+"stop"  )
				os.Exit ( 1 ) 
			case "GABG" :
				execute ( PATH+"/"+"garbage"  )
		}
		log.Println( "Step 4")

		ch <- btString
		log.Println( "Step 5")
	}
}

func execute ( cmd string  ) {
	//err := syscall.Exec ( cmd , nil , os.Environ () )
	log.Println( "6" )
	res, err := exec.Command ( cmd ).Output ( )
	if err != nil {

		log.Println ( err )

	}
	log.Println("res from run", res )


}

