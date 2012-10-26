#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>

#include "duckchat.h"
#include "raw.h"

#define STDIN 1
#define STDOUT 0
#define CHAR_AMOUNT 1
#define BUFF_SIZE 32

int main(int argc, char *argv[]){

    int ret,            // general return variable 
        i,              // general counter
        sockfd;         // file descriptor for client socket
    
    char 
        *server,        // domain name of the server the client will connect to
        *port,          // port number on server-end
        *uname,         // username the user requests

        inputBuf[32],   // buffer input from command line
        inputChar,      // used along with buffer for user input 
    
        token1[32],     // used in parsing client requests
        token2[32],     // used in parsing client requests 

        activeChannel[32],  // name of channel user is on 
        **channels;     // list of channels user can intera
   
     struct addrinfo 
        hints,          
        *serv, 
        *servIter;  

    struct request_login 
        packLogin;
    
    struct 
    
     
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
    hints.ai_family = AF_INET; 
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = 0;
    
    // attempt getaddrinfo on the provided server
    ret =  getaddrinfo(server, port, &hints, &serv);
    if (ret != 0){
        printf("Error occurred during getaddrinfo: errorcode(%d)\n",ret);  
        return 1;
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
     
    ret = write(sockfd, (char*) &packLogin, sizeof(packLogin));
    if (ret == -1){
        printf("Error occurred at login: %s\n", strerror(errno));
        return 1;
    }
    
// (4, 5, 6) Main event loop
    ret = raw_mode();
    if (ret != 0){
        printf("Issue entering raw mode");
        return 1;
    }
    while(1){//begin Main event loop

// (4) Get user input
        memset(inputBuf, '\0', BUFF_SIZE);
        i = 0;
        inputChar = '\0';
        do{
            ret = read(STDIN, &inputChar, CHAR_AMOUNT);    
            if (ret != CHAR_AMOUNT){
                printf("Error occurred during user input: %s\n", strerror(errno));
                return 1;
            }
            if (i >= 31)
                continue;
            else{
                inputBuf[i] = inputChar;
                write(STDOUT, &(inputBuf[i]), CHAR_AMOUNT);   
            }
            i++;
        }while(inputChar != '\n');

// (5) Parse user input and send to server
        token1 = strtok(inputBuffer, ' ');
        token2 = strtok(NULL, ' ');
        //TODO: Impliment all commands
        if(inputBuf[0] == '/' ){ // send command
            if(strcmp("/exit", token1) == 0){
            }else if(strcmp("/join", token1)){ //check token 2
            }else if(strcmp("/leave", token1)){ //check token 2
            }else if(strcmp("/list", token1)){
            }else if(strcmp("/who", token1)){ // check token 2
            }else if(strcmp("/switch", token1)){ // check token 2
            }else{ //wrong
            }
        }else{ // say
            //XXX: Impliment say
        }
// (6) Send the request to the server

    }//end Main event loop
    cooked_mode();  
}

char *parse(char *input){
}
