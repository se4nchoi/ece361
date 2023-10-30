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
#include <sys/resource.h>
#include <errno.h>
#include <math.h>


#define MAX_LINE 2048
#define PKT_SIZE 1000

enum { NS_PER_SECOND = 1000000000 };

long calculateSD(long data[]);
void sub_timespec(struct timespec t1, struct timespec t2, struct timespec *td)
{
    td->tv_nsec = t2.tv_nsec - t1.tv_nsec;
    td->tv_sec  = t2.tv_sec - t1.tv_sec;
    if (td->tv_sec > 0 && td->tv_nsec < 0)
    {
        td->tv_nsec += NS_PER_SECOND;
        td->tv_sec--;
    }
    else if (td->tv_sec < 0 && td->tv_nsec > 0)
    {
        td->tv_nsec -= NS_PER_SECOND;
        td->tv_sec++;
    }
}

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
    clock_t resp_time;
    struct timeval timeout={0,0}; 
    struct timespec start, finish, delta;

    long timeout_vals[2048];
    long total = 0;
    long stddev = 0;
    long count = 1;
    long to_to_use;

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

    if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("[C] Socket not set up\n");
        exit(1);
    }

    // /* set up socket */
    // struct timeval timeout={0,500}; 
    // if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO,(char*)&timeout,sizeof(struct timeval)) < 0) {
    //     perror("Error in setting timeout");
    //     exit(EXIT_FAILURE);
    // }

    printf("[C] Type in filename (ex. ftp <file name>):\n");
    // scanf("%s\n", buf);

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
    printf("%s\n", filename);
    if (fileExists(filename)) {

        // SECTION 2 (round trip time)
        clock_t resp_time = clock();

        // ping server with "ftp"
        // clock_gettime(CLOCK_REALTIME, &start);
        sendto(s, "ftp", 3, 0, (struct sockaddr*)&server_addr, server_addrlen);
        // wait for "yes"
        bzero(buf, MAX_LINE);
        recvfrom(s, (char *) buf, MAX_LINE, 0, (struct sockaddr*)&server_addr, &server_addrlen);
        // clock_gettime(CLOCK_REALTIME, &finish);

        resp_time = clock() - resp_time;
        double trip_time = ((double)resp_time)/CLOCKS_PER_SEC; // seconds

        printf("[C] Ack recv: %s\n", buf);
    
        strncpy(substr, buf, 3);
        if (strcmp(substr, "yes") != 0) {
            printf("[C] Server did not respond with \"yes\"\n");
            exit(1);
        } else {
            printf("[C] A file transfer can start.\n");
            timeout_vals[count] = delta.tv_sec * 1000000 + delta.tv_nsec/1000;
            total += timeout_vals[count];
            stddev = calculateSD(timeout_vals);
            to_to_use = total/count + 4 * stddev;

            timeout.tv_sec = to_to_use/1000000;
            timeout.tv_usec = to_to_use%1000000;

            if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO,(char*)&timeout,sizeof(struct timeval)) < 0) {
                        perror("Error in setting timeout");
                        exit(EXIT_FAILURE);
                    }
        }

        printf("--Section 1 complete--\n");

        sub_timespec(start, finish, &delta);
        // printf("Time passed: %d.%.9ld\n", (int)delta.tv_sec, delta.tv_nsec);

        printf("[] Round trip time: %f s\n", trip_time);
        printf("--Section 2 complete--\n");

        
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
        
        // sending each fragment
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
            
            // repeat sending this fragment
            while (1) {
                clock_gettime(CLOCK_REALTIME, &start);
                bytes_sent = sendto(s, buf, ptr, 0, (struct sockaddr*)&server_addr, server_addrlen);
                printf("[C] Packet %d of size %d sent\n", frag_no, bytes_sent);
                int recvlen = recvfrom(s, (char *) tmp, MAX_LINE, 0, (struct sockaddr*)&server_addr, &server_addrlen);
                clock_gettime(CLOCK_REALTIME, &finish);

                if (recvlen >=0 ) {
                    memcpy(tmp2, "ACK ", 4);
                    sprintf(tmp2+4, "%d", frag_no);

                    if (strcmp(tmp, tmp2)==0){
                        printf("[C] Packet %d acknowledged\n", frag_no);
                        bzero(data, 1000);
                        break;
                    } 
                    
                    sub_timespec(start, finish, &delta);
                    timeout_vals[count] = delta.tv_sec * 1000000 + delta.tv_nsec/1000;
                    total += timeout_vals[count];
                    stddev = calculateSD(timeout_vals);
                    to_to_use = total/count + 4 * stddev;

                    timeout.tv_sec = to_to_use/1000000;
                    timeout.tv_usec = to_to_use%1000000;

                    /* set up socket */
                    if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO,(char*)&timeout,sizeof(struct timeval)) < 0) {
                        perror("Error in setting timeout");
                        exit(EXIT_FAILURE);
                    }
                } else {
                    printf("[C] Error no: %d\n", errno);
                    printf("[C] Packet %d not acknowledged, sending again\n", frag_no);

                    continue;
                }

            }
       }
        printf("--Section 3 complete--\n");



       fclose(stream);



    } else {
        printf("File does not exist\n");
        exit(1);
    }

    close(s);
    exit(1);

}

long calculateSD(long data[]) {
    long sum = 0, mean, SD = 0;
    int i;
    for (i = 0; i < 10; ++i) {
        sum += data[i];
    }
    mean = sum / 10;
    for (i = 0; i < 10; ++i) {
        SD += pow(data[i] - mean, 2);
    }
    return sqrt(SD / 10);
}