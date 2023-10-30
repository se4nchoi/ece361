#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/stat.h>



#define MAX_PENDING 5
#define MAX_LINE 2048

struct packet {
    unsigned int total_frag;
    unsigned int frag_no;
    unsigned int size;
    char* filename;
    char filedata[1000];
};

void getPacketFromString(struct packet* pkt, int nBytes, char* buf);


int
main(int argc, char * argv[])
{
    struct sockaddr_in client_addr, server_addr;
    socklen_t server_addrlen, client_addrlen;
    char buf[MAX_LINE];
    char tmp[16];
    int buf_len;
    int s, new_s;
    int nBytes;

    char *host, *port;
    int port_int;
    if (argc==2) {
        port = argv[1];
        port_int = atoi(port);
    }
    else {
        fprintf(stderr, "need 2 arguments: server <UDP listen port> \n");
        exit(1);
    }

    /* build address data structure */
    bzero((char *)&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port_int);
    server_addrlen = sizeof(server_addr);


    if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("[S] simplex-talk: socket");
        exit(1);
    }

    if ((bind(s, (struct sockaddr *)&server_addr, sizeof(server_addr))) < 0) {
        perror("[S] simplex-talk: bind");
        exit(1);
    }

    printf("[S] Done with binding");

    /* wait for connection, then receive and print text */
    while(1) {
        printf("\n[S] Waiting for messages ...\n");
        nBytes = recvfrom(s, (char *) buf, MAX_LINE, 0, (struct sockaddr *)&client_addr, &client_addrlen);

        if (strcmp(buf, "ftp")==0) {
            printf("[S] ftp received, sending acknowledgement\n");
            sendto(s, "yes", 3, 0, (struct sockaddr*)&client_addr, client_addrlen);
            printf("[S] acknowledgement sent\n");
        } else {
            printf("[S] Unknown message: %s\n", buf);
        }

        FILE *fp;
        int total_frag;
        struct packet* pkt;
        pkt = (struct packet*) malloc(sizeof(struct packet));

        // receiving and acknowledging the first packet
        while (1) {
            nBytes = recvfrom(s, buf, MAX_LINE, 0, (struct sockaddr *)&client_addr, &client_addrlen);
            printf("[S] %d bytes received\n", nBytes);
            getPacketFromString(pkt, nBytes, buf);

            if (pkt->frag_no!=0){
                printf("[S] Packet %d received out of sequence, request for retry\n", pkt->frag_no);
                sendto(s, "NACK", 4, 0, (struct sockaddr*)&client_addr, client_addrlen);
            } else {
                memcpy(tmp, "ACK ", 4);
                sprintf(tmp+4, "%d", pkt->frag_no);
                sendto(s, tmp, sizeof(tmp), 0, (struct sockaddr*)&client_addr, client_addrlen);
                printf("[S] Packet 0 acknowledgement sent\n");
                total_frag = pkt->total_frag;

                char new_filename[16] = "copy-";
                strcat(new_filename, pkt->filename);

                fp = fopen( new_filename , "wb" );

                fwrite(pkt->filedata, sizeof(char) , pkt->size , fp );

                break;
            }
        }

        // receiving the remaining packets
        // change here to total_frag for correct implementation
        for (int i=1; i<total_frag; i++) {
            while (1) {
                nBytes = recvfrom(s, buf, MAX_LINE, 0, (struct sockaddr *)&client_addr, &client_addrlen);
                printf("[S] %d bytes received\n", nBytes);
                getPacketFromString(pkt, nBytes, buf);
                if (pkt->frag_no!=i){
                    printf("[S] Packet %d received out of sequence, request for retry\n", pkt->frag_no);
                    sendto(s, "NACK", 4, 0, (struct sockaddr*)&client_addr, client_addrlen);
                } else {
                    memcpy(tmp, "ACK ", 4);
                    sprintf(tmp+4, "%d", pkt->frag_no);
                    sendto(s, tmp, sizeof(tmp), 0, (struct sockaddr*)&client_addr, client_addrlen);
                    printf("[S] Packet %d acknowledgement sent\n", pkt->frag_no);

                    // printf("%s\n", pkt->filedata);
                    fwrite(pkt->filedata, sizeof(char) , pkt->size , fp );
                    break;
                }
            }
        }

        fclose(fp);

        close(s);
        exit(1);
    }
}

void getPacketFromString(struct packet* pkt, int nBytes, char* buf){
    char* data;    
    pkt->total_frag = atoi(strtok(buf, ":"));
    pkt->frag_no = atoi(strtok(NULL, ":"));
    pkt->size = atoi(strtok(NULL, ":"));
    pkt->filename = strtok(NULL, ":");

    data = pkt->filename + sizeof(pkt->filename) + 1;
    memcpy(pkt->filedata, data, pkt->size);
}
