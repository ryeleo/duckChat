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

        inputBuf[SAY_MAX],      // buffer input from command line
        sayBuf[SAY_MAX],        // buffer to send message to server 
        inputChar,              // used along with buffer for user input
        consoleBuf[256],

        delim,          // delim char for tokens    
        *token1 = malloc(SAY_MAX),     // used in parsing client requests
        *token2 = malloc(SAY_MAX),     // used in parsing client requests 

        activeChannel[CHANNEL_MAX] = "Common",  // name of channel user is on 
        **channels;     // list of channels user can intera
   
     struct addrinfo 
        hints,          
        *serv, 
        *servIter;  

    struct request_login 
        packLogin;
    
    struct request_logout
        packLogout;

    struct request_join
        packJoin;

    struct request_leave
        packLeave;

    struct request_say
        packSay;

    struct request_list
        packList;
    
    struct request_who
        packWho;

    struct request_keep_alive
        packKeepAlive;
     
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
    packLogin.req_type = htonl(REQ_LOGIN);
    strncpy(packLogin.req_username, uname, USERNAME_MAX);

    // send the login message
    ret = write(sockfd, (char*) &packLogin, sizeof(packLogin));
    if (ret == -1){
        printf("Error occurred at login: %s\n", strerror(errno));
        return 1;
    }

    // initialize user join message
    memset(&packJoin, 0, sizeof(packJoin));
    packJoin.req_type = htonl(REQ_JOIN);
    strncpy(packJoin.req_channel, activeChannel, strlen(activeChannel));

    // send the join message
    ret = write(sockfd, (char*) &packJoin, sizeof(packJoin));
    if (ret == -1){
        printf("Error occurred during join: %s\n", strerror(errno));
        return 1; 
    }
    
// (4, 5, 6) Main event loop
    ret = raw_mode();
    if (ret != 0){
        printf("Issue entering raw mode");
        bad_exit();
    }
    while(1){//begin Main event loop

// (4) Get user input
        memset(inputBuf, '\0', SAY_MAX);
        memset(sayBuf, '\0', SAY_MAX);
        i = 0;
        inputChar = '\0';
        do{
            ret = read(STDIN, &inputChar, CHAR_AMOUNT);    
            if (ret != CHAR_AMOUNT){
                printf("Error occurred during user input: %s\n", strerror(errno));
                bad_exit();
            }
            if (i >= SAY_MAX - 1){ // if have read full amount, wait for \n
                continue;
            }else if(inputChar == '\n'){
                break;
            }else{
                inputBuf[i] = inputChar;
                write(STDOUT, &(inputBuf[i]), CHAR_AMOUNT);   
            }
            i++;
        }while(inputChar != '\n');
        printf("\n"); // newlines absorbed in while loop above   
        strncpy(sayBuf, inputBuf, strlen(inputBuf));
    
// (5, 6) Parse user input and send to server
        delim = ' ';
        token1 = strtok(inputBuf, &delim);
        token2 = strtok(NULL, &delim);
        
        if(sayBuf[0] == '/' ){ // send command
            if(strcmp("/exit", token1) == 0){

                // initialize user logout message
                memset(&packLogout, 0, sizeof(packLogout));
                packLogout.req_type = htonl(REQ_LOGOUT);

                // send the logout message
                ret = write(sockfd, (char*) &packLogout, sizeof(packLogout));
                if (ret == -1){
                    printf("Error occurred during exit: %s\n", strerror(errno));
                    bad_exit();
                }
                
                clean_exit(); // user logout
                
            }else if(strcmp("/join", token1) == 0){ //check token 2
            
                // initialize user join message
                memset(&packJoin, 0, sizeof(packJoin));
                packJoin.req_type = htonl(REQ_JOIN);
                strncpy(packJoin.req_channel, token2, strlen(token2));

                // send the join message
                ret = write(sockfd, (char*) &packJoin, sizeof(packJoin));
                if (ret == -1){
                    printf("Error occurred during join: %s\n", strerror(errno));
                    bad_exit();
                }

            }else if(strcmp("/leave", token1) == 0){ //check token 2
            
                // initialize user leave message
                memset(&packLeave, 0, sizeof(packLeave));
                packLeave.req_type = htonl(REQ_LEAVE);
                strncpy(packLeave.req_channel, token2, strlen(token2));

                // send the leave message
                ret = write(sockfd, (char*) &packLeave, sizeof(packLeave));
                if (ret == -1){
                    printf("Error occurred during leave: %s\n", strerror(errno));
                    bad_exit();
                }
     
            }else if(strcmp("/list", token1) == 0){
            
                // initialize user list message
                memset(&packList, 0, sizeof(packList));
                packList.req_type = htonl(REQ_LIST);

                // send the list message
                ret = write(sockfd, (char*) &packList, sizeof(packList));
                if (ret == -1){
                    printf("Error occurred during list: %s\n", strerror(errno));
                    bad_exit();
                }
            
            }else if(strcmp("/who", token1) == 0){ // check token 2

                // initialize user who message
                memset(&packWho, 0, sizeof(packWho));
                packWho.req_type = htonl(REQ_WHO);
                strncpy(packWho.req_channel, token2, strlen(token2));

                // send the who message
                ret = write(sockfd, (char*) &packWho, sizeof(packWho));
                if (ret == -1){
                    printf("Error occurred during who: %s\n", strerror(errno));
                    bad_exit();
                }
    
            }else if(strcmp("/switch", token1) == 0){ // check token 2
  
                strncpy(activeChannel, token2, CHANNEL_MAX); 

            }else{ //wrong
                printf("*Unkown command\n"); 
            }
        }else{ // say
            
            // initialize user message
            memset(&packSay, 0, sizeof(packSay));
            packSay.req_type = htonl(REQ_SAY);
            strncpy(packSay.req_channel, activeChannel, strlen(activeChannel));
            strncpy(packSay.req_text, sayBuf, strlen(sayBuf));

            // send the message
            ret = write(sockfd, (char*) &packSay, sizeof(packSay));
            if (ret == -1){
                printf("Error occurred during say: %s\n", strerror(errno));
                bad_exit();
            }

        }

        //TODO: HC
        ret = read(sockfd, consoleBuf, 256); 
        if (ret == -1){
            printf("Error occurred during read from server: %s\n", strerror(errno));
            bad_exit();
        }
        printf("%s\n", consoleBuf);

    }//end Main event loop
}

void clean_exit(){
    cooked_mode();
    exit(0);
}

void bad_exit(){
    cooked_mode();
    exit(1);
}
