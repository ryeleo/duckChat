CC=gcc
CPP=g++
PROJ_DIR=.
CFLAGS= -Wall -W -g

LOADLIBES= -lnsl -lresolv #-lsocket 

all: client server

client: client.c raw.c
	$(CC) client.c raw.c $(LOADLIBES) $(CFLAGS) -o $(PROJ_DIR)/client

server: server.cpp
	$(CPP) server.cpp $(LOADLIBES) $(CFLAGS) -o $(PROJ_DIR)/server

clean:
	rm -f client server *.o
