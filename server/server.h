
/*In this file we declared the main function of the server*/


#ifndef SERVER_H
#define SERVER_H

#define MAX_RESPONSE_WAITING_TIME 15000 // 15 seconds
#define MAX_USERNAME_LEN 20+1
#define BULLS_COWS_STR_LEN 3
#define WIN_INDEX_LEN 2
#define FOUR_BULLS '4'
#define UNINITIALIZED -1
int Main_server(char* port_str);


#endif
