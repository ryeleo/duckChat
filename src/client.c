#include <stdio.h>
#include <errno.h>
#include <socket.h>

#include "duckchat.h"
#include "raw.h"


int main(int argc, char *argv[]){

// (1) Join/Login Process
    if (argc != 4){
        printf("Usage: %s <server> <port> <username>");
        return 1;
    }
    
    cSock = socket(,SOCK_DGRAM, UDP); 
    
// (2, 3) Main event loop
    while(true){//begin Main event loop

// (2) Read/Parse user commands

// (3) Send user command to server

    }//end Main event loop
    char buffer[32];    
    raw_mode();
    scanf("%s", buffer);
    printf("this my shit: %s\n", buffer);
    cooked_mode();  
}

int initConnect(){
}
