/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/
/*
# Description:
#	In this header file we have defined some parameters and declared our functions
*/
/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

#ifndef SOCKET_EXAMPLE_SHARED_H
#define SOCKET_EXAMPLE_SHARED_H

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

#define SERVER_ADDRESS_STR "127.0.0.1"
#define SERVER_PORT 2345
#define NUM_OF_WORKER_THREADS 2
#define SEND_STR_SIZE 256 
#define LAST_PLAYER -1
#define ONE_RET_VAL 1
#define ZERO_RET_VAL 0
#define MAX_MOVE_LENGHT 10
#define WAIT_TIME_TO_SEND 1000000 
#define PLAYER_LEN_NAME 8
#define STRINGS_ARE_EQUAL( Str1, Str2 ) ( strcmp( (Str1), (Str2) ) == 0 )

// ***** STATUS CONSTANTS *****
static const int STATUS_CODE_SUCCESS = 0;
static const int STATUS_CODE_FAILURE = -1;

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

#endif // SOCKET_EXAMPLE_SHARED_H