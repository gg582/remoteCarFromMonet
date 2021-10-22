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
#define SLEEP_TIME	200
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
	u_int32_t value , ir0 , ir1 ;

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


		ret = read ( ir [ 0 ]  , &ir0 , sizeof ( u_int32_t ) ) ;

		if ( ret < 0 ) {
			ioctl ( dev , PI_CMD_STOP, sizeof ( struct ioctl_info ) );
			continue;
		}

		ret = read ( ir [ 1 ] , &ir1 , sizeof ( u_int32_t ) ) ;

		if ( ret < 0 ) {
			ioctl ( dev , PI_CMD_STOP, sizeof ( struct ioctl_info ) );
			continue;
		}

		if ( value < AVOID_DIST ) {

			if ( ir0 && ir1 ) {

					ioctl ( dev , PI_CMD_BACKWARD , sizeof ( struct ioctl_info ) );
					usleep ( SLEEP_TIME * ONE_MILI_SEC *1.5 ) ;
					ioctl ( dev , PI_CMD_RIGHT , sizeof ( struct ioctl_info )) ;

					usleep ( SLEEP_TIME * ONE_MILI_SEC ) ;
					printf ("right --> (%u)\n", value);

			} else if ( ir0 ) {

				ioctl ( dev , PI_CMD_RIGHT , sizeof ( struct ioctl_info )) ;

				usleep ( SLEEP_TIME * ONE_MILI_SEC ) ;
				printf ("right --> (%u)\n", value);

			} else if ( ir1 ) {
			
				ioctl ( dev , PI_CMD_LEFT , sizeof ( struct ioctl_info ) );

				usleep ( SLEEP_TIME * ONE_MILI_SEC ) ;

				printf ("left --> (%u)\n", value);
			} else {

				ioctl ( dev , PI_CMD_BACKWARD , sizeof ( struct ioctl_info ) );
				usleep ( SLEEP_TIME * ONE_MILI_SEC *1.5 ) ;


				if ( rand () % 2 ) {
					
					ioctl ( dev , PI_CMD_RIGHT , sizeof ( struct ioctl_info ) );

					usleep ( SLEEP_TIME * ONE_MILI_SEC ) ;

				} else {

					ioctl ( dev , PI_CMD_LEFT , sizeof ( struct ioctl_info ) );

					usleep ( SLEEP_TIME * ONE_MILI_SEC ) ;


				}
			}

		} else {

			ioctl ( dev , PI_CMD_FORWARD , sizeof ( struct ioctl_info ) );

			usleep ( SLEEP_TIME * ONE_MILI_SEC ) ;
		}
		
	}


	close ( sr04 ) ;
	close ( ir [ 0 ] ) ;
	close ( ir [ 1 ] ) ;
	close ( dev ) ;	

	return 0 ;
}


