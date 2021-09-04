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

#include "ioctl_car_cmd.h"

#define AVOID_DIST	20
#define LOOP_COUNT	100
#define MOVE_TIME	1
#define TURN_TIME	500
#define DETECT_TIME	100
#define ONE_MILI_SEC	1000	

bool isRunning = true ;

void sigHandler ( int dummy ) {
	isRunning = false ;
}

int main () {

	int dev ;
	int sr04 ; 
	int isLeft = 0 ;
	unsigned int step = 0;

	dev = open ( DEVNAME , O_RDWR ) ;

	if ( dev < 0 ) { exit ( 1 ) ; }

	sr04 = open ( SR04 , O_RDONLY ) ;

	if ( sr04 < 0 ) { exit ( 1 ) ; }

	signal ( SIGINT , sigHandler ) ;

	while ( isRunning ) {

		int 	ret ;
		ssize_t value ;

		ret = read ( sr04 , &value , sizeof ( ssize_t ) ) ;

		if ( value < AVOID_DIST ) {

			if ( ioctl ( dev , PI_CMD_STOP, sizeof ( struct ioctl_info ) ) < 0 ) continue;

			printf ("stop --> (%lu)\n", value);

			sleep ( MOVE_TIME ) ;

			if ( ioctl ( dev , PI_CMD_BACKWARD , sizeof ( struct ioctl_info ) )  < 0 ) continue;

			printf ("backward --> (%lu)\n", value);

			sleep ( MOVE_TIME ) ;

			if ( ioctl ( dev , PI_CMD_STOP, sizeof ( struct ioctl_info ) ) < 0 ) continue;

			printf ("stop --> (%lu)\n", value);

			sleep ( MOVE_TIME ) ;

			isLeft = rand () % 2 ;

			if ( isLeft ) {
				ret = read ( sr04 , &value , sizeof ( ssize_t ) ) ;
				if ( ioctl ( dev , PI_CMD_LEFT , sizeof ( struct ioctl_info ) ) < 0 ) continue;
				printf ("left --> (%lu)\n", value);
				usleep ( TURN_TIME * ONE_MILI_SEC ) ;
			} else {
				ret = read ( sr04 , &value , sizeof ( ssize_t ) ) ;
				if ( ioctl ( dev , PI_CMD_RIGHT , sizeof ( struct ioctl_info ) )  <0 ) continue;
				printf ("right --> (%lu)\n", value);
				usleep ( TURN_TIME * ONE_MILI_SEC ) ;

			}

			if ( ioctl ( dev , PI_CMD_STOP, sizeof ( struct ioctl_info ) ) < 0 ) continue;

			sleep ( MOVE_TIME ) ;

		} else {
			
			if ( ioctl ( dev , PI_CMD_FORWARD , sizeof ( struct ioctl_info ) ) < 0 ) continue;

			if ( !(step++ % LOOP_COUNT)) printf (	"forward --> (%lu)\n", value);

			step = 0;

			usleep ( DETECT_TIME * ONE_MILI_SEC ) ;
		}
	}

	ioctl ( dev , PI_CMD_STOP , sizeof ( struct ioctl_info ) ) ;

	close ( sr04 ) ;
	close ( dev ) ;	

	return 0 ;
}

