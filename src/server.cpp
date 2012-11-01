#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>

#include <arpa/inet.h>

#include <sys/socket.h>
#include <sys/types.h>

#include "client.h"
#include "duckchat.h"
#include "raw.h"

#include <iostream>
#include <map>
#include <cstring>

using namespace std;

int main(int argc, char **argv){
    
    int
        req_type,
        sockfd,
        ret,
        i;
    
    char 
        *domain,
        *port;

    struct addrinfo
        hints,
        *serv,
        *servIter;

//initialize maps for users and channels
    //TODO

// (1) Check usage

    // make sure we have a domain and port
    if (argc != 3){
        printf("Usage: %s <domain/address> <port>\n", argv[0]);
        return -1;
    }
    
    // assign our local char* to point to the cmdline input for readability
    domain = argv[1];
    port = argv[2];

    // make sure domain path is not too long
    #ifdef UNIX_PATH_MAX
    if ( (strlen(domain) + 1) > UNIX_PATH_MAX){
        printf("Exceeded unix-path-max for domain. \n "
                "Please enter a different server name. \n");
        return -1;
    }
    #endif
 
// (2) Bind socket to later listen on

    // initialize hints for call to getaddrinfo
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC; // INET/INET6 capable 
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;
    
    // attempt getaddrinfo on us (the server)
    ret =  getaddrinfo(domain, port, &hints, &serv);
    if (ret != 0){
        printf("[ERROR: %d] failed call to getaddrinfo: %s\n", __LINE__, strerror(errno));   
        return -1;
    }

    // attempt to create our socket matching the addrinfo given
    sockfd = -1;
    servIter = serv;    
    while(sockfd == -1 && servIter != NULL ){
        sockfd = socket(servIter->ai_family, servIter->ai_socktype, servIter->ai_protocol);
        if (sockfd == -1)
            servIter = servIter->ai_next;
    }
    if (sockfd == -1){
        printf("[ERROR: %d] could not establish socket: %s\n", __LINE__, strerror(errno));   
        return -1;
    }
    if(servIter == NULL){
        printf("[ERROR: %d] could not establish socket\n", __LINE__);   
        return -1;
    }
    
    // set serve to be the information on the sockfd that succeeded
    serv = servIter;

    // bind our socket to the port requested
    ret = bind(sockfd, serv->ai_addr, serv->ai_addrlen);
    if (ret == -1){
        printf("[ERROR: %d] could not establish socket: %s\n", __LINE__, strerror(errno));   
        return -1;
    }
        
// (3) Main server event loop
/*    // main event loop, take packets and act on them
    while(1){

        //listen on the udp port
        recvfrom();
        switch (req_type){
            case REQ_LOGIN:
                //parse the login request
                //
            case REQ_LOGOUT:
                //parse the logout request
                //remove user from all channels
            case REQ_JOIN:
                //parse the join request
            case REQ_LEAVE:
                //parse the leave request
            case REQ_SAY:
                //parse the say request
            case REQ_WHO:
                //parse who request
            case REQ_LIST:
                //parse list request
            default:
                //send to the user that senda bad request "Server did not understand your request"
                // do this without a helper function
                return -1;
        }

    }// end main event loop
  */  
    // should never get to the end of the main control loop
    return 0;
}

//parseLogin must do two things:
//add the user to the users map, storing their ip address
//have the user join channel Common
int parseLogin(){
    
    return 0;
}

//parseLogout must do two things:
//remove the user from all channels s/he is still in
//remove the user from users map
int parseLogout(){

    return 0;
}

//parseJoin needs to add the user to the map of the requested channel
int parseJoin(){

    return 0;
}

//parseLeave needs to remove the user from the requested channel
int parseLeave(){

    return 0;
}

//parseSay needs to do the following:
//find out which channel to say at
//sendto() the message to all users in the given channel
int parseSay(){

    return 0;
}

//parseWho needs to construct and send the SAY_WHO datagram containing a list of users in a channel
int parseWho(){
  
    return 0;
}

//parseList needs to construct the TXT_List datagram containing a formatted list of all channels
int parseList(){

    return 0;
}
