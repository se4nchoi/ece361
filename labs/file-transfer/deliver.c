#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>


#define MAX_LINE 2048
#define PKT_SIZE 1000

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
    struct sockaddr_in client_addr, server_addr;
    socklen_t server_addrlen, client_addrlen;
    char *host, *port;
    int port_int;
    char buf[MAX_LINE];
    char filename[256];
    int s;
    int len;
    int byte_sent;
    clock_t seconds;

    if (argc==3) {
        host = argv[1];
        port = argv[2];
        port_int = atoi(port);
    }
    else {
        fprintf(stderr, "[C] Input: deliver <server address> <server port number>\n");
        exit(1);
    }

    /* translate host name into peer's IP address */
    hp = gethostbyname(host);
    if (!hp) {
        fprintf(stderr, "[C] simplex-talk: unknown host: %s\n", host);
        exit(1);
    }

    /* build address data structure */
    bzero((char *)&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    bcopy(hp->h_addr_list[0], (char *)&server_addr.sin_addr, hp->h_length);
    server_addr.sin_port = htons(port_int);
    server_addrlen = sizeof(server_addr);

    /* set up socket */
    if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("[C] Socket not set up\n");
        exit(1);
    }

    printf("[C] Type in filename (ex. ftp <file name>):\n");
    gets(buf);

    char substr[5];

    memset(substr, '\0', 4);
    strncpy(substr, buf, 3);
    if (strcmp(substr,"ftp")==0) {
        printf("[C] Command received: %s\n", substr);
    } else {
        printf("[C] Invalid command\n");
        exit(1);
    }

    memset(filename, '\0', strlen(&buf[4])+1);
    strncpy(filename, &buf[4], strlen(&buf[4]));
    if (fileExists(filename)) {
        
        // ping server with "ftp"
        sendto(s, "ftp", 3, 0, (struct sockaddr*)&server_addr, server_addrlen);
        // wait for "yes"
        bzero(buf, MAX_LINE);
        recvfrom(s, (char *) buf, MAX_LINE, 0, (struct sockaddr*)&server_addr, server_addrlen);
        printf("[C] Ack recv: %s\n", buf);

        strncpy(substr, buf, 3);
        if (strcmp(substr, "yes") != 0) {
            printf("[C] Server did not respond with \"yes\"\n");
            exit(1);
        } else {
            printf("[C] A file transfer can start.\n");
        }

        printf("--Section 1 complete--\n");
        /*TODO: 
        n. add eof (0x05) at the end of data array
        
        */

       // check file size
        FILE *stream;
        stream = fopen(filename, "r");
        off_t filesize;
        struct stat st;
        stat(filename, &st);
        filesize = st.st_size;
        printf("[C] Filename: %s, Filesize: %lld\n", filename, filesize);

        int total_frag = 1+(filesize-1)/PKT_SIZE;
        char data[1000];
        char tmp[16];
        unsigned int frag_no;
        unsigned int size;
        unsigned int ptr;
        
       for (unsigned int i=0; i<1; i++) {
            bzero(buf, MAX_LINE);
            buf[0]='\0';
            bzero(tmp, 16);
            tmp[0]='\0';
            ptr = 0;

            size = fread(&data, sizeof(char), PKT_SIZE, stream); 

            // copy fragment number to buf
            frag_no = i;            
            sprintf(tmp, "%d", total_frag);
            strcpy(buf+ptr, tmp);
            ptr = ptr + strlen(tmp);
            buf[ptr] = ':';
            ptr = ptr + 1;
            bzero(tmp, 16);
            tmp[0]='\0';

            sprintf(tmp, "%d", frag_no);
            strcpy(buf+ptr, tmp);
            ptr = ptr + strlen(tmp);
            buf[ptr] = ':';
            ptr = ptr + 1;
            bzero(tmp, 16);
            tmp[0]='\0';

            // itoa(size, &tmp, 10);
            sprintf(tmp, "%d", size);
            strcpy(buf+ptr, tmp);
            ptr = ptr + strlen(tmp);
            buf[ptr] = ':';
            ptr = ptr + 1;
            bzero(tmp, 16);
            tmp[0]='\0';

            strcpy(buf+ptr, filename);
            ptr = ptr + strlen(filename);
            buf[ptr] = ':';
            ptr = ptr + 1;
            strcpy(buf+ptr, data);
            buf[ptr+size] = '\0';

            byte_sent = sendto(s, buf, 3, 0, (struct sockaddr*)&server_addr, server_addrlen);
            printf("[C] Packet %d of size %d sent\n", frag_no, byte_sent);

       }
       fclose(stream);



    } else {
        printf("File does not exist\n");
        exit(1);
    }

    close(s);
    exit(1);

}
