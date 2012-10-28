#ifndef CLIENT_H
#define CLIENT_H

#include "duckchat.h"

int packRequest(struct request *message, 
                request_t req_type, 
                char *channel, 
                char *say, 
                char *uname);

//Be sure errno is set upon return w/ -1!!!
int sendRequest(int fd, struct request *message);

void bad_exit();
void clean_exit();


#endif
