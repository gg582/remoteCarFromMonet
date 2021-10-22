#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <stdbool.h>
#include "../common/ioctl_car_cmd.h"

#define CMD_LEN 5
#define CMP_LEN 4
#define DEVNAME 		"/dev/car/motor"
#define DIR_FORWARD		"FWRD"
#define DIR_LEFT		"LEFT"
#define DIR_RIGHT		"RGHT"
#define DIR_BACKWARD		"BKWD"
#define DIR_TERMINATION		"TERM"

void error(char *msg) {
  perror(msg);
  exit(1);
}

int main(int argc, char **argv) {

  int parentfd; /* parent socket */
  int childfd; /* child socket */
  int portno; /* port to listen on */
  int clientlen; /* byte size of client's address */
  struct sockaddr_in serveraddr; /* server's addr */
  struct sockaddr_in clientaddr; /* client addr */
  struct hostent *hostp; /* client host info */
  char buf[CMD_LEN]; /* message buffer */
  char *hostaddrp; /* dotted decimal host addr string */
  int optval; /* flag value for setsockopt */

  /* 
   * check command line arguments 
   */
  portno = 10102 ;

  /* 
   * socket: create the parent socket 
   */
  parentfd = socket(AF_INET, SOCK_STREAM, 0);
  if (parentfd < 0) 
    error("ERROR opening socket");

  /* setsockopt: Handy debugging trick that lets 
   * us rerun the server immediately after we kill it; 
   * otherwise we have to wait about 20 secs. 
   * Eliminates "ERROR on binding: Address already in use" error. 
   */
  optval = 1;

  setsockopt(parentfd, SOL_SOCKET, SO_REUSEADDR, 
	     (const void *)&optval , sizeof(int));

  /*
   * build the server's Internet address
   */
  bzero((char *) &serveraddr, sizeof(serveraddr));

  /* this is an Internet address */
  serveraddr.sin_family = AF_INET;

  /* let the system figure out our IP address */
  serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);

  /* this is the port we will listen on */
  serveraddr.sin_port = htons((unsigned short)portno);

  /* 
   * bind: associate the parent socket with a port 
   */
  if (bind(parentfd, (struct sockaddr *) &serveraddr, 
	   sizeof(serveraddr)) < 0) 
    error("ERROR on binding");

  /* 
   * listen: make this socket ready to accept connection requests 
   */
  if (listen(parentfd, 5) < 0) /* allow 5 requests to queue up */ 
    error("ERROR on listen");

  /* 
   * main loop: wait for a connection request, echo input line, 
   * then close connection.
   */

  clientlen = sizeof(clientaddr);

  childfd = accept(parentfd, (struct sockaddr *) &clientaddr, &clientlen);


  if (childfd < 0) 
    error("ERROR on accept");
 
  int motor = open ( DEVNAME, O_RDWR ) ;

  while (true) {

    int recvSize = 0;
    int n = 0;

    bzero(buf, CMD_LEN);

    recvSize = read(childfd, buf, CMD_LEN );

    if (recvSize < 0) {
      error("ERROR reading from socket");
    }

    if ( recvSize < 5 ) {
        printf("server received %d bytes: %s", recvSize , buf);
    	printf("We have lost the connection\n");
	break;
    }
        printf("server received %d bytes: %s", recvSize , buf);

    if ( strncmp ( buf , DIR_FORWARD , CMP_LEN) == 0 ) {
        printf("DIRECTION --> FORWARD\n");
    	ioctl ( motor , PI_CMD_FORWARD) ;
    } else if ( strncmp ( buf , DIR_LEFT , CMP_LEN) == 0 ) {
        printf("DIRECTION --> LEFT\n");
	ioctl ( motor , PI_CMD_LEFT) ;
    } else if ( strncmp ( buf , DIR_RIGHT,CMP_LEN) == 0 ) {
        printf("DIRECTION --> RIGHT\n");
	ioctl ( motor , PI_CMD_RIGHT ) ;
    } else if ( strncmp ( buf , DIR_BACKWARD,CMP_LEN) == 0 ) {
        printf("DIRECTION --> BACKWARD\n");
	ioctl ( motor , PI_CMD_BACKWARD) ;
    } else if ( strncmp ( buf , DIR_TERMINATION, CMP_LEN) == 0 ) {
        printf("DIRECTION --> TERMINATION\n");
	ioctl ( motor , PI_CMD_STOP) ;
    } else {
        printf("DEFAULT DIRECTION --> STOP\n");
	ioctl ( motor , PI_CMD_STOP) ;
    }
  }
    close(childfd);
}
