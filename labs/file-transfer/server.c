#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define MAX_PENDING 5
#define MAX_LINE 256

int
main(int argc, char * argv[])
{
    struct sockaddr_in client_addr, server_addr;
    socklen_t server_addrlen, client_addrlen;
    char buf[MAX_LINE];
    int buf_len;
    int s, new_s;
    int nBytes;

    char *host, *port;
    if (argc==2) {
        port = argv[1];
    }
    else {
        fprintf(stderr, "need 2 arguments: server <UDP listen port> \n");
        exit(1);
    }

    /* build address data structure */
    bzero((char *)&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(5432);
    server_addrlen = sizeof(server_addr);


    if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("simplex-talk: socket");
        exit(1);
    }

    if ((bind(s, (struct sockaddr *)&server_addr, sizeof(server_addr))) < 0) {
        perror("simplex-talk: bind");
        exit(1);
    }

    printf("Done with binding");

    /* wait for connection, then receive and print text */
    while(1) {
        /* print information of sin */
        printf("sin_port: %d\n", server_addr.sin_port);
        printf("s_addr: %d\n", ntohl(server_addr.sin_addr.s_addr));

        printf("\nWaiting for messages ...\n");
        nBytes = recvfrom(s, (char *) buf, MAX_LINE, 0, (struct sockaddr *)&client_addr, &client_addrlen);
        buf[nBytes] = '\0';
        printf("%d Bytes received\n", nBytes);

        if (strcmp(buf, "ftp")==0) {
            printf("ftp received, sending acknowledgement\n");
            sendto(s, "yes", 3, 0, (struct sockaddr*)&client_addr, client_addrlen);
            printf("acknowledgement sent\n");
        } else {
            printf("Unknown message: %s\n", buf);
        }

        close(s);
        exit(1);
    }
}