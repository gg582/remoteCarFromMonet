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
#define IR0 "/dev/car/ir0"
#define IR1 "/dev/car/ir1"

#include <ioctl_car_cmd.h>

#define AVOID_DIST	100	
#define LOOP_COUNT	100
#define MOVE_TIME	1
#define TURN_TIME	330
#define ONE_MILI_SEC	1000	
bool isRunning = true ;

void sigHandler ( int dummy ) {
	isRunning = false ;
}

int main () {

	int dev ;
	int sr04 ; 
	int ir [ 2 ] ;
	unsigned int step = 0;

	dev = open ( DEVNAME , O_RDWR ) ;
	ir [ 0 ] = open ( IR0 , O_RDONLY ) ;
	ir [ 1 ] = open ( IR1 , O_RDONLY ) ;

	if ( dev < 0 ) { exit ( 1 ) ; }

	sr04 = open ( SR04 , O_RDONLY ) ;

	if ( sr04 < 0 ) { exit ( 1 ) ; }

	signal ( SIGINT , sigHandler ) ;

	while ( isRunning ) {

		int 	 ret;
		u_int32_t value , ir0 , ir1 ;

		ret = read ( sr04 , &value , sizeof ( ssize_t ) ) ;
		if ( ret < 0 ) continue ;
		ret = read ( ir [ 0 ]  , &ir0 , sizeof ( ssize_t ) ) ;
		if ( ret < 0 ) continue ;
		ret = read ( ir [ 1 ] , &ir1 , sizeof ( ssize_t ) ) ;
		if ( ret < 0 ) continue ;

		if ( value < AVOID_DIST ) {

			ioctl ( dev , PI_CMD_STOP, sizeof ( struct ioctl_info ) );
//			printf ("stop --> (%u)\n", value);

//			sleep ( MOVE_TIME ) ;
	


		ir0 = ! ir0 ;
		ir1 = ! ir1 ;

		if ( !ir0 && !ir1 ) {

			ioctl ( dev , PI_CMD_BACKWARD , sizeof ( struct ioctl_info ) );
			usleep ( 600 * ONE_MILI_SEC ) ;
			ioctl ( dev , PI_CMD_STOP, sizeof ( struct ioctl_info ) );
		}
		if ( ir0 ) {
			while ( ir0 && ir1 ) {
				ret = read ( ir [ 0 ]  , &ir0 , sizeof ( ssize_t ) ) ;
				ret = read ( ir [ 1 ]  , &ir1 , sizeof ( ssize_t ) ) ;
				ioctl ( dev , PI_CMD_LEFT , sizeof ( struct ioctl_info )) ;
			}
			printf ("left --> (%u)\n", value);
			usleep ( TURN_TIME * ONE_MILI_SEC ) ;
			} else {
		
				while ( ir0 && ir1 ) {
					ret = read ( ir [ 0 ]  , &ir0 , sizeof ( ssize_t ) ) ;
					ret = read ( ir [ 1 ]  , &ir1 , sizeof ( ssize_t ) ) ;
					ioctl ( dev , PI_CMD_RIGHT , sizeof ( struct ioctl_info ) );
				}
				printf ("right --> (%u)\n", value);
				usleep ( TURN_TIME * ONE_MILI_SEC ) ;
	
			}

		ioctl ( dev , PI_CMD_STOP, sizeof ( struct ioctl_info ) );
		if ( value < 60 ) {
//			sleep ( MOVE_TIME ) ;
			ioctl ( dev , PI_CMD_BACKWARD , sizeof ( struct ioctl_info ) );
		
			printf ("backward --> (%u)\n", value);

			usleep ( 700 * ONE_MILI_SEC ) ;

			ioctl ( dev , PI_CMD_STOP, sizeof ( struct ioctl_info ) );

			printf ("stop --> (%u)\n", value);
		}
//		sleep ( MOVE_TIME ) ;

	} else {
	
		ioctl ( dev , PI_CMD_FORWARD , sizeof ( struct ioctl_info ) );


		if ( !(step++ % LOOP_COUNT)) printf (	"forward --> (%u)\n", value);

			step = 0;

		}
	}

	ioctl ( dev , PI_CMD_STOP , sizeof ( struct ioctl_info ) ) ;

	close ( sr04 ) ;
	close ( ir [ 0 ] ) ;
	close ( ir [ 1 ] ) ;
	close ( dev ) ;	

	return 0 ;
}

