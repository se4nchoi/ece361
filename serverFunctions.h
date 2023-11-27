#define MAX_CHAR 128

int handle_login(struct message *msg, struct User* clients);

void handle_logout();

void handle_joinsession(char (*parsedCommands)[MAX_CHAR]);

void handle_leavesession();

void handle_createsession(char (*parsedCommands)[MAX_CHAR]);

void handle_runList();

void handle_runQuit();
