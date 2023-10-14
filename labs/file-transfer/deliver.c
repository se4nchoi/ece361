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
    FILE *fp = fopen(filename, "rb");
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
        stream = fopen(filename, "rb");
        off_t filesize;
        struct stat st;
        stat(filename, &st);
        filesize = st.st_size;
        printf("[C] Filename: %s, Filesize: %lld\n", filename, filesize);

        int total_frag = 1+(filesize-1)/PKT_SIZE;
        char data[1000];
        char tmp[16];
        char tmp2[16];
        unsigned int frag_no;
        unsigned int size;
        unsigned int ptr;

        int bytes_sent;
        int timeout;
        
       for (unsigned int i=0; i<total_frag; i++) {
            bzero(buf, MAX_LINE);
            bzero(tmp, 16);
            ptr = 0;

            size = fread(data, sizeof(char), PKT_SIZE, stream); 

            frag_no = i;      

            // copy total number of fragments     
            sprintf(tmp, "%d", total_frag);
            memcpy(buf+ptr, tmp, strlen(tmp));
            ptr = ptr + strlen(tmp);
            buf[ptr] = ':';
            ptr = ptr + 1;
            bzero(tmp, 16);
            tmp[0]='\0';

            // copy fragment number
            sprintf(tmp, "%d", frag_no);
            memcpy(buf+ptr, tmp, strlen(tmp));
            ptr = ptr + strlen(tmp);
            buf[ptr] = ':';
            ptr = ptr + 1;
            bzero(tmp, 16);
            tmp[0]='\0';

            // copy size of data
            sprintf(tmp, "%d", size);
            memcpy(buf+ptr, tmp, strlen(tmp));
            ptr = ptr + strlen(tmp);
            buf[ptr] = ':';
            ptr = ptr + 1;
            bzero(tmp, 16);
            tmp[0]='\0';

            // copy filename
            memcpy(buf+ptr, filename, strlen(filename));
            ptr = ptr + strlen(filename);
            buf[ptr] = ':';
            ptr = ptr + 1;
            memcpy(buf+ptr, data, size);
            ptr = ptr + size;

            // printf("%s", buf);
            timeout = 0;
            while (1) {
                bytes_sent = sendto(s, buf, ptr, 0, (struct sockaddr*)&server_addr, server_addrlen);
                printf("[C] Packet %d of size %d sent\n", frag_no, bytes_sent);
                recvfrom(s, (char *) tmp, MAX_LINE, 0, (struct sockaddr*)&server_addr, server_addrlen);

                memcpy(tmp2, "ACK ", 4);
                sprintf(tmp2+4, "%d", frag_no);


                if (strcmp(tmp, tmp2)==0){
                    printf("[C] Packet %d acknowledged\n", frag_no);
                    bzero(data, 1000);
                    break;
                } else {
                    printf("[C] Packet not acknowledged, sending again\n");
                    timeout = timeout + 1;
                    if (timeout > 16) {
                        printf("[C] Timeout: tried 16 times\n");
                        fclose(stream);
                        exit(1);
                    }
                }
            }


       }


       fclose(stream);



    } else {
        printf("File does not exist\n");
        exit(1);
    }

    close(s);
    exit(1);

}
