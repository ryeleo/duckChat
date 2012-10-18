#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>

#include <sys/socket.h>
#include <sys/types.h>

#include "duckchat.h"
#include "raw.h"


int main(int argc, char *argv[]){

    int ret;
    int sockfd; // FD for primary client socket
    char *server, *port, *uname;
    struct addrinfo hints, *serv, *servIter;  

    struct request_login packLogin;
    
// (1) Client Initialization
    
    // verfiy input params 
    if (argc != 4){
        printf("Usage: %s <server> <port> <username>", argv[0]);
        return 1;
    }
    
    // sort out input params
    server = argv[1];
    port = argv[2];
    uname = argv[3];
  
    // initialize hints for call to getaddrinfo
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC; 
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = 0;
    
    // attempt getaddrinfo on the provided server
    ret =  getaddrinfo(server, port, &hints, &serv);
    if (ret != 0){
        printf("Error occurred during getaddrinfo: errorcode(%d)\n",ret);  
        return 1;
    }
 
    // attempt to create our socket matching the addrinfo given
    for(servIter = serv ; sockfd == -1 && servIter != NULL  ; servIter = servIter->ai_next)
        sockfd = socket(servIter->ai_family, servIter->ai_socktype, servIter->ai_protocol);
    if (sockfd == -1){
        printf("Could not establish socket: %s\n", strerror(errno));
        return 1;
    }
    if(servIter == NULL){
        printf("Could not establish socket. \nIs the server online?\n");
        return 1;
    }
   
// (2) Connect to Server
    
    ret = connect(sockfd, servIter->ai_addr, servIter->ai_addrlen);
    if (ret == -1){
        printf("Could not connect to server: %s\n", strerror(errno));
        return 1;
    }
    
// (3) User Login

    // initialize user login message
    memset(&packLogin, 0, sizeof(packLogin));
    packLogin.req_type = REQ_LOGIN;
    strncpy(packLogin.req_username, uname, USERNAME_MAX);

    // send the login message 
    ret = write(sockfd, packLogin, sizeof(packLogin));
    if (ret == -1){
        printf("Error occurred at login: %s\n", strerror(errno));
        return 1;
    }
    
// (4, 5) Main event loop
    ret = raw_mode();
    if (ret != 0){
        printf("Issue entering raw mode");
        return 1;
    }
    while(1){//begin Main event loop

// (2) Read/Parse user commands

// (3) Send user command to server

    }//end Main event loop
    cooked_mode();  
}
