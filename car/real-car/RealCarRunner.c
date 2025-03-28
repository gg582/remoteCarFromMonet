#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <netdb.h>
#include <fcntl.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include "../common/ioctl_car_cmd.h"

#define CMD_LEN 100
#define CMP_LEN 4
#define DEVNAME 		"/dev/car/motor"
#define DIR_FORWARD		"FWRD"
#define DIR_LEFT		"LEFT"
#define DIR_RIGHT		"RGHT"
#define DIR_BACKWARD		"BKWD"
#define DIR_TERMINATION		"TERM"

void error(char *msg)
{
    perror(msg);
    exit(1);
}
int motor ;

void sigHandler ( int dummy )
{

    ioctl ( motor, PI_CMD_STOP) ;
    close ( motor ) ;
    exit ( 0 ) ;

}
void Clock ( int64_t *val )
{
    struct timespec ts;
    timespec_get(&ts,TIME_UTC);

}
int main(int argc, char **argv)
{

    int parentFd; /* parent socket */
    int childFd; /* child socket */
    int portNum; /* port to listen on */
    unsigned int clientLen; /* byte size of client's address */
    struct sockaddr_in serverAddr; /* server's addr */
    struct sockaddr_in clientAddr; /* client addr */
    char buf[CMD_LEN]; /* message buffer */
    int optVal; /* flag value for setsockopt */
    signal ( SIGKILL, sigHandler ) ;
    printf ( "Car begins.\n" );

    /*
     * check command line arguments
     */
    portNum = 10102 ;

    /*
     * socket: create the parent socket
     */
    parentFd = socket(AF_INET, SOCK_STREAM, 0);
    if (parentFd < 0) {

        error("ERROR opening socket");
    }

    /* setsockopt: Handy debugging trick that lets
     * us rerun the server immediately after we kill it;
     * otherwise we have to wait about 20 secs.
     * Eliminates "ERROR on binding: Address already in use" error.
     */
    optVal = 1;

    setsockopt(parentFd, SOL_SOCKET, SO_REUSEADDR,
               (const void *)&optVal, sizeof(int));

    /*
     * build the server's Internet address
     */
    bzero((char *) &serverAddr, sizeof(serverAddr));

    /* this is an Internet address */
    serverAddr.sin_family = AF_INET;

    /* let the system figure out our IP address */
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    /* this is the port we will listen on */
    serverAddr.sin_port = htons((unsigned short)portNum);

    /*
     * bind: associate the parent socket with a port
     */
    if (bind(parentFd, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) < 0) {
        error("ERROR on binding");
    }

    /*
     * listen: make this socket ready to accept connection requests
     */

    printf ( "Waiting for connection\n" ) ;

    if (listen(parentFd, 1) < 0) { /* allow 1 requests to queue up */
        error("ERROR on listen");
    }

    /*
     * main loop: wait for a connection request, echo input line,
     * then close connection.
     */

    clientLen = sizeof(clientAddr);

    childFd = accept(parentFd, (struct sockaddr *) &clientAddr, &clientLen);

    if (childFd < 0) {
        error("ERROR on accept");
    }

    printf ( "Connection established\n" ) ;

    motor = open ( DEVNAME, O_RDWR ) ;

    char cmd [ 5 ] ;
    int recvSize ;
    while (1) {


        bzero(buf, CMD_LEN);

        recvSize = read (childFd, buf,  CMD_LEN );

        if ( ( recvSize == 0) ) {
            printf ( "ERROR on reading\n" );
            exit ( 1 ) ;
        }

        printf ( "received json string is : %s\n",  buf ) ;
        char timestampGetString [ 95 ] ;

        int64_t timestampOld, nsecTS;


        sscanf ( buf,  "{\"MotorBytes\":\"%[^\"]\",\"TimeStamp\":\"%u\"}\n%*[^\n]", cmd, &timestampOld ) ;

        Clock (&nsecTS);

        printf ( "time Delay : %lld ( nsec ) \n",nsecTS - timestampOld);
        printf ( "COMMAND from carcon : %s\n", cmd ) ;

        if ( strncmp ( cmd, DIR_FORWARD, CMP_LEN) == 0 ) {
            printf("DIRECTION --> FORWARD\n");
            ioctl ( motor, PI_CMD_FORWARD) ;
        } else if ( strncmp ( cmd, DIR_LEFT, CMP_LEN) == 0 ) {
            printf("DIRECTION --> LEFT\n");
            ioctl ( motor, PI_CMD_LEFT) ;
        } else if ( strncmp ( cmd, DIR_RIGHT,CMP_LEN) == 0 ) {
            printf("DIRECTION --> RIGHT\n");
            ioctl ( motor, PI_CMD_RIGHT ) ;
        } else if ( strncmp ( cmd, DIR_BACKWARD,CMP_LEN) == 0 ) {
            printf("DIRECTION --> BACKWARD\n");
            ioctl ( motor, PI_CMD_BACKWARD) ;
        } else if ( strncmp ( cmd, DIR_TERMINATION, CMP_LEN) == 0 ) {
            printf("DIRECTION --> TERMINATION\n");
            ioctl ( motor, PI_CMD_STOP) ;
            break;
        } else {
            printf("DEFAULT DIRECTION --> STOP\n");
            ioctl ( motor, PI_CMD_STOP) ;
        }
    }
    close(childFd);
}
