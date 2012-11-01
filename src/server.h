#ifndef SERVER_H
#define SERVER_H

//parseLogin must do two things:
//add the user to the users map, storing their ip address
//have the user join channel Common
int parseLogin();
    
//parseLogout must do two things:
//remove the user from all channels s/he is still in
//remove the user from users map
int parseLogout();

//parseJoin needs to add the user to the map of the requested channel
int parseJoin();

//parseLeave needs to remove the user from the requested channel
int parseLeave();

//parseSay needs to do the following:
//find out which channel to say at
//sendto() the message to all users in the given channel
int parseSay();

//parseWho needs to construct and send the SAY_WHO datagram containing a list of users in a channel
int parseWho();

//parseList needs to construct the TXT_List datagram containing a formatted list of all channels
int parseList();

#endif
