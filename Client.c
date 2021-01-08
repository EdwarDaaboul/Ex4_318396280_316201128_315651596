/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/
/*
In this file there is all the function for the client in order to contact successfully with server.
 */
 /*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#include <Windows.h>

#include "msgs_file.h"
#include "ThreadsChecks.h"
#include "SocketExampleShared.h"
#include "SocketSendRecvTools.h"


/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

SOCKET m_socket;
BOOL server_approved = FALSE;
BOOL server_denied = FALSE;
BOOL req_move = FALSE;
BOOL setup_move = FALSE;
BOOL done = FALSE;
HANDLE recvd_msg_event;
HANDLE recieving_thread;
int waiting_status = NO_WAIT;
char op_name[MAX_USERNAME_LEN];
BOOL was_error = FALSE;
BOOL fail_menu = FALSE;
BOOL no_op = FALSE;
/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

//Reading data coming from the server
static DWORD RecvDataThread(void)
{
	TransferResult_t RecvRes;
	while (!done)
	{
		char* AcceptedStr = NULL;
		RecvRes = ReceiveMsg(&AcceptedStr, m_socket);
		if (RecvRes == TRNS_FAILED)
		{
			printf("Socket error while trying to write data to socket\n");
			return 0x555;
		}
		else if (RecvRes == TRNS_DISCONNECTED)
		{
			fail_menu = TRUE;
		}
		else
		{
			printf("%s\n", AcceptedStr); //DELETEME
			//server approved status - check done step 5 part 1
			if (is_msgs_equal(AcceptedStr, "SERVER_APPROVED\n"))
			{
			
				//update approval status
				if (got_approval(AcceptedStr) == STATUS_CODE_FAILURE) was_error = TRUE;
			}
			//server denied status - check done step 5 part 2
			else if (is_denied(AcceptedStr))
			{
				if (got_denied(AcceptedStr) == STATUS_CODE_FAILURE) was_error = TRUE;
			}
			//server main menu - check done step 6 , check done step 8 , check done step 7 part 2
			
			else if (is_msgs_equal(AcceptedStr, "SERVER_MAIN_MENU\n"))
			{
				
				if (got_main_menu(AcceptedStr) == STATUS_CODE_FAILURE) was_error = TRUE;
			}
			else if (is_msgs_equal(AcceptedStr, "SERVER_NO_OPPONENTS\n"))
			{
				no_op = TRUE;
				//waiting_status =WAIT_30;
			}
			//server send SERVER_INVITE and oponent name saved in op_name-> step 7 part 1
			else if (is_invite(AcceptedStr)) { printf("Game is on\n"); }
			//CLIENT SETUP - check done step 9
			else if (is_msgs_equal(AcceptedStr, "SERVER_SETUP_REQUSET\n"))
			{
				waiting_status = NO_WAIT;
				if (stup_move(AcceptedStr) == STATUS_CODE_FAILURE) was_error = TRUE;
			}
			//CLIENT MOVE - check done step 10 part 1
			else if (is_msgs_equal(AcceptedStr, "SERVER_PLAYER_MOVE_REQUEST\n"))
			{
				if (requested_move(AcceptedStr) == STATUS_CODE_FAILURE) was_error = TRUE;
			}
			//update results - check done step 10 part 2
			else if (is_results_msg(AcceptedStr)) {} 
			//winner message - check done step 11 and 13
			else if (is_winner_msg(AcceptedStr)) { main_menu_client_req(AcceptedStr); }
			//draw message -> check done step 12 and 13
			else if (is_draw_msg(AcceptedStr)) { main_menu_client_req(AcceptedStr); }


			//To add SERVER_OPPONENT_QUIT (14) -> print Opponent quit.
			/*else if (is_msgs_equal(AcceptedStr, "SERVER_GAME_OVER_MENU\n"))
			{
				if (game_over_menu(AcceptedStr) == STATUS_CODE_FAILURE) was_error = TRUE;
			}
			*/
		}
		free(AcceptedStr);
		if (SetEvent_check(recvd_msg_event) == STATUS_CODE_FAILURE) return STATUS_CODE_FAILURE;
		if (was_error) return STATUS_CODE_FAILURE;

	}

	return STATUS_CODE_SUCCESS;
}

//Sending data to the server
int send_msg(SEND_MSG_ARGS_params_t send_msg_args) {
	char SendStr[SEND_STR_SIZE];
	TransferResult_t SendRes;
	int MsgTypLen = strlen(send_msg_args.message_type);
	//gets_s(SendStr, sizeof(SendStr)); //Reading a string from the keyboard
	strcpy_s(SendStr, MAX_MESSAGE_TYPE_LEN, send_msg_args.message_type);
	//checking arguments and did the appropriate format
	if (strcmp(send_msg_args.message_args, " ") != 0) {
		strcat_s(SendStr, MAX_MESSAGE_TYPE_LEN + 1, ":");
		strcat_s(SendStr, MAX_MESSAGE_TYPE_LEN + MAX_USERNAME_LEN + 1, send_msg_args.message_args);
	}
	strcat_s(SendStr, MAX_MESSAGE_TYPE_LEN + MAX_USERNAME_LEN + 2, "\n");

	//send the msg
	SendRes = SendMsg(SendStr, m_socket);

	if (SendRes == TRNS_FAILED)
	{
		printf("Socket error while trying to write data to socket\n");
		return STATUS_CODE_FAILURE;
	}
	return STATUS_CODE_SUCCESS;
}
//creating thread that recieve data from resever and recieve data
int start_recieve_msgs() {

	recieving_thread = CreateThread(
		NULL,
		0,
		(LPTHREAD_START_ROUTINE)RecvDataThread,
		NULL,
		0,
		NULL
	);
	if (NULL == recieving_thread)
	{
		printf("Error: Couldn't create thread\n");
		return STATUS_CODE_FAILURE;
	}
	return STATUS_CODE_SUCCESS;

}

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

int initiate_event() {
	DWORD last_error;

	/* Get handle to event by name. If the event doesn't exist, create it */
	recvd_msg_event = CreateEvent(
		NULL, /* default security attributes */
		FALSE,       /* manual-reset event */
		FALSE,      /* initial state is non-signaled */
		NULL);         /* name */

	/* Check if succeeded and handle errors */
	last_error = GetLastError();
	/* If last_error is ERROR_SUCCESS, then it means that the event was created.
		If last_error is ERROR_ALREADY_EXISTS, then it means that the event already exists */
	if (last_error == ERROR_ALREADY_EXISTS) {
		printf("Event already exist.\n");
		return STATUS_CODE_FAILURE;
	}
	else if (last_error != ERROR_SUCCESS) {
		printf("Error while creating thread!\n");
		return STATUS_CODE_FAILURE;
	}
	else {
		// Creation succed
		return STATUS_CODE_SUCCESS;
	}
}

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/
int event_wait_decode_beginning(DWORD waiting_res, char* server_ip, char* port_str) {
	if (was_error) return FINISH;
	if (waiting_res == WAIT_TIMEOUT) {
		printf("1 Failed connecting to server on %s:%s.\n", server_ip, port_str);
		if (print_fail_menu() == 1) return RETRY;
		else return FINISH;
	}
	else if (waiting_res == WAIT_OBJECT_0) {
		// event signaled
		// got response, check if approved or denied
		if (server_approved)
		{
			// server approved
			return CONTINUE;
		}
		else if (server_denied)
		{
			// server_denied
			printf("Server on %s:%s denied the connection request.\n", server_ip, port_str);
			if (print_fail_menu() == 1) return RETRY;
			else return FINISH;
		}
		else {
			// unexpected message
			printf("Error - recieved unexpected message!\n");
			return WAIT_AGAIN_BGN;
		}
	}
	else {
		// Waiting failed
		printf("Error while waiting for event!\n");
		return FINISH;
	}

}
int event_wait_decode_proc(DWORD waiting_res, char* server_ip, char* port_str) {
	if (was_error) return FINISH;
	if (done)	   return FINISH;
	if (waiting_res == WAIT_TIMEOUT) {
		//printf("the waiting status is:%d\n", waiting_status); //DELETEME 
		if (waiting_status == WAIT_15) {
			printf("2 Failed connecting to server on %s:%s.\n", server_ip, port_str);
			if (print_fail_menu() == 1) return RETRY;
			else return FINISH;
		}
		if (waiting_status == WAIT_30) {
			printf("now im 30 to 15 \n");
			waiting_status = WAIT_15;
		}
		return WAIT_AGAIN_PROC;
	}
	else if (waiting_res == WAIT_OBJECT_0) {
		// check flags
		if (fail_menu) return RETRY;
		return WAIT_AGAIN_PROC;
	}
	else {
		// Waiting failed
		printf("Error while waiting for event!\n");
		return FINISH;
	}
}


/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

short port_str2short(char* port_str) {
	short res = atoi(port_str);
	return res;
}


//request message
int send_client_request(char* username) {
	// Sending client request with username to start connection -> intiate formate
	SEND_MSG_ARGS_params_t send_msg_args;
	strcpy_s(send_msg_args.message_type, MAX_MESSAGE_TYPE_LEN, "CLIENT_REQUEST");
	strcpy_s(send_msg_args.message_args, MAX_USERNAME_LEN, username);
	//sending the request message --> formate function -> send the message
	if (send_msg(send_msg_args) != STATUS_CODE_SUCCESS) return STATUS_CODE_FAILURE;
	return STATUS_CODE_SUCCESS;

}

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

int MainClient(char* server_ip, char* port_str, char* username)
{
	SOCKADDR_IN clientService;

	// Initialize Winsock.
	WSADATA wsaData;

	//Call WSAStartup and check for errors.
	//initialing Winsock Environment
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != NO_ERROR) {
		printf("Error while starting connection at WSAStartup()\n");
		return STATUS_CODE_FAILURE;
	}


	// Create a socket.
	m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	// Check for errors to ensure that the socket is a valid socket.
	if (m_socket == INVALID_SOCKET) {
		printf("Error while starting connection at socket()\n");
		WSACleanup();
		return STATUS_CODE_FAILURE;
	}


	// Connect to a server. Create a sockaddr_in object clientService and set  values.
	clientService.sin_family = AF_INET;
	clientService.sin_addr.s_addr = inet_addr(server_ip); //Setting the IP address to connect to
	clientService.sin_port = htons(port_str2short(port_str)); //Setting the port to connect to.

	// Call the connect function, passing the created socket and the sockaddr_in structure as parameters. 
	// Check for general errors.
retry_connect:
	if (connect(m_socket, (SOCKADDR*)&clientService, sizeof(clientService)) == SOCKET_ERROR) {
		//fail to connect -> print num 3 in client behavior
		printf("3 Failed connecting to server on %s:%s.\n", server_ip, port_str);
		//if the client choose 1 -> reconnect
		if (print_fail_menu() == 1) goto retry_connect;
		//if the client choose 2 -> Exit 
		else goto cleanup_and_exit_0;
	}
	//at this point the connect success
	printf("Connected to server on %s:%s\n", server_ip, port_str);
	// Send and receive data.
	//this event will did the wait for recieve data (recvd_msg_event). 
	if (initiate_event() == STATUS_CODE_FAILURE) { was_error = TRUE;  goto cleanup_and_exit_1; } 
	// Sending client request with username to start connection -> client behavior num 5 part 1
	if (send_client_request(username) != STATUS_CODE_SUCCESS) { was_error = TRUE;  goto cleanup_and_exit_2; }
	// Create thread that start recieving messages -> client behavior num 5 part 2
	if (start_recieve_msgs() == STATUS_CODE_FAILURE) { was_error = TRUE;  goto cleanup_and_exit_2; } 




	DWORD waiting_res;
	int waiting_decode;

wait_for_first_message:
	waiting_res = WaitForSingleObject(recvd_msg_event, MAX_RESPONSE_WAITING_TIME);
	waiting_decode = event_wait_decode_beginning(waiting_res, server_ip, port_str);
	switch (waiting_decode)
	{
	case CONTINUE:   break;
	case RETRY:      goto retry_connect;
	case FINISH:     goto cleanup_and_exit_3;
	case WAIT_AGAIN_BGN: goto wait_for_first_message;
	}
	
	/// HERE WE SUCCESSED TO CONNECT - READY TO APPLICATION

wait_for_message:
	//printf("now we are waiting for message from server\n");
	waiting_res = WaitForSingleObject(recvd_msg_event, MAX_RESPONSE_WAITING_TIME);
	waiting_decode = event_wait_decode_proc(waiting_res, server_ip, port_str);

	switch (waiting_decode)
	{
	case RETRY:      goto retry_connect;
	case FINISH:     goto cleanup_and_exit_3;
	case WAIT_AGAIN_PROC: goto wait_for_message;
	}



cleanup_and_exit_3:
	printf("Waiting for thread!\n");

	waiting_res = WaitForSingleObject(recieving_thread, MAX_RESPONSE_WAITING_TIME);
	if (waiting_res == WAIT_TIMEOUT) {
		printf("Terminating thread!\n");
		BOOL Terminate = TerminateThread(recieving_thread, 0);
		if (Terminate == 0) {
			printf("Threads termenation failed. Error is %d\n", GetLastError());
		}
	}
	CloseHandle(recieving_thread);

cleanup_and_exit_2:
	CloseHandle(recvd_msg_event);

	//cleanup at initiate_event point -> we have to cleanup the socket and WSACleanup
cleanup_and_exit_1:
	closesocket(m_socket);

	//cleanup for fail to connect spesific and  WSACleanup for general clean
cleanup_and_exit_0:
	WSACleanup();
	if (was_error) return STATUS_CODE_FAILURE;
	else          return STATUS_CODE_SUCCESS;
}

