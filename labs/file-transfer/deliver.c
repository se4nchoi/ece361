#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX_LINE 256

bool fileExists (char *filename) {
  FILE *fp = fopen(filename, "r");
  if (fp) {
    fclose(fp);
    return true;
  }
  return false;
}

int main(int argc, char **argv){
 
  if (argc != 2) {
    printf("Usage: %s <server address> <server port number>\n", argv[0]);
    exit(1);
  }

  struct hostent *host = argv[1];
  int port = atoi(argv[2]);
  struct sockaddr_in sin;
  
  char buffer[MAX_LINE];
  socklen_t addr_size;

  bzero((char *)&sin, sizeof(sin));
  sin.sin_family = AF_INET;
  bcopy(hp->h_addr, (char *)&sin.sin_addr, hp->h_length);
  sin.sin_port = htons(port);
  int addrlen = sizeof(sin);

  int sockfd;

  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("[-]socket error");
    exit(1);
  }

  print("Type in filename (ex. ftp <file name>)\n");
  scanf("%s", buffer);
  if (buffer[0:3] != "ftp ") {
    printf("Invalid command\n");
    exit(1);
  }
  
  char *filename = buffer[4:];
  if (fileExists(filename)) {
    // ping server with "ftp"
    sendto(sockfd, "ftp", 3, 0, (struct sockaddr*)&sin, addrlen);
    printf("[+] FTP ping sent\n");
    // wait for "yes"
    
    bzero(buffer, MAX_LINE);
    recvfrom(sockfd, buffer, 0, (struct sockaddr*)&sin, &addrlen);
    printf("[+] Ack recv: %s\n", buffer);

    if (buffer[0:2] != "yes") {
      printf("Server did not respond with \"yes\"\n");
      exit(1);
    } else {
      printf("A file transfer can start.\n");

    }

  } else {
    printf("File does not exist\n");
    exit(1);
  }
}