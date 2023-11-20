#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/stat.h>

#define SA struct sockaddr 

int
main(int argc, char * argv[])
{
    struct sockaddr_in client_addr, server_addr, cli;
    socklen_t server_addrlen, client_addrlen;
    int sockfd, connfd, len;
    

    char* port;
    int port_int;

    char buf[2048];

    if (argc==2) {
        port = argv[1];
        port_int = atoi(port);
    }
    else {
        fprintf(stderr, "need 2 arguments: server <TCP listen port> \n");
        exit(1);
    }

    /* build address data structure */
    bzero((char *)&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port_int);
    server_addrlen = sizeof(server_addr);

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Failed to connect");
        exit(1);
    }

    if ((bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr))) != 0) {
        perror("[S] simplex-talk: bind");
        exit(1);
    }

    if ((listen(sockfd, 5)) != 0) { 
        printf("Listen failed...\n"); 
        exit(0); 
    } 
    else
        printf("Server listening..\n"); 
    
    len = sizeof(cli);
    connfd = accept(sockfd, (SA*)&cli, &len); 
    if (connfd < 0) { 
        printf("server accept failed...\n"); 
        exit(0); 
    } 
    else
        printf("server accept the client...\n"); 

    while (1) {
        bzero(buf, sizeof(buf));
        read(connfd, buf, sizeof(buf));
        printf("Server received: %s\n", buf);
        // if (strcmp(buf, "login")==0) {
        //     printf("before sent ack\n");
        //     send(connfd, "ack", 3, 0);
        //     printf("after sent ack\n");
        // } else {
        //     send(connfd, "nack", 4, 0);
        // }
        write(connfd, "ack", 3);
    }


    return 0;
}