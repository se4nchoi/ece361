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

#define MAX_CHAR 128

int login(char (*parsedCommands)[MAX_CHAR]) {
    printf("doing login\n");

    char *host, *port;
    struct sockaddr_in client_addr, server_addr;
    socklen_t server_addrlen, client_addrlen;
    struct hostent *hp;
    int port_int;

    int s;

    host = parsedCommands[3];
    port = parsedCommands[4];
    port_int = atoi(port);

    hp = gethostbyname(host);
    if (!hp) {
        fprintf(stdin, "unknown host: %s\n", host);
        return; 
    }

    /* build address data structure */
    bzero((char *)&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    bcopy(hp->h_addr_list[0], (char *)&server_addr.sin_addr, hp->h_length);
    server_addr.sin_port = htons(port_int);
    server_addrlen = sizeof(server_addr);

     /* set up socket */
    if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("[C] Socket setup failed\n");
        return -1; 
    }
        
    // connect the client socket to server socket
    if (connect(s, (SA*)&server_addr, sizeof(server_addr))
        != 0) {
        printf("connection with the server failed...\n");
        exit(0);
    }
    else
        printf("connected to the server..\n");
    
    char message[2048];
    char buf[2048];
    send(s, "login", 5, 0);
    recv(s, buf, sizeof(buf), 0);

    if (strcmp(buf, "ack")==0) {
        printf("Acknowledged\n");
    } else {
        printf("Not acknowledged\n");
        close(s);
    }
    return s;
}

void logout() {
    printf("doing logout\n");
}

void joinsession(char (*parsedCommands)[MAX_CHAR]) {
    printf("doing joinsession\n");

}

void leavesession() {
    printf("doing leavesession\n");

}

void createsession(char (*parsedCommands)[MAX_CHAR]) {
    printf("doing createsession\n");

}

void runList() {
    printf("doing list\n");

}

void runQuit() {
    printf("doing quit\n");

}