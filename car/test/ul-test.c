#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <time.h>
#include <stdbool.h>
int main () {

	int myDevice = open ( "/dev/car/sr04" , O_RDWR ) ;
	int num = 0 ;
	int i ;
	unsigned long val = 0 ;
	int ret;

	if ( myDevice == -1 ) {
		perror ( "Open Funtion Failed" ) ; 
		return -1 ;
	}	

	for ( i = 0 ; i < 10000 ; i ++ ) {

		ret = read ( myDevice , & val , sizeof ( unsigned long ) ) ;

		if ( ret ==  0 ) {
			printf ("Error.. \n");
			continue;
		}

		printf ( "Distance : %d cm \n" , val ) ;

		usleep (100 * 1000);
	}

	close ( myDevice ) ;

	return 0 ;
}

