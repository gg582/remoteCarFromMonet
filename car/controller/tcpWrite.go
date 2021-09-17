package main 

import (
		"os"
		"fmt"
		"net"
	   )

func main () {

	argument := os.Args 

	if len ( argument ) == 1 {

		return
			
	}

	
	port := ":" + argument [ 1 ] 

	conn , err := net.Listen ( "tcp" , port )

	handleError ( err ) 

	
	for {

		connWriter , err := conn.Accept ()

		handleError ( err )

		motor , err := os.OpenFile ( "/dev/car/motor_tun" , os.O_RDONLY , 0775 )
		
		handleError ( err )

		buf := make ( []byte , 5 )

		_ , err  = motor.Read ( buf )
		
		handleError ( err )
		
		motor.Close ()

		fmt.Println ( "%#U %#U %#U %#U" , buf )
		connWriter.Write ( buf )

	}

}


func handleError ( err  error ) {
	if err != nil {
		panic ( err )
	}
}
