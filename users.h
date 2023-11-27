#include <stdio.h>
#include <stdlib.h>

#define MAX_CHAR 128
#define MAX_CLIENTS 30


struct User {
    char userid[MAX_CHAR];
    char password[MAX_CHAR];
    int socket;
};

void makeUserList(char *filename, struct User* clients) {
    FILE * fp;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;

    fp = fopen(filename, "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);
    
    int i =0;
    while ((read = getline(&line, &len, fp)) != -1) {
        struct User new_client;
        line[read-1] = '\0';
        strcpy(new_client.userid, line);
        read = getline(&line, &len, fp);
        line[read-1] = '\0';
        strcpy(new_client.password, line);
        clients[i] = new_client;

        i += 1;
    }

    fclose(fp);
    if (line)
        free(line);
}