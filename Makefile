CC=gcc

make: client.c server.c
	$(CC) -o client client.c clientFunctions.c
	$(CC) -o server server.c serverFunctions.c