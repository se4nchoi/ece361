#if !defined(_clientFunctions_h_)
#define _clientFunctions_h_

#define MAX_CHAR 128

int login(char (*parsedCommands)[MAX_CHAR]);

void logout();

void joinsession(char (*parsedCommands)[MAX_CHAR]);

void leavesession();

void createsession(char (*parsedCommands)[MAX_CHAR]);

void runList();

void runQuit();

#endif