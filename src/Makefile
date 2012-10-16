CC=gcc
PROJ_DIR=..
CFLAGS=-Wall -W -g

LOADLIBES=-lnsl #-lsocket

all: client server

client: client.c raw.c
	$(CC) client.c raw.c $(LOADLIBES) $(CFLAGS) -o $(PROJ_DIR)/bin/client

server: server.c 
	$(CC) server.c $(LOADLIBES) $(CFLAGS) -o $(PROJ_DIR)/bin/server

clean:
	rm -f client server *.o

