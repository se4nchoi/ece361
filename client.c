#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/stat.h>

#include "clientFunctions.h"

#define MAX_SIZE 2048
#define MAX_CHAR 128
#define MAX_COMMAND 5

void makeParsedCommand(char* buffer,  char (*parsedCommands)[MAX_CHAR]) {
    int i,j,ctr;

    j=0; ctr=0;
    for (i=0; i<=(strlen(buffer)); i++) {
        if (buffer[i]==' ' || buffer[i] =='\0') {
            parsedCommands[ctr][j]='\0';
            ctr++;
            j=0;
        } else {
            parsedCommands[ctr][j] = buffer[i];
            j++;
        }
    }
}

int getCommandType(char* cmd) {
    if (strcmp(cmd, "/login")==0) {
        return 0;
    } else if (strcmp(cmd, "/logout")==0) {
        return 1;
    } else if (strcmp(cmd, "/joinsession")==0) {
        return 2;
    } else if (strcmp(cmd, "/leavesession")==0) {
        return 3;
    } else if (strcmp(cmd, "/createsession")==0) {
        return 4;
    } else if (strcmp(cmd, "/list")==0) {
        return 5;
    } else if (strcmp(cmd, "/quit")==0) {
        return 6;
    }

}

int
main()
{   
    char buf[MAX_SIZE];
    // client receiving cmd from user
    printf("Client running successfully...\n");
    char userid[MAX_CHAR];
    int s;
    while (1) {
        gets(buf);
        printf("buffer: %s\n", buf);

        // split input string to check cmd/message from first word
        char parsedCommands[MAX_COMMAND][MAX_CHAR];
        makeParsedCommand(buf, parsedCommands);
        
        for (int i=0; i<MAX_COMMAND; i++) {
            printf("[%d]: %s\n", i, parsedCommands[i]);
        }

        char* firstWord = parsedCommands[0];

        if (firstWord[0]=='/') {
            // command
            switch (getCommandType(firstWord)) {
                case 0:
                    s = login(parsedCommands);
                    strcpy(userid, parsedCommands[1]);
                    break;
                case 1:
                    printf("logout command\n");
                    logout();
                    break;
                case 2:
                    printf("joinsession command\n");
                    joinsession(parsedCommands);
                    break;
                case 3:
                    printf("leavesession command\n");
                    leavesession();
                    break;
                case 4:
                    createsession(userid, s);
                    break;
                case 5:
                    printf("list command\n");
                    runList();
                    break;
                case 6:
                    printf("quit command\n");
                    runQuit();
                    break;
                default:
                    break;
            }

        } else {
            // message
            
            printf("Check if you are in a session first.\n");
        }
    }

    return 0;
}
