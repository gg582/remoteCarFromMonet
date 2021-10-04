#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <linux/types.h>
#include <signal.h>
#include <sys/ioctl.h>

#define DEVNAME "/dev/car/motor_tun"
#define SR04 "/dev/car/sr04"
#define IR0 "/dev/car/ir0"
#define IR1 "/dev/car/ir1"

#include <ioctl_car_cmd.h>

#define AVOID_DIST  150
#define LOOP_COUNT	1000
#define MOVE_TIME	1
#define SLEEP_TIME	700
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

		ret = read ( sr04 , &value , sizeof ( u_int32_t ) ) ;
		if ( ret < 0 ) ioctl ( dev , PI_CMD_STOP , sizeof ( struct ioctl_info ) );
		ret = read ( ir [ 0 ]  , &ir0 , sizeof ( u_int32_t ) ) ;
		if ( ret < 0 ) ioctl ( dev , PI_CMD_STOP , sizeof ( struct ioctl_info ) ) ;
		ret = read ( ir [ 1 ] , &ir1 , sizeof ( u_int32_t ) ) ;
		if ( ret < 0 )  ioctl ( dev , PI_CMD_STOP , sizeof ( struct ioctl_info ) ) ;

		

	if ( value < AVOID_DIST ) {


		if ( ir0 && ir1 ) {

			ioctl ( dev , PI_CMD_BACKWARD , sizeof ( struct ioctl_info ) );
			usleep ( 700 * ONE_MILI_SEC ) ;
			ioctl ( dev , PI_CMD_STOP, sizeof ( struct ioctl_info ) );
			usleep ( 700 * ONE_MILI_SEC ) ;
		}
		else if ( ir0 ) {
			ioctl ( dev , PI_CMD_LEFT , sizeof ( struct ioctl_info )) ;
			while ( ir0 || ir1 ) {
				ret = read ( ir [ 0 ]  , &ir0 , sizeof ( u_int32_t ) ) ;
				ret = read ( ir [ 1 ]  , &ir1 , sizeof ( u_int32_t ) ) ;
			}
			printf ("left --> (%u)\n", value);
			ioctl ( dev , PI_CMD_STOP, sizeof ( struct ioctl_info ) );
			} else if ( ir1 ) {
		
				ioctl ( dev , PI_CMD_RIGHT , sizeof ( struct ioctl_info ) );
				while ( ir0 || ir1 ) {
					ret = read ( ir [ 0 ]  , &ir0 , sizeof ( u_int32_t ) ) ;
					ret = read ( ir [ 1 ]  , &ir1 , sizeof ( u_int32_t ) ) ;
				}
				ioctl ( dev , PI_CMD_STOP, sizeof ( struct ioctl_info ) );
				printf ("right --> (%u)\n", value);
	
			}

		} else {
		
			if ( ir0 && ir1 ) {

				ioctl ( dev , PI_CMD_BACKWARD , sizeof ( struct ioctl_info ) );
				usleep ( 700 * ONE_MILI_SEC ) ;
				ioctl ( dev , PI_CMD_STOP, sizeof ( struct ioctl_info ) );
				usleep ( 700 * ONE_MILI_SEC ) ;
			}
			else if ( ir0 ) {
				ioctl ( dev , PI_CMD_LEFT , sizeof ( struct ioctl_info )) ;
				while ( ir0 || ir1 ) {
					ret = read ( ir [ 0 ]  , &ir0 , sizeof ( u_int32_t ) ) ;
					ret = read ( ir [ 1 ]  , &ir1 , sizeof ( u_int32_t ) ) ;
				}
				printf ("left --> (%u)\n", value);
				} else if ( ir1 ) {
			
					ioctl ( dev , PI_CMD_RIGHT , sizeof ( struct ioctl_info ) );
					while ( ir0 || ir1 ) {
						ret = read ( ir [ 0 ]  , &ir0 , sizeof ( u_int32_t ) ) ;
						ret = read ( ir [ 1 ]  , &ir1 , sizeof ( u_int32_t ) ) ;
					}


					ioctl ( dev , PI_CMD_STOP, sizeof ( struct ioctl_info ) );
					usleep ( 700 * ONE_MILI_SEC ) ;
					printf ("right --> (%u)\n", value);

				} else {
					ioctl ( dev , PI_CMD_FORWARD , sizeof ( struct ioctl_info ) );
					usleep ( 700 * ONE_MILI_SEC ) ;
					ioctl ( dev , PI_CMD_STOP, sizeof ( struct ioctl_info ) );
					usleep ( 700 * ONE_MILI_SEC ) ;
				}


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


