#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/stat.h>
#include <errno.h>
#include <arpa/inet.h>

#include "serverFunctions.h"
#include "users.h"
#include "message.h"

#define SA struct sockaddr 

int
main(int argc, char * argv[])
{
    struct sockaddr_in client_addr, server_addr, cli;
    int master_socket, client_socket[MAX_CLIENTS], new_socket, valread, activity, sd, max_sd;
    socklen_t server_addrlen, client_addrlen;

    //set of socket descriptors
    fd_set readfds;  

    char* port;
    int port_int;

    char buffer[2048];

    // reading input argument
    if (argc==2) {
        port = argv[1];
        port_int = atoi(port);
    }
    else {
        fprintf(stderr, "need 2 arguments: server <TCP listen port> \n");
        exit(1);
    }

    struct User* clients = malloc(sizeof(struct User) * MAX_CLIENTS);
    makeUserList("clients.txt", clients);

    //initialise all client_socket[] to 0 so not checked
    for (int i = 0; i < MAX_CLIENTS; i++) 
    {
        client_socket[i] = 0;
    }

    //create a master socket, SOCK_STREAM:TCP
    if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0) 
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }


    /* build address data structure */
    bzero((char *)&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port_int);
    server_addrlen = sizeof(server_addr);


    //bind the socket to localhost port 8888
    if (bind(master_socket, (struct sockaddr *)&server_addr, server_addrlen)<0) 
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if ((listen(master_socket, 5)) != 0) { 
        printf("Listen failed...\n"); 
        exit(0); 
    } 
    else
        printf("Server listening..\n"); 
    

    while(1) 
    {
        //clear the socket set
        FD_ZERO(&readfds);
        //add master socket to set
        FD_SET(master_socket, &readfds);
        max_sd = master_socket;
		
        //add child sockets to set
        for (int i = 0 ; i < MAX_CLIENTS ; i++) 
        {
            //socket descriptor
			sd = client_socket[i];
            
			//if valid socket descriptor then add to read list
			if(sd > 0)
				FD_SET( sd , &readfds);
            
            //highest file descriptor number, need it for the select function
            if(sd > max_sd)
				max_sd = sd;
        }
        //wait for an activity on one of the sockets , timeout is NULL , so wait indefinitely
        activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL);
   
        if ((activity < 0) && (errno!=EINTR)) 
        {
            printf("select error");
        }
         
        //If something happened on the master socket , then its an incoming connection
        if (FD_ISSET(master_socket, &readfds)) 
        {
            if ((new_socket = accept(master_socket, (struct sockaddr *)&client_addr, (socklen_t*)&client_addrlen))<0)
            {
                perror("accept");
                exit(EXIT_FAILURE);
            }
            
            //inform user of socket number - used in send and receive commands
            printf("New connection , socket fd is %d , ip is : %s , port : %d \n" , new_socket , inet_ntoa(client_addr.sin_addr) , ntohs(client_addr.sin_port));

            recv(new_socket, buffer, sizeof(buffer), 0);
            struct message msg;
            stringToMessage(&msg, buffer);
            bzero(buffer, 2048);
            for (int i=0; i<MAX_CLIENTS; i++) {
                if (strlen(clients[i].userid)==0){
                    printf("Invalid User Id\n");
                    break;
                }
                if (strcmp(msg.source, clients[i].userid)==0){
                    printf("Valid User Id, checking password\n");
                    if (strcmp(msg.data, clients[i].password)==0){
                        printf("Password correct\n");
                        //add new socket to array of sockets
                        for (int i = 0; i < MAX_CLIENTS; i++) 
                        {
                            //if position is empty
                            if( client_socket[i] == 0 )
                            {
                                client_socket[i] = new_socket;
                                printf("Adding to list of sockets as %d\n" , i);
                                
                                break;
                            }
                        }
                        send(new_socket, "ACK", 3, 0);

                    } else {
                        printf("Password incorrect\n");
                        send(new_socket, "NACK", 4, 0);
                    }
                    break;
                }
            }
             
        }
         
        //else its some IO operation on some other socket :)
        for (int i = 0; i < MAX_CLIENTS; i++) 
        {
            sd = client_socket[i];
             
            if (FD_ISSET( sd , &readfds)) 
            {
                //Check if it was for closing , and also read the incoming message
                if ((valread = read( sd , buffer, 1024)) == 0)
                {
                    //Somebody disconnected , get his details and print
                    getpeername(sd , (struct sockaddr*)&client_addr , (socklen_t*)&client_addrlen);
                    printf("Host disconnected , ip %s , port %d \n" , inet_ntoa(client_addr.sin_addr) , ntohs(client_addr.sin_port));
                     
                    //Close the socket and mark as 0 in list for reuse
                    close( sd );
                    client_socket[i] = 0;
                }
                 
                //Echo back the message that came in
                else
                {
                    //set the string terminating NULL byte on the end of the data read
                    buffer[valread] = '\0';
                    send(sd , buffer , strlen(buffer) , 0 );
                }
            }
        }
    }


    return 0;
}