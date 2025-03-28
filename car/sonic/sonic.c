#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <time.h>
#include <stdbool.h>

int main ()
{

    int myDevice = open ( "/dev/car/sr04", O_RDWR ) ;
    int num = 0 ;
    int i ;
    unsigned long val = 0 ;
    int ret;
    char buf [255];

    if ( myDevice == -1 ) {
        perror ( "Open Funtion Failed" ) ;
        return -1 ;
    }

    ret = read ( myDevice, & val, sizeof ( unsigned long ) ) ;

    memset  ( buf, 0x00, sizeof (buf));
    sprintf ( buf, "%d\n", val ) ;
    write (1, buf, strlen (buf));

    close ( myDevice ) ;

    return 0 ;
}

