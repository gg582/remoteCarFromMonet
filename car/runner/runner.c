#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <linux/types.h>
#include <signal.h>
#include <sys/ioctl.h>

#define DEVNAME "/dev/car/motor"
#define SR04 "/dev/car/sr04"
#define IR0 "/dev/car/left_ir"
#define IR1 "/dev/car/right_ir"

#include <ioctl_car_cmd.h>

#define AVOID_DIST 60
#define MOVE_TIME	1
#define SLEEP_TIME	100
#define TURN_SLEEP_TIME	(100*3)
#define ONE_MILI_SEC	1000	

int dev ;

void sigHandler ( int dummy ) {
	ioctl ( dev , PI_CMD_STOP , sizeof ( struct ioctl_info ) ) ;
	exit ( 0 ) ;
}

int main () {

	int sr04 ; 
	int ir [ 2 ] ;
	int ret ;
	u_int32_t value , irLeft , irRight ;

	puts ( "Runner begins" ) ;

	dev = open ( DEVNAME , O_RDWR ) ;
	ir [ 0 ] = open ( IR0 , O_RDONLY ) ;
	ir [ 1 ] = open ( IR1 , O_RDONLY ) ;

	if ( dev < 0 ) { exit ( 1 ) ; }

	sr04 = open ( SR04 , O_RDONLY ) ;

	if ( sr04 < 0 ) { exit ( 1 ) ; }

	signal ( SIGINT , sigHandler ) ;

	while ( true ) {

		ret = read ( sr04 , &value , sizeof ( u_int32_t ) ) ;

		if ( ret < 0 ) {
			ioctl ( dev , PI_CMD_STOP, sizeof ( struct ioctl_info ) );
			continue;
		}

		ret = read ( ir [ 0 ]  , &irLeft , sizeof ( u_int32_t ) ) ;

		if ( ret < 0 ) { irLeft = 0; }

		ret = read ( ir [ 1 ] , &irRight , sizeof ( u_int32_t ) ) ;

		if ( ret < 0 ) { irRight = 0; }

		if ( value < AVOID_DIST ) {

			if ( irLeft && irRight ) {

				ioctl ( dev , PI_CMD_BACKWARD , sizeof ( struct ioctl_info ) );
				usleep ( SLEEP_TIME * ONE_MILI_SEC ) ;
				printf ("backward --> (%u)\n", value);

			} else if ( irLeft ) {
				ioctl ( dev , PI_CMD_RIGHT , sizeof ( struct ioctl_info )) ;
				usleep ( SLEEP_TIME * ONE_MILI_SEC ) ;
				printf ("right --> (%u)\n", value);

			} else if ( irRight ) {
				ioctl ( dev , PI_CMD_LEFT , sizeof ( struct ioctl_info ) );
				usleep ( SLEEP_TIME * ONE_MILI_SEC ) ;
				printf ("left --> (%u)\n", value);
			}

		} else {
			static int counter;
			if ( !(counter++ % 10) ) {
				printf ("forward --> (%u)\n", value);
			}
			ioctl ( dev , PI_CMD_FORWARD , sizeof ( struct ioctl_info ) );
		}
		
	}

	close ( sr04 ) ;
	close ( ir [ 0 ] ) ;
	close ( ir [ 1 ] ) ;
	close ( dev ) ;	

	return 0 ;
}


