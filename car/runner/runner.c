#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <sys/time.h>
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

int64_t Clock ()
{

    struct timeval tsp;
    gettimeofday ( &tsp, NULL ) ;
    return ( int64_t )tsp.tv_usec ;

}

void sigHandler ( int dummy )
{

    fclose ( clockLog ) ;
    fclose ( motorclockLog ) ;
    close ( sr04 ) ;
    close ( ir [ 0 ] ) ;
    close ( ir [ 1 ] ) ;
    close ( dev ) ;

    ioctl ( dev, PI_CMD_STOP, sizeof ( struct ioctl_info ) ) ;
    fprintf ( motorclockLog, "/dev/car/motor: %ld\n", Clock () ) ;
    exit ( 0 ) ;
}

int main ()
{

    u_int32_t dist=0, irLeft =0, irRight=0, prev_dist = 0;
    u_int8_t mean_diff = 0;
    u_int64_t diff = 0;
    u_int16_t accum_length = 0;

    motorclockLog= fopen ( WR_LOGNAME, "wt" );
    clockLog     = fopen ( LOGNAME, "rt" ) ;


    puts ( "Runner begins" ) ;

    dev = open ( DEVNAME, O_RDWR ) ;
    ir [ 0 ] = open ( IR0, O_RDONLY ) ;
    ir [ 1 ] = open ( IR1, O_RDONLY) ;


    sr04 = open ( SR04, O_RDONLY ) ;


    signal ( SIGKILL, sigHandler ) ;

    while ( true ) {

        prev_dist = dist;
        read ( sr04, &dist, sizeof ( u_int32_t ) ) ;
        int64_t tstamp=Clock () ;
        int64_t clockPrev = 0 ;

        fscanf ( clockLog, "%*[^\n]/dev/car/sr04_tun: %ld%*[^\n]", &clockPrev ) ;

        printf ( "sr04_delay : %ld\n", tstamp - clockPrev );


        tstamp=Clock () ;

        clockPrev = 0 ;


        read ( ir [ 0 ], &irLeft, sizeof ( u_int32_t ) ) ;

        fscanf ( clockLog, "%*[^\n]/dev/car/left_ir_tun: %ld%*[^\n]", &clockPrev ) ;

        tstamp=Clock () ;
        printf ( "left_ir_delay : %ld\n", (int64_t) ( tstamp - clockPrev )  ) ;




        read ( ir [ 1 ], &irRight, sizeof ( u_int32_t ) ) ;
        fscanf ( clockLog, "%*[^\n]/dev/car/right_ir_tun: %ld%*[^\n]", &clockPrev ) ;

        tstamp=Clock () ;

        printf (  "right_ir_delay : %ld\n", (int64_t) ( tstamp - clockPrev )  ) ;

        accum_length++;

        diff += dist - prev_dist > 0? dist - prev_dist: prev_dist - dist;
        mean_diff = diff / accum_length;

        if ( dist < AVOID_DIST ) {



            if ( ( irLeft && irRight ) | ( dist < BACK_DIST ) ) {
                ioctl ( dev, PI_CMD_BACKWARD, sizeof ( struct ioctl_info ) );
                fprintf ( motorclockLog, "/dev/car/motor: %ld\n", Clock () ) ;
                printf ("backward --> (%u)\n", dist) ;


            } else if ( irLeft ) {
                ioctl ( dev, PI_CMD_RIGHT, sizeof ( struct ioctl_info )) ;
                fprintf ( motorclockLog, "/dev/car/motor: %ld\n", Clock () ) ;
                printf ("right --> (%u)\n", dist);

            } else if ( irRight ) {
                ioctl ( dev, PI_CMD_LEFT, sizeof ( struct ioctl_info ) );
                fprintf ( motorclockLog, "/dev/car/motor: %ld\n", Clock () ) ;
                printf ("left --> (%u)\n", dist);
            } else if ( !dist ) {
                ioctl ( dev, PI_CMD_FORWARD ) ;
                fprintf ( motorclockLog, "/dev/car/motor: %ld\n", Clock () ) ;
            } else {
                ioctl ( dev, PI_CMD_LEFT, sizeof ( struct ioctl_info ) );
                fprintf ( motorclockLog, "/dev/car/motor: %ld\n", Clock () ) ;
                printf ("left ( predefined ) --> (%u)\n", dist);
            }


        } else {
            static int counter;

            if ( irLeft && irRight ) {
                ioctl ( dev, PI_CMD_BACKWARD, sizeof ( struct ioctl_info ) );
                fprintf ( motorclockLog, "/dev/car/motor: %ld", Clock () ) ;
                printf ("backward --> (%u)\n", dist) ;


            } else if ( irLeft) {
                ioctl ( dev, PI_CMD_LEFT, sizeof ( struct ioctl_info )) ;
                fprintf ( motorclockLog, "/dev/car/motor: %ld", Clock () ) ;
                printf ("left --> (%u)\n", dist);

            } else if ( irRight ) {

                ioctl ( dev, PI_CMD_RIGHT, sizeof ( struct ioctl_info ) );
                fprintf ( motorclockLog, "/dev/car/motor: %ld", Clock () ) ;
                printf ("right --> (%u)\n", dist);
            } else {
                if ( !(counter++ % 10) ) {
                    printf ("forward --> (%u)\n", dist);
                }
                ioctl ( dev, PI_CMD_FORWARD, sizeof ( struct ioctl_info ) );
                fprintf ( motorclockLog, "/dev/car/motor: %ld\n", Clock () ) ;
            }

        }
    }

    return 0 ;
}


