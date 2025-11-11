all: server client

server: server.c
	gcc -o server server.c -lsocket -lnsl

client: client.c
	gcc -o client client.c -lsocket -lnsl