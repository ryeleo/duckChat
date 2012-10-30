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
        sockfd,         // file descriptor for client socket
        activeChannel_index;    // this is the index that points to the current active channel in **channels 
    
    char 
        *server,        // domain name of the server the client will connect to
        *port,          // port number on server-end
        *uname,         // username the user requests

        inputBuf[SAY_MAX],      // buffer input from command line
        sayBuf[SAY_MAX],        // buffer to send message to server 
        inputChar,              // used along with buffer for user input
        consoleBuf[256],

        //TODO: check return of malloc
        delim,                          // delim char for tokens    
        *token1 = malloc(SAY_MAX),      // used in parsing client requests
        *token2 = malloc(SAY_MAX),      // used in parsing client requests 

        activeChannel[CHANNEL_MAX] = "Common";  // name of channel user is on 
        //*channels[] = ;     // list of channels user can intera
   
    struct addrinfo 
        hints,          // used to define what type of socket the client needs
        *serv,          // used to hold socket-descr that the server has offered us to use
        *servIter;      // used to iterate through the socket-descr the server has offered us to use

    request *message = malloc(MESSAGE_MAX); // this is out message we will send to server
    if (message == NULL){
        printf("Error occurred durring malloc");
        bad_exit();
    }
    
    fd_set 
        read_fdset;

// (1) Client Initialization
    
    // verfiy input params 
    if (argc != 4){
        printf("Usage: %s <server> <port> <username>", argv[0]);
        bad_exit();
    }
    
    // name input params
    server = argv[1];
    port = argv[2];
    uname = argv[3];

    // make sure domain path is not too long
    #ifdef UNIX_PATH_MAX
    if ( (strlen(server) + 1) > UNIX_PATH_MAX){
        printf("Exceeded unix-path-max for domain. \n "
                "Please enter a different server name. \n");
        bad_exit();
    }
    #endif 

    // initialize hints for call to getaddrinfo
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC; // INET/INET6 capable 
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = 0;
    
    // attempt getaddrinfo on the provided server
    ret =  getaddrinfo(server, port, &hints, &serv);
    if (ret != 0){
        printf("Error occurred during getaddrinfo: errorcode(%d)\n",ret);  
        bad_exit();
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
        bad_exit();
    }
    if(servIter == NULL){
        printf("Could not establish socket. \nIs the server online?\n");
        bad_exit();
    }
    serv = servIter;
// (2) Connect to Server
    
    // make a UPD connect request
    ret = connect(sockfd, serv->ai_addr, serv->ai_addrlen);
    if (ret == -1){
        printf("Could not connect to server: %s\n", strerror(errno));
        bad_exit();
    }
    
// (3) User Login and Channel join

    // pack login request
    ret = packRequest(message, REQ_LOGIN, NULL, NULL, uname);
    if (ret == -1){
        printf("Error packing request\n");
        bad_exit();
    }
    
    // send login request
    ret = sendRequest(sockfd, message);
    if (ret == -1){
        printf("Error sending request\n");
        bad_exit();
    }

    // pack join request
    ret = packRequest(message, REQ_JOIN, activeChannel, NULL, NULL);
    if (ret == -1){
        printf("Error packing request\n");
        bad_exit();
    }
    
    // send join request
    ret = sendRequest(sockfd, message);
    if (ret == -1){
        printf("Error sending request\n");
        bad_exit();
    }

    // initialize our channels
    activeChannel_index = 0;
   //channels[activeChannel_index] = malloc(strnlen(activeChannel, CHANNEL_MAX));

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


        // main loop to collect user input and server messages
	while(1){
	   
           
            // setup for call to select
            FD_ZERO(&read_fdset);
            FD_SET(STDIN, &read_fdset);
            FD_SET(sockfd, &read_fdset);
    
            ret = select(sockfd+1, &read_fdset, NULL, NULL, NULL);
	    // read from server
            if( i == 0 && FD_ISSET(sockfd, &read_fdset)){
	        ret = recvfrom(sockfd, consoleBuf, 256, 0, (serv->ai_addr), &(serv->ai_addrlen)); 
                if (ret == -1){
                    printf("Error occurred during read from server: %s\n", strerror(errno));
                    bad_exit();
	        }
           	(consolBuf, consolBuf); 
                printf("%s\n", consoleBuf);
	
            // read from stdin
            }else if(FD_ISSET(STDIN, &read_fdset)){
    
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
                    i++;
                }

	    // default case	
            }else{
                if (ret == -1){
                    perror("Error during select");
                    bad_exit();
                }
	    }
	}
        
        printf("\n"); // newlines absorbed in while loop above   
        strncpy(sayBuf, inputBuf, strnlen(inputBuf, SAY_MAX));
    
// (5) Parse user input

        delim = ' ';
        token1 = strtok(inputBuf, &delim);
        token2 = strtok(NULL, &delim);
        
        if(sayBuf[0] == '/' ){ // implies command
            
            if(strcmp("/exit", token1) == 0){

                // pack the message for logout
                ret = packRequest(message, REQ_LOGOUT, NULL, NULL, NULL);
                if (ret == -1){
                    printf("Error from packRequest\n");
                    bad_exit();
                }

                // halt the client successfully
                clean_exit();
                
            }else if(strcmp("/join", token1) == 0){ 
    
                // check the channel parameter
                if (token2 == NULL)            
                    goto UNKOWN_COMMAND;

                // pack the message for join
                ret = packRequest(message, REQ_JOIN, token2, NULL, NULL);
                if (ret == -1){
                    printf("Error from packRequest\n");
                    bad_exit();
                }
    
                //channel[]

            }else if(strcmp("/leave", token1) == 0){ //check token 2

                // check the channel parameter
                if (token2 == NULL)            
                    goto UNKOWN_COMMAND;
    
                // pack the message for leave
                ret = packRequest(message, REQ_LEAVE, token2, NULL, NULL);
                if (ret == -1){
                    printf("Error from packRequest\n");
                    bad_exit();
                }
     
            }else if(strcmp("/list", token1) == 0){
            
                // pack the message for list
                ret = packRequest(message, REQ_LIST, NULL, NULL, NULL);
                if (ret == -1){
                    printf("Error from packRequest\n");
                    bad_exit();
                }
            
            }else if(strcmp("/who", token1) == 0){ // check token 2

                // check the channel parameter
                if (token2 == NULL)            
                    goto UNKOWN_COMMAND;
    
                // pack the message for who
                ret = packRequest(message, REQ_WHO, token2, NULL, NULL);
                if (ret == -1){
                    printf("Error from packRequest\n");
                    bad_exit();
                }

/*            }else if(strcmp("/switch", token1) == 0){ // check token 2
 
                // check the channel parameter
                if (token2 == NULL)            
                    goto UNKOWN_COMMAND;
   
                while(strncmp(activeChannel, channels[])){
           	
		}
     
                // modify our local activeChannel to be the one requested 
                strncpy(activeChannel, token2, CHANNEL_MAX); 
*/
            }else{ //wrong
                
                UNKOWN_COMMAND:
                // inform the client user the command is not valid
                printf("*Unkown command\n"); 

            }

        }else{ // implies say message
                
            // pack the message for say
            ret = packRequest(message, REQ_SAY, activeChannel, sayBuf, NULL);
            if (ret == -1){
                printf("Error from packRequest\n");
                bad_exit();
            }
 
        }

// (6) Send client message to server

        ret = sendRequest(sockfd, message);
        if (ret == -1){
            printf("Error during sendRequest\n");
            bad_exit();
        }


    }//end Main event loop

    // should never get here    
    bad_exit();

}

int parseServer(char *dest, char *input){
	
	text_say say = (text_say) input;
	sscanf(dest, "[%s][%s]: %s", say.txt_channel, say.txt_username, say.txt_text);
	return 0;
}

int packRequest(struct request *message, request_t req_type, char *channel, char *say, char *uname){

    // verify input params
    if (req_type > 7 && req_type < 0)
        return -1;
    
    if (channel != NULL)
        if ( (strlen(channel) + 1) > CHANNEL_MAX)
            return -1;
    
    if (say != NULL)
        if ( (strlen(say) + 1) > SAY_MAX)
            return -1;
    
    if (uname != NULL)
        if ( (strlen(uname) + 1) > USERNAME_MAX)
            return -1;

    // initialize user message
    memset(message, 0, MESSAGE_MAX);

    // set req_type field
    message->req_type = htonl(req_type);

    // set remaining fields based on req_type
    switch (req_type){
        case REQ_LOGIN:
            strncpy(((request_login*)message)->req_username, uname, strlen(uname));        
            break;

        case REQ_LOGOUT:
            // only needs req_type
            break;
            
        case REQ_JOIN:
            strncpy(((request_join*)message)->req_channel, channel, strlen(channel));        
            break;
          
        case REQ_LEAVE: 
            strncpy(((request_leave*)message)->req_channel, channel, strlen(channel));        
            break; 
         
        case REQ_SAY:
            strncpy(((request_say*)message)->req_channel, channel, strlen(channel));
            strncpy(((request_say*)message)->req_text, say, strlen(say));
            break;             
 
        case REQ_LIST:
            // only needs req_type
            break;
        
        case REQ_WHO:
            strncpy(((request_who*)message)->req_channel, channel, strlen(channel));
            break;

        case REQ_KEEP_ALIVE:
            // only needs req_type
            break;
        
        default:
            // should literally NEVER get here
            return -1;
    }
    
    // exit the function successfully
    return 0;

}

int sendRequest(int fd, struct request *message){
    
    int 
        ret,        // error checking return val
        req_type;   // request type for switch
     
    // initialize the request type for our switch
    req_type = ntohl(message->req_type);
    
    // write the message to the socket, checking the max length of each message
    switch (req_type){
        
        case REQ_LOGIN:
            ret = write(fd, message, sizeof(request_t) + USERNAME_MAX);
            if (ret == -1){
                perror("Error during sendRequest");
                return -1;
            }
            break;

        case REQ_LOGOUT:
            ret = write(fd, message, sizeof(request_t));
            if (ret == -1){
                perror("Error during sendRequest");
                return -1;
            }
            break;
            
        case REQ_JOIN:
            ret = write(fd, message, sizeof(request_t) + CHANNEL_MAX);
            if (ret == -1){
                perror("Error during sendRequest");
                return -1;
            }
            break;
          
        case REQ_LEAVE: 
            ret = write(fd, message, sizeof(request_t) + CHANNEL_MAX);
            if (ret == -1){
                perror("Error during sendRequest");
                return -1;
            }
            break;
         
        case REQ_SAY:
            ret = write(fd, message, sizeof(request_t) + CHANNEL_MAX + SAY_MAX);
            if (ret == -1){
                perror("Error during sendRequest");
                return -1;
            }
            break;
 
        case REQ_LIST:
            ret = write(fd, message, sizeof(request_t));
            if (ret == -1){
                perror("Error during sendRequest");
                return -1;
            }
            break;
        
        case REQ_WHO:
            ret = write(fd, message, sizeof(request_t) + CHANNEL_MAX);
            if (ret == -1){
                perror("Error during sendRequest");
                return -1;
            }
            break;

        case REQ_KEEP_ALIVE:
            ret = write(fd, message, sizeof(request_t));
            if (ret == -1){
                perror("Error during sendRequest");
                return -1;
            }
            break;
        
        default:
            // should literally NEVER get here
            return -1;
    }
    return 0;
}

void clean_exit(){
    cooked_mode();
    exit(0);
}

void bad_exit(){
    cooked_mode();
    exit(1);
}
