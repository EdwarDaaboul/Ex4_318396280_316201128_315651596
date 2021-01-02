/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/
/* 
Description:
#	In this file we have defined some parameters and functions
*/
/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

#include "SocketSendRecvTools.h"


#include <stdio.h>
#include <string.h>

void print_msg(char *msg) {
	int idx = 0;
	while (1) {
		printf("%c", msg[idx]);
		if (msg[idx] == '\n') break;
		if (msg[idx] == '\0') { printf("\n"); break; }
		idx++;
	}
}
int MsgLen(char *msg_buffer) {
	int len = 0;
	while (1) {
		if (msg_buffer[len] == '\n') break;
		len++;
	}
	return len;
}
BOOL is_msgs_equal(char *msg1, char *msg2) {
	int idx = 0;
	while (1) {
		if (msg1[idx] == msg2[idx]) {
			if (msg1[idx] == '\n') return TRUE;
			idx++;
		}
		else return FALSE;
	}
}
int upper_case(char *buffer) {

	int i = 0;
	while (*(buffer + i) != '\0')
	{
		if (*(buffer + i) >= 'a' && *(buffer + i) <= 'z')
		{
			*(buffer + i) = *(buffer + i) - 'a' + 'A';
		}
		i++;
	}return 0;

}


/*oOoOoOoOoOoOoO  Functions to send messages  oOoOoOoOoOoOoOoOoOoOoO*/

TransferResult_t SendBuffer( const char* Buffer, int BytesToSend, SOCKET sd )
{
	const char* CurPlacePtr = Buffer;
	int BytesTransferred;
	int RemainingBytesToSend = BytesToSend;
	
	while ( RemainingBytesToSend > 0 )  
	{
		/* send does not guarantee that the entire message is sent */
		BytesTransferred = send (sd, CurPlacePtr, RemainingBytesToSend, 0);
		if ( BytesTransferred == SOCKET_ERROR ) 
		{
			printf("Sending message failed!\n");
			return TRNS_FAILED;
		}
		
		RemainingBytesToSend -= BytesTransferred;
		CurPlacePtr += BytesTransferred; // <ISP> pointer arithmetic
	}

	return TRNS_SUCCEEDED;
}
TransferResult_t SendMsg( const char *Msg, SOCKET sd )
{
	/* Send the request to the server on socket sd */
	int TotalMsgSizeInBytes;
	TransferResult_t SendRes;

	/* The request is sent in two parts. First the Length of the msg (stored in 
	   an int variable ), then the msg itself. */
		
	TotalMsgSizeInBytes = (int)( MsgLen(Msg) + 1 ); // terminating \n

	SendRes = SendBuffer( 
		(const char *)( &TotalMsgSizeInBytes ),
		(int)( sizeof(TotalMsgSizeInBytes) ), // sizeof(int) 
		sd );

	if ( SendRes != TRNS_SUCCEEDED ) return SendRes ;

	SendRes = SendBuffer( 
		(const char *)( Msg ),
		(int)( TotalMsgSizeInBytes ), 
		sd );

	return SendRes;
}

/*oOoOoOoOoOoOoO  Functions to receive messages  oOoOoOoOoOoOoOoOoOoOoO*/

TransferResult_t ReceiveBuffer( char* OutputBuffer, int BytesToReceive, SOCKET sd )
{
	char* CurPlacePtr = OutputBuffer;
	int BytesJustTransferred;
	int RemainingBytesToReceive = BytesToReceive;
	
	while ( RemainingBytesToReceive > 0 )  
	{
		/* send does not guarantee that the entire message is sent */
		BytesJustTransferred = recv(sd, CurPlacePtr, RemainingBytesToReceive, 0);
		if ( BytesJustTransferred == SOCKET_ERROR ) 
		{
			printf("Receiving message failed!\n");
			return TRNS_FAILED;
		}		
		else if ( BytesJustTransferred == 0 )
			return TRNS_DISCONNECTED; // recv() returns zero if connection was gracefully disconnected.

		RemainingBytesToReceive -= BytesJustTransferred;
		CurPlacePtr += BytesJustTransferred; // <ISP> pointer arithmetic
	}

	return TRNS_SUCCEEDED;
}
TransferResult_t ReceiveMsg( char** OutputMsgPtr, SOCKET sd )
{
	/* Recv the the request to the server on socket sd */
	int TotalMsgSizeInBytes;
	TransferResult_t RecvRes;
	char* MsgBuffer = NULL;

	if ( ( OutputMsgPtr == NULL ) || ( *OutputMsgPtr != NULL ) )
	{
		printf("The first input to ReceiveString() must be " 
			   "a pointer to a char pointer that is initialized to NULL. For example:\n"
			   "\tchar* Buffer = NULL;\n"
			   "\tReceiveString( &Buffer, ___ )\n" );
		return TRNS_FAILED;
	}

	/* The request is received in two parts. First the Length of the msg (stored in 
	   an int variable ), then the msg itself. */
		
	RecvRes = ReceiveBuffer( 
		(char *)( &TotalMsgSizeInBytes ),
		(int)( sizeof(TotalMsgSizeInBytes) ), // 4 bytes
		sd );

	if ( RecvRes != TRNS_SUCCEEDED ) return RecvRes;

	MsgBuffer = (char*)malloc( TotalMsgSizeInBytes * sizeof(char) );

	if (MsgBuffer == NULL) {
		printf("Failed to allocate memory for recieved message!\n");
		return TRNS_FAILED;
	}

	RecvRes = ReceiveBuffer( 
		(char *)( MsgBuffer ),
		(int)( TotalMsgSizeInBytes), 
		sd );

	if ( RecvRes == TRNS_SUCCEEDED ) 
		{ *OutputMsgPtr = MsgBuffer; }
	else
	{
		free( MsgBuffer );
	}
		
	return RecvRes;
}
