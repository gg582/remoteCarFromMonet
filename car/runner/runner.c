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
	if ( dev < 0 ) {
		exit ( 1 ) ;
	}
	sr04 = open ( SR04 , O_RDONLY ) ;
	if ( sr04 < 0 ) {
		exit ( 1 ) ;
	}
	signal ( SIGINT , sigHandler ) ;

	while ( isRunning ) {

		int ret ;
		ssize_t value ;

		ret = read ( sr04 , &value , sizeof ( ssize_t ) ) ;

		if ( value < 10 ) {

			ioctl ( dev , PI_CMD_STOP, sizeof ( struct ioctl_info ) ) ;
			sleep ( 1 ) ;
			ioctl ( dev , PI_CMD_BACKWARD , sizeof ( struct ioctl_info ) ) ;
			printf ("backward --> (%d)\n", value);
			sleep ( 1 ) ;
			ioctl ( dev , PI_CMD_STOP, sizeof ( struct ioctl_info ) ) ;
			sleep ( 1 ) ;

			isLeft = rand () % 2 ;
			if ( isLeft ) {
				ioctl ( dev , PI_CMD_LEFT , sizeof ( struct ioctl_info ) ) ;
				printf ("left --> (%d)\n", value);
				sleep ( 1 ) ;
			} else {
				ioctl ( dev , PI_CMD_RIGHT , sizeof ( struct ioctl_info ) ) ;
				printf ("right --> (%d)\n", value);
				sleep ( 1 ) ;
			}
			ioctl ( dev , PI_CMD_STOP, sizeof ( struct ioctl_info ) ) ;
			sleep ( 1 ) ;
		} else {
			
			ioctl ( dev , PI_CMD_FORWARD , sizeof ( struct ioctl_info ) ) ;
			if ( !(step++ % 100)) printf ("forward --> (%d)\n", value);
		}
	}

	ioctl ( dev , PI_CMD_STOP , sizeof ( struct ioctl_info ) ) ;
	close ( sr04 ) ;
	close ( dev ) ;	
	return 0 ;
}

