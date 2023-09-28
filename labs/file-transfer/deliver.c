#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdbool.h>

#define MAX_LINE 256

bool fileExists (char *filename) {
    printf("%s\n", filename);
    FILE *fp = fopen(filename, "r");
    if (fp) {
        fclose(fp);
        return true;
    }
    return false;
}

int
main(int argc, char * argv[])
{
    FILE *fp;
    struct hostent *hp;
    struct sockaddr_in sin;
    char *host, *port;
    char buf[MAX_LINE];
    int s;
    int len;

    if (argc==3) {
        host = argv[1];
        port = argv[2];
    }
    else {
        fprintf(stderr, "Input: deliver <server address> <server port number>\n");
        exit(1);
    }

    /* translate host name into peer's IP address */
    hp = gethostbyname(host);
    if (!hp) {
        fprintf(stderr, "simplex-talk: unknown host: %s\n", host);
        exit(1);
    }

    /* build address data structure */
    bzero((char *)&sin, sizeof(sin));
    sin.sin_family = AF_INET;
    bcopy(hp->h_addr_list[0], (char *)&sin.sin_addr, hp->h_length);
    /* TODO: make it such that it can take any port */
    sin.sin_port = htons(5432);
    socklen_t addrlen = sizeof(sin);

    /* set up socket */
    if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket not set up\n");
        exit(1);
    }

    printf("Type in filename (ex. ftp <file name>):\n");
    gets(buf);

    char substr[5];

    memset(substr, '\0', 4);
    strncpy(substr, buf, 3);
    if (strcmp(substr,"ftp")==0) {
        printf("Command received: %s\n", substr);
    } else {
        printf("Invalid command\n");
        exit(1);
    }
    
    char* filename = &buf[4];
    if (fileExists(filename)) {
        // ping server with "ftp"
        sendto(s, "ftp", 3, 0, (struct sockaddr*)&sin, addrlen);
        printf("[+] FTP ping sent\n");
        // wait for "yes"
        bzero(buf, MAX_LINE);
        recvfrom(s, (char *) buf, MAX_LINE, 0, (struct sockaddr*)&sin, &addrlen);
        printf("[+] Ack recv: %s\n", buf);

        strncpy(substr, buf, 3);
        if (strcmp(substr, "yes") != 0) {
            printf("Server did not respond with \"yes\"\n");
            exit(1);
        } else {
            printf("A file transfer can start.\n");
        }

    } else {
        printf("File does not exist\n");
        exit(1);
    }

    close(s);
    exit(1);

    // /* main loop: get and send lines of text */
    // while (1) {
    //     /* print information of sin*/
    //     printf("sin_port: %d\n", sin.sin_port);
    //     printf("s_addr: %d\n", ntohl(sin.sin_addr.s_addr));

    //     // 1. ask for "ftp <filename>"
    //     // 2. check existence of file; exit if not exist; send ftp to server
    //     // 3. wait to receive from server

    //     fgets(buf, sizeof(buf), stdin);

    //     sendto(s, (char *) buf, MAX_LINE, 0, (struct sockaddr *)&sin, addrlen);

    //     printf("Message sent");
    //     close(s);
    //     exit(1);
    // }

}
