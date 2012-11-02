CC=gcc
CPP=g++
PROJ_DIR=..
CFLAGS= -Wall -W -g

LOADLIBES= -lnsl -lsocket -lresolv

all: client server

client: client.c raw.c
	$(CC) client.c raw.c $(LOADLIBES) $(CFLAGS) -o $(PROJ_DIR)/bin/client

server: server.cpp
	$(CPP) server.cpp $(LOADLIBES) $(CFLAGS) -o $(PROJ_DIR)/bin/server

clean:
	rm -f client server *.o

