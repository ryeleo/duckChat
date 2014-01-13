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

// c++ includes

#include <string>
#include <iostream>
#include <map>
#include <cstring>
#include <list>

using namespace std;

int main(int argc, char **argv){
    
    int
        req_type,
        sockfd,
        ret;
        //i;
    
    char 
        *domain,
        *port,
        buf[TEXT_MAX];

    struct addrinfo
        hints,
        *serv,
        *servIter,
        clientAddrInfo;
    
    struct sockaddr
        *client_SA;
    
    string
        username,
        channel;

    request
        *message;

//initialize maps for users and channels
    list<pair<string, struct sockaddr> >::iterator userAddrIter;
    list< pair<string, struct sockaddr> > userAddrList;
    pair<string, struct sockaddr> userAddrItem;
    

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
    
    // main event loop; receive packets and act on them
    while(1){

        //listen on the udp port
        ret = recvfrom(sockfd, buf, TEXT_MAX, 0, (struct sockaddr*) &clientAddrInfo.ai_addr, &clientAddrInfo.ai_addrlen);
        if (ret == 0 || ret == -1){
            printf("[ERROR] Issue during recieve from client\n");
            continue;
        }

        // copy stuff for readability
        message = (request*) buf;
        if(message == NULL){
            printf("[ERROR] Issue during recieve from client\n");
            continue;
        }
        client_SA = clientAddrInfo.ai_addr;
        if(client_SA == NULL){
            printf("[ERROR] Issue during recieve from client\n");
            continue;
        }
        req_type = ntohl(message->req_type);
        if(req_type > 7 || req_type < 0){
            printf("[ERROR] Issue during recieve from client\n");
            continue;
        }

        switch (req_type){
    
            // add user to userMap
            case REQ_LOGIN:
                

                // setup our userMapItem with the username
                username = string(((request_login*) message)->req_username);
                userAddrItem.first = username;
                memcpy(&userAddrItem.second, (char*) client_SA, sizeof(client_SA));

                cout << "Recieved login request from" << username << endl;

                // push the userAddrItem onto the list 
                userAddrList.push_back(userAddrItem);
        
                break;

            // remove user from userMap
            case REQ_LOGOUT:
                
                printf("Received logout request\n");

                // setup our userMapItem with the username
                username = string(((request_login*) message)->req_username);
                userAddrItem.first = username;
                userAddrItem.second = *client_SA;

                // find our userAddrItem in the list
                for(userAddrIter = userAddrList.begin() ; userAddrIter != userAddrList.end() ; userAddrIter++){
                    if(userAddrItem.first == userAddrIter->first)
                        break;
                }

                // check that the user did exist in the list
                if (userAddrIter == userAddrList.end()){
                    printf("Could not log out\n");
                    break;
                }
    
                // remove the user from our user list
                userAddrList.erase(userAddrIter);
                
                break;

            case REQ_JOIN:
               /* 
                // initialize the userAddrList
                userAddrList.clear();
                
                channel = string(((request_join*)message)->req_channel); 

                // find the channel, get an iterator that points to the userAddrlist associated with it
                chanMapIt = chanMap.find(channel);
                
                // if the channel does not currently exist
                if (chanMapIt == chanMap.end()){
                
                    // put the users addr onto the list
                    userAddrList.push_back(client_STR);
                    chanMapItem.first = ((request_join*) message)->req_channel;
                    chanMapItem.second = userAddrList;
                    
                    // create the list, push on this user, and add that to the map
                    chanMap.insert(chanMapItem); 
                
                // the channel does exist, use the iterator to add this user to the list
                }else{
                    (chanMapIt->second).push_back(client_STR);
                }
                */
                break;
                
            case REQ_LEAVE:
            break ;
                //parse the leave request
            case REQ_SAY:

                break;
                //parse the say request
            case REQ_WHO:
            
                break;
                //parse who request
            case REQ_LIST:
        
                break;
                //parse list request
            default:
                //send to the user that senda bad request "Server did not understand your request"
                // do this without a helper function
                return -1;

        } // end of switch statement

    }// end main event loop
  
    // should never get to the end of the main control loop
    return 0;
}


struct sockaddr str2sockaddr(string input){
    return *((sockaddr*) input.c_str());
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
