/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/
/*  
# Description:
#	In this header file we have defined some parameters and declared our functions
*/
/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

#ifndef SOCKET_SEND_RECV_TOOLS_H
#define SOCKET_SEND_RECV_TOOLS_H

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/
#define _CRT_SECURE_NO_WARNINGS
#define MAX_MESSAGE_TYPE_LEN 19
#define MAX_USERNAME_LEN 20+1
#define MAX_RESPONSE_WAITING_TIME 15000 // 15 seconds
#define MAX_MOVE_LEN 4+1
#define LEN_BULLS_OR_COWS 1+1
/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/
typedef struct
{
	char message_type[MAX_MESSAGE_TYPE_LEN]; // input
	char message_args[MAX_USERNAME_LEN+ MAX_MOVE_LEN]; //input
} SEND_MSG_ARGS_params_t;

typedef enum { TRNS_FAILED, TRNS_DISCONNECTED, TRNS_SUCCEEDED } TransferResult_t;
typedef enum { CONTINUE, RETRY, FINISH, WAIT_AGAIN_BGN, WAIT_AGAIN_PROC, FAIL_MENU} WaitDecode_t;
typedef enum { NO_WAIT, WAIT_15, WAIT_30 } WaitStatus_t;


void print_msg(char *msg);
/**
 * print_msg() prints the message from the client.
 *
 * Accepts:
 * -------
 * a pointer to a string.
 *
 * Returns:
 * -------
 * no return
 */
BOOL is_msgs_equal(char *msg1, char *msg2);
/**
 * is_msgs_equal() equals the message from the client with the message type given.
 *
 * Accepts:
 * -------
 * a pointer to two strings.
 *
 * Returns:
 * -------
 * TRUE if equal, FALSE else.
 */
int upper_case(char *buffer); 
/**
 * is_msgs_equal() equals the message from the client with the message type given.
 *
 * Accepts:
 * -------
 * a pointer to two strings.
 *
 * Returns:
 * -------
 * STATUS_CODE_SUCCESS on success, STATUS_CODE_FALIURE on faliure else.
 */
int MsgLen(char *msg_buffer);
/**
 * MsgLen() calcutes the length of the buffer.
 *
 * Accepts:
 * -------
 * a pointer to a strings.
 *
 * Returns:
 * -------
 * STATUS_CODE_SUCCESS on success, STATUS_CODE_FALIURE on faliure else.
 */

TransferResult_t SendBuffer( const char* Buffer, int BytesToSend, SOCKET sd );

/**
 * SendMsg() uses a socket to send a string.
 * Str - the string to send. 
 * sd - the socket used for communication.
 */ 
TransferResult_t SendMsg( const char *Str, SOCKET sd );

/**
 * Accepts:
 * -------
 * ReceiveBuffer() uses a socket to receive a buffer.
 * OutputBuffer - pointer to a buffer into which data will be written
 * OutputBufferSize - size in bytes of Output Buffer
 * BytesReceivedPtr - output parameter. if function returns TRNS_SUCCEEDED, then this 
 *					  will point at an int containing the number of bytes received.
 * sd - the socket used for communication.
 *
 * Returns:
 * -------
 * TRNS_SUCCEEDED - if receiving succeeded
 * TRNS_DISCONNECTED - if the socket was disconnected
 * TRNS_FAILED - otherwise
 */ 
TransferResult_t ReceiveBuffer( char* OutputBuffer, int RemainingBytesToReceive, SOCKET sd );

/**
 * ReceiveMsg() uses a socket to receive a string, and stores it in dynamic memory.
 * 
 * Accepts:
 * -------
 * OutputStrPtr - a pointer to a char-pointer that is initialized to NULL, as in:
 *
 *		char *Buffer = NULL;
 *		ReceiveMsg( &Buffer, ___ );
 *
 * a dynamically allocated string will be created, and (*OutputStrPtr) will point to it.
 * 
 * sd - the socket used for communication.
 * 
 * Returns:
 * -------
 * TRNS_SUCCEEDED - if receiving and memory allocation succeeded
 * TRNS_DISCONNECTED - if the socket was disconnected
 * TRNS_FAILED - otherwise
 */ 
TransferResult_t ReceiveMsg( char** OutputStrPtr, SOCKET sd );


/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

#endif // SOCKET_SEND_RECV_TOOLS_H