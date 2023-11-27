#define MAX_NAME 128
#define MAX_DATA 128

#define LOGIN 0
#define LOGOUT 1
#define JOIN 2
#define LEAVE 3
#define CREATE 4
#define LIST 5
#define QUIT 6

struct message {
    unsigned int type;
    unsigned int size;
    unsigned char source[MAX_NAME];
    unsigned char data[MAX_DATA];
};

void makeLoginMessage(struct message* msg, char* userid, char* password) {
    msg->type = LOGIN;
    strcpy(msg->source, userid);
    strcpy(msg->data, password);
    msg->size = strlen(password);
}

void messageToString(char* buf, struct message* msg) {
    sprintf(buf, "%d", msg->type);
    strcat(buf, ":");

    char size[3];
    sprintf(size, "%d", msg->size);
    strcat(buf, size);
    strcat(buf, ":");
    strcat(buf, msg->source);
    strcat(buf, ":");
    strcat(buf, msg->data);
}

void stringToMessage(struct message* msg, char* buf) {
    msg->type = atoi(strtok(buf, ":"));
    msg->size = atoi(strtok(NULL, ":"));
    strcpy(msg->source, strtok(NULL, ":"));
    strcpy(msg->data, strtok(NULL, ":"));
}