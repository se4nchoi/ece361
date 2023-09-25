// simple client and server program that communicate with UDP sockets for sending and receiving data.

// for now, the server will run with command server <UDP listen port>
// and client will run with command deliver <server address> <server port number>

// make file should execute 
// 1. run the server with the above command
//  a) open a udp socket and listen at the specified port number
//  b) receive a message from the client, if the message is "ftp", send a message "yes" to client and exit if not "ftp"
// 2. run the client program, which will 
//  a) ask for a user input message in the command format : ftp <file name>
//  b) check existence of the requested file, if exists send a message "ftp" to server and exit if not exist
//  c) receive message from the server, if the message is "yes", print out "A file transfer can start" and exit if message is not "yes"

#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h> // read(), write(), close()

#define SA struct sockaddr

void connection(int connfd) {

}

int main() {
  int sockfd, connfd, len;
  struct sockaddr_in servaddr, cli;

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd == -1) {
    printf("socket creation failed...\n");
  }

  printf("hello world, server.\n");

  return 0;
}