#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <time.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/types.h>

#define DEVNAME "/dev/car/motor"
#define SR04 "/dev/car/sr04"
#define IR0 "/dev/car/left_ir"
#define IR1 "/dev/car/right_ir"

#include <ioctl_car_cmd.h>

#define AVOID_DIST 80
#define BACK_DIST  15
#define MOVE_TIME	1
#define SLEEP_TIME	60
#define ONE_MILI_SEC	1000	

#define LOGNAME "/home/pi/remote-car/tunnel_delay.log" 
#define WR_LOGNAME "/home/pi/remote-car/motor_tun_delay.log"

int dev ;
int sr04 ; 
int ir [ 2 ] ;

FILE * clockLog ;
FILE * motorclockLog ;

u_int32_t Clock () {

	struct timespec tsp;
	clock_gettime ( CLOCK_MONOTONIC , &tsp );
	return ( u_int32_t )tsp.tv_nsec;

}

void sigHandler ( int dummy ) {

	fclose ( clockLog ) ;
	fclose ( motorclockLog ) ;
	close ( sr04 ) ;
	close ( ir [ 0 ] ) ;
	close ( ir [ 1 ] ) ;
	close ( dev ) ;	

	ioctl ( dev , PI_CMD_STOP , sizeof ( struct ioctl_info ) ) ;
	fprintf ( motorclockLog , "/dev/car/motor: %u" , Clock () ) ; 
	exit ( 0 ) ;
}

int main () {

	u_int32_t value=0 , irLeft =0  , irRight=0 ;

	motorclockLog= fopen ( WR_LOGNAME , "wt" );
	clockLog     = fopen ( LOGNAME , "rt" ) ;
	

	puts ( "Runner begins" ) ;

	dev = open ( DEVNAME , O_RDWR ) ;
	ir [ 0 ] = open ( IR0 , O_RDONLY ) ;
	ir [ 1 ] = open ( IR1 , O_RDONLY) ;


	sr04 = open ( SR04 , O_RDONLY ) ;


	signal ( SIGKILL , sigHandler ) ;

	while ( true ) {
		
		read ( sr04 , &value , sizeof ( u_int32_t ) ) ;
		u_int32_t tstamp=Clock () ;
 		u_int32_t clockPrev = 0 ;

		fscanf ( clockLog , "%*[^\n]/dev/car/sr04_tun: %u%*[^\n]" , &clockPrev ) ;

		printf ( "sr04_delay : %u\n" , tstamp - clockPrev );


		tstamp=Clock () ;

		clockPrev = 0 ;


		read ( ir [ 0 ]  , &irLeft , sizeof ( u_int32_t ) ) ;

		fscanf ( clockLog , "%*[^\n]/dev/car/left_ir_tun: %u%*[^\n]" , &clockPrev ) ;

		tstamp=Clock () ;
		printf ( "left_ir_delay : %u\n" , ( u_int32_t ) ( tstamp - clockPrev )  ) ;




		read ( ir [ 1 ] , &irRight , sizeof ( u_int32_t ) ) ;
		fscanf ( clockLog , "%*[^\n]/dev/car/right_ir_tun: %u%*[^\n]" , &clockPrev ) ;

		tstamp=Clock () ;

		printf (  "right_ir_delay : %u\n" , ( u_int32_t ) ( tstamp - clockPrev )  ) ;


		if ( value < AVOID_DIST ) {



			if ( ( irLeft && irRight ) | ( value < BACK_DIST ) ) {
				ioctl ( dev , PI_CMD_BACKWARD , sizeof ( struct ioctl_info ) );
				fprintf ( motorclockLog , "/dev/car/motor: %u" , Clock () ) ; 
				printf ("backward --> (%u)\n", value) ;


			} else if ( irLeft ) {
				ioctl ( dev , PI_CMD_RIGHT , sizeof ( struct ioctl_info )) ;
				fprintf ( motorclockLog , "/dev/car/motor: %u" , Clock () ) ; 
				printf ("right --> (%u)\n", value);

			} else if ( irRight ) {
				ioctl ( dev , PI_CMD_LEFT , sizeof ( struct ioctl_info ) );
				fprintf ( motorclockLog , "/dev/car/motor: %u" , Clock () ) ; 
				printf ("left --> (%u)\n", value);
			} else if ( !value ) {
				ioctl ( dev , PI_CMD_FORWARD ) ;	
				fprintf ( motorclockLog , "/dev/car/motor: %u" , Clock () ) ; 
			} else {
					ioctl ( dev , PI_CMD_LEFT , sizeof ( struct ioctl_info ) );
					fprintf ( motorclockLog , "/dev/car/motor: %u" , Clock () ) ; 
					printf ("left ( predefined ) --> (%u)\n", value);
			}


		} else {
			static int counter;

			if ( irLeft && irRight ) {
				ioctl ( dev , PI_CMD_BACKWARD , sizeof ( struct ioctl_info ) );
				fprintf ( motorclockLog , "/dev/car/motor: %u" , Clock () ) ; 
				printf ("backward --> (%u)\n", value) ;


			} else if ( irLeft) {
				ioctl ( dev , PI_CMD_LEFT , sizeof ( struct ioctl_info )) ;
				fprintf ( motorclockLog , "/dev/car/motor: %u" , Clock () ) ; 
				printf ("left --> (%u)\n", value);

			} else if ( irRight ) {

				ioctl ( dev , PI_CMD_RIGHT , sizeof ( struct ioctl_info ) );
				fprintf ( motorclockLog , "/dev/car/motor: %u" , Clock () ) ; 
				printf ("right --> (%u)\n", value);
			}
			else {
				if ( !(counter++ % 10) ) {
					printf ("forward --> (%u)\n", value);
				}
			       	ioctl ( dev , PI_CMD_FORWARD , sizeof ( struct ioctl_info ) );
				fprintf ( motorclockLog , "/dev/car/motor: %u" , Clock () ) ; 
			}
		
		}
	}

	return 0 ;
}


