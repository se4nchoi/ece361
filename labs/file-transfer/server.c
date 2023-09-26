#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define MAX_PENDING 5
#define MAX_LINE 256

// // function to receive file
// int recvFile(char* buf, int s)
// {
//     int i;
//     char ch;
//     for (i = 0; i < s; i++) {
//         ch = buf[i];
//         if (ch == EOF)
//             return 1;
//         else
//             printf("%c", ch);
//     }
//     return 0;
// }

int
main(int argc, char * argv[])
{
    struct sockaddr_in sin;
    char buf[MAX_LINE];
    int buf_len, addr_len;
    int s, new_s;
    int nBytes;

    int *port;
    if (argc==2) {
        port = argv[1];
    }
    else {
        fprintf(stderr, "need 2 arguments: server <UDP listen port> \n");
        exit(1);
    }


    /* build address data structure */
    bzero((char *)&sin, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons(port);
    int addrlen = sizeof(sin);


    if ((s = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("simplex-talk: socket");
        exit(1);
    }

    if ((bind(s, (struct sockaddr *)&sin, sizeof(sin))) < 0) {
        perror("simplex-talk: bind");
        exit(1);
    }

    /* wait for connection, then receive and print text */
    while(1) {
        /* print information of sin */
        printf("Numeric: %u\n", ntoh(sin.sin_addr.s_addr));
        printf("sin_port: %d\n", sin.sin_port);
        printf("s_addr: %d\n", ntoh(sin.sin_addr.s_addr));

        printf("\nWaiting for file ...\n");
        nBytes = recvfrom(s, buf, MAX_LINE, 0, (struct sockaddr *)&sin, addrlen);
        
        printf("%d Bytes received", nBytes);

        close(s);
        exit(1);
    }
}