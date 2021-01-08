#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
//SEARCHME
//CHANGEME
//CHANGEDHERE
//ADDEDHERE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <io.h>
#include <errno.h>

#include "SocketExampleShared.h"
#include "SocketSendRecvTools.h"
#include "server.h"
#include "FileHandlers.h"
#include "MessageDecoding.h"

//DELETEME

#define stdin (__acrt_iob_func(0))

HANDLE ExitHandle = NULL;
HANDLE versus_event = NULL;
HANDLE finish_writing_event = NULL;
HANDLE  oponent_name_event = NULL;
int g_num_of_hanldes; 
HANDLE* g_workers_handles;
BOOL Exit = FALSE;
HANDLE MutexHandleForFile = NULL;
HANDLE MutexHandleForCreatingFile = NULL;
BOOL two_players_connected = FALSE;
int There_are_two_players = LAST_PLAYER;				//CHANGEDHERE was: int last_played = LAST_PLAYER;
char* GameSessionFile = "GameSession.txt";
int lines_num = 0;
char user1_name[MAX_USERNAME_LEN];
char user2_name[MAX_USERNAME_LEN];
int win[WIN_INDEX_LEN] = { -1,-1 };		//ADDEDHERE


static int FindFirstUnusedThreadSlot();
static void CleanupWorkerThreads();
static DWORD ServiceThread(SOCKET* t_socket);
static DWORD DeniedThread(SOCKET* t_socket);
SOCKET MainSocket;
send_msg(SEND_MSG_ARGS_params_t send_msg_args, SOCKET m_socket) {
	char SendStr[SEND_STR_SIZE];
	TransferResult_t SendRes;
	int MsgTypLen = strlen(send_msg_args.message_type);
	strcpy_s(SendStr, MAX_MESSAGE_TYPE_LEN, send_msg_args.message_type);
	if (strcmp(send_msg_args.message_args, " ") != 0) {
		strcat_s(SendStr, MAX_MESSAGE_TYPE_LEN + 1, ":");
		strcat_s(SendStr, MAX_MESSAGE_TYPE_LEN + MAX_USERNAME_LEN + MAX_MOVE_LEN + 1, send_msg_args.message_args);
	}
	strcat_s(SendStr, MAX_MESSAGE_TYPE_LEN + MAX_USERNAME_LEN + MAX_MOVE_LEN + 2, "\n");
	SendRes = SendMsg(SendStr, m_socket);

	if (SendRes == TRNS_FAILED)
	{
		printf("Socket error while trying to write data to socket\n");
		return STATUS_CODE_FAILURE;
	}
	return STATUS_CODE_SUCCESS;
}
int handling_wait_code(int wait_code) {
	////handling the wait time for the WaitForSingleObject() function
	switch (wait_code)
	{
	case WAIT_TIMEOUT:
		printf("Threads timeout!\n");
		return STATUS_CODE_FAILURE;
		break;
	case WAIT_OBJECT_0:
		return STATUS_CODE_SUCCESS;
		break;
	case WAIT_FAILED:
		printf("Process failed! Error is %d\n", GetLastError());
		return STATUS_CODE_FAILURE;
		break;
	default:
		printf("Waiting Failed: 0x%x\n", wait_code);
		return STATUS_CODE_FAILURE;
	}
}

char* cows_and_bulls(char client1_choice[4], char client2_guess[4], char buffer[3])
{
	int i, bulls = 0, cows = 0;
	for (i = 0; i < 4; i++)
	{
		if (client1_choice[i] == client2_guess[i])
			bulls++;
		else if (NULL != strchr(client1_choice, client2_guess[i]))
			cows++;
	}
	_itoa_s(bulls, buffer, 3, 10);
	_itoa_s(cows, buffer + 1, 3, 10);
	printf("the number of bulls is : %c\n", buffer[0]);
	printf("the number of cows is : %c\n", buffer[1]);
	return buffer;

}
/*this file checks if file do exist and no need to open once more*/
void close_all_handles() {
	if (!CloseHandle(ExitHandle)) printf("Failed to close EXITHANDLE\n", WSAGetLastError());
	if (!CloseHandle(versus_event)) printf("Failed to close VERSUSHANDLE\n", WSAGetLastError());
	if (!CloseHandle(finish_writing_event)) printf("Failed to close WRITINGHANDLE\n", WSAGetLastError());
	if (!CloseHandle(oponent_name_event)) printf("Failed to close NAMEHANDLE\n", WSAGetLastError());
}
static void CleanupWorkerThreads()
{
	int Ind;

	for (Ind = 0; Ind < g_num_of_hanldes; Ind++)
	{
		if (g_workers_handles[Ind] != NULL)
		{
			// poll to check if thread finished running:
			DWORD Res = WaitForSingleObject(g_workers_handles[Ind], MAX_RESPONSE_WAITING_TIME);

			if (Res == WAIT_OBJECT_0)
			{

				CloseHandle(g_workers_handles[Ind]);
				g_workers_handles[Ind] = NULL;
				break;
			}
			else
			{
				BOOL Terminate = TerminateThread(g_workers_handles[Ind], 0);
				if (Terminate == 0) {
					printf("Threads termenation failed. Error is %d\n", GetLastError());
				}
				continue;
			}
		}
	}
}
static DWORD send_server_draw(SOCKET* t_socket){
	// This function sends SERVER_DRAW and then MAIN_MENU messages.
	TransferResult_t SendRes;
	char SendStr[SEND_STR_SIZE];
	strcpy(SendStr, "SERVER_DRAW\n");
	SendRes = SendMsg(SendStr, *t_socket);
	if (SendRes == TRNS_FAILED)
	{
		printf("Error while sending SERVER_DRAW, closing thread.\n");
		closesocket(*t_socket);
		return STATUS_CODE_FAILURE;
	}
	strcpy(SendStr, "SERVER_MAIN_MENU\n");
	SendRes = SendMsg(SendStr, *t_socket);
	if (SendRes == TRNS_FAILED)
	{
		printf("Error while sending SERVER_MAIN_MENU, closing thread.\n");
		closesocket(*t_socket);
		return STATUS_CODE_FAILURE;
	}
	return STATUS_CODE_SUCCESS;
}
static DWORD send_server_win(SOCKET* t_socket, char* winner_id,char* other_client_numbers) {
	// This function sends SERVER_WIN and then MAIN_MENU messages.
	TransferResult_t SendRes;
	char SendStr[SEND_STR_SIZE];
	strcpy(SendStr, "SERVER_WIN:");
	strcat(SendStr, winner_id);
	strcat(SendStr, ";");
	strcat(SendStr, other_client_numbers); 
	strcat(SendStr, '\n');
	SendRes = SendMsg(SendStr, *t_socket);
	if (SendRes == TRNS_FAILED)
	{
		printf("Error while sending SERVER_WIN, closing thread.\n");
		closesocket(*t_socket);
		return STATUS_CODE_FAILURE;
	}
	strcpy(SendStr, "SERVER_MAIN_MENU\n");
	SendRes = SendMsg(SendStr, *t_socket);
	if (SendRes == TRNS_FAILED)
	{
		printf("Error while sending SERVER_MAIN_MENU, closing thread.\n");
		closesocket(*t_socket);
		return STATUS_CODE_FAILURE;
	}
	return STATUS_CODE_SUCCESS;

}
static DWORD send_no_opponet(SOCKET* t_socket) {
	// This function sends NO OPONENT and then MAIN_MENU messages.
	TransferResult_t SendRes;
	char SendStr[SEND_STR_SIZE];
	strcpy(SendStr, "SERVER_NO_OPPONENTS\n");
	SendRes = SendMsg(SendStr, *t_socket);
	if (SendRes == TRNS_FAILED)
	{
		printf("Error while sending SERVER_NO_OPPONENTS, closing thread.\n");
		closesocket(*t_socket);
		return STATUS_CODE_FAILURE;
	}
	strcpy(SendStr, "SERVER_MAIN_MENU\n");
	SendRes = SendMsg(SendStr, *t_socket);
	if (SendRes == TRNS_FAILED)
	{
		printf("Error while sending SERVER_MAIN_MENU, closing thread.\n");
		closesocket(*t_socket);
		return STATUS_CODE_FAILURE;
	}
	return STATUS_CODE_SUCCESS;
}
static DWORD send_player_move_request(SOCKET* t_socket, char* client_id) {

	// This func sends player move request
	TransferResult_t SendRes;
	char SendStr[SEND_STR_SIZE] = "SERVER_PLAYER_MOVE_REQUEST\n";
	SendRes = SendMsg(SendStr, *t_socket);
	if (SendRes == TRNS_FAILED)
	{
		printf("Error while sending SERVER_PLAYER_MOVE_REQUEST, closing thread.\n");
		closesocket(*t_socket);
		return STATUS_CODE_FAILURE;
	}
	return STATUS_CODE_SUCCESS;
}
static DWORD send_invite_and_setup_request(SOCKET* t_socket, char* client_id) {

	// This func send invite an then req move messages
	TransferResult_t SendRes;
	char SendStr[SEND_STR_SIZE];
	char invite[MAX_MESSAGE_TYPE_LEN + MAX_USERNAME_LEN + 2] = "SERVER_INVITE:";
	strcat(invite, client_id);
	strcat(invite, "\n");
	strcpy(SendStr, invite);
	SendRes = SendMsg(SendStr, *t_socket);
	if (SendRes == TRNS_FAILED)
	{
		printf("Error while sending SERVER_INVITE, closing thread.\n");
		closesocket(*t_socket);
		return STATUS_CODE_FAILURE;
	}
	strcpy(SendStr, "SERVER_SETUP_REQUSET\n"); //CHANGEDHERE strcpy(SendStr, "SERVER_PLAYER_MOVE_REQUEST\n")
	SendRes = SendMsg(SendStr, *t_socket);
	if (SendRes == TRNS_FAILED)
	{
		printf("Error while sending SERVER_SETUP_REQUSET, closing thread.\n"); 
		closesocket(*t_socket);
		return STATUS_CODE_FAILURE;
	}
	return STATUS_CODE_SUCCESS;
}
static DWORD send_sever_main_menu(SOCKET* t_socket) {
	TransferResult_t SendRes;
	char SendStr[SEND_STR_SIZE];
	strcpy(SendStr, "SERVER_MAIN_MENU\n");
	SendRes = SendMsg(SendStr, *t_socket);
	if (SendRes == TRNS_FAILED)
	{
		printf("Error while sending SERVER_MAIN_MENU, closing thread.\n");
		closesocket(*t_socket);
		return STATUS_CODE_FAILURE;
	}
	return STATUS_CODE_SUCCESS;
}
static DWORD send_approval(SOCKET *t_socket) {
	char SendStr[SEND_STR_SIZE];
	TransferResult_t SendRes;
	printf("we sent approval\n");
	strcpy(SendStr, "SERVER_APPROVED\n");
	
	SendRes = SendMsg(SendStr, *t_socket);
	if (SendRes == TRNS_FAILED)
	{
		printf("Error while sending SERVER_APPROVED, closing thread.\n");
		closesocket(*t_socket);
		return STATUS_CODE_FAILURE;
	}
	strcpy(SendStr, "SERVER_MAIN_MENU\n");
	SendRes = SendMsg(SendStr, *t_socket);
	if (SendRes == TRNS_FAILED)
	{
		printf("Error while sending SERVER_MAIN_MENU, closing thread.\n");
		closesocket(*t_socket);
		return STATUS_CODE_FAILURE;
	}
	return STATUS_CODE_SUCCESS;
}
static DWORD send_results_server_plays(SOCKET* t_socket, char op_move[MAX_MOVE_LEN], char player_number[MAX_MOVE_LEN], char oponent[MAX_USERNAME_LEN],char result[BULLS_COWS_STR_LEN], char username[MAX_USERNAME_LEN], int my_line_num) {
	//This func sends results message
	char opponent_move[MAX_MOVE_LEN];
	char this_player_number[MAX_MOVE_LEN];
	char bulls = result[0];
	char cows = result[1];
	TransferResult_t SendRes;
	for (int i = 0; i < MAX_MOVE_LEN; i++) {
		if (op_move[i] == '\n') {
			opponent_move[i] = '\0';
			break;
		}
		opponent_move[i] = op_move[i];
	}
	for (int i = 0; i < 10; i++) {
		if (player_number[i] == '\n') {
			this_player_number[i] = '\0';
			break;
		}
		this_player_number[i] = player_number[i];
	}

	char main_str[256] = "SERVER_GAME_RESULTS:";
	strcat(main_str, bulls);
	strcat(main_str, ";");
	strcat(main_str, cows);
	strcat(main_str, ";");
	strcat(main_str, oponent);
	strcat(main_str, ";");
	strcat(main_str, opponent_move);
	strcat(main_str, "\n");
	char SendStr[SEND_STR_SIZE];
	strcpy(SendStr, main_str);
	SendRes = SendMsg(SendStr, *t_socket);
	if (SendRes == TRNS_FAILED)
	{
		printf("Service socket error while writing, closing thread.\n");
		closesocket(*t_socket);
		return STATUS_CODE_FAILURE;
	}
	if (bulls == FOUR_BULLS)	win[my_line_num] = ONE_RET_VAL;
	else win[my_line_num] = ZERO_RET_VAL;

	return STATUS_CODE_SUCCESS;

}
static DWORD ServiceThread(SOCKET* t_socket)
{
	// this func manages messages for a specific client
	printf("thread made\n"); //DELETEME
	char SendStr[SEND_STR_SIZE];
	char RecvStr[SEND_STR_SIZE];
	char* AcceptedStr = NULL;
	char client_id[MAX_USERNAME_LEN];
	char op_id[MAX_USERNAME_LEN];
	char op_move[MAX_MOVE_LEN];
	char player_number[MAX_MOVE_LEN]; //ADDEDHERE
	char player_move[MAX_MOVE_LEN];
	BOOL i_created_file = FALSE;
	TransferResult_t SendRes;
	TransferResult_t RecvRes;

	while (1)
	{
		AcceptedStr = NULL;
		RecvRes = ReceiveMsg(&AcceptedStr, *t_socket);
		printf("%s\n", AcceptedStr); //DELETEME
		if (RecvRes == TRNS_FAILED)
		{
			printf("Service socket error while reading, closing thread.\n");
			closesocket(*t_socket);
			free(AcceptedStr);
			return STATUS_CODE_FAILURE;
		}
		else if (RecvRes == TRNS_DISCONNECTED)
		{
			printf("Connection closed while reading, closing thread.\n");
			closesocket(*t_socket);
			free(AcceptedStr);
			return STATUS_CODE_FAILURE;
		}

		if (is_client_request(AcceptedStr, client_id)) 
		{
			two_players_connected = FALSE;
			if (send_approval(t_socket) == STATUS_CODE_FAILURE) {
				free(AcceptedStr);
				return STATUS_CODE_FAILURE;
			}
		}	
		else if (is_client_versus(AcceptedStr))
		{
			int checking_res;
		play_versus:
			checking_res = cheacking_if_file_exits();
			if (checking_res == ZERO_RET_VAL) {
				printf("first one creating file \n ");
				// wait an event. if waited 15 sec and no one signaled event, send no_oponent
				// when event signaled, request moves from two clients.
				lines_num = 0;
				DWORD wait_res = WaitForSingleObject(versus_event, MAX_RESPONSE_WAITING_TIME);
				if (wait_res == WAIT_OBJECT_0) {
					// We have another player to play
					send_invite_and_setup_request(t_socket, client_id);
					There_are_two_players = ONE_RET_VAL;
					i_created_file = TRUE;
				}
				else if (wait_res == WAIT_TIMEOUT) {
					// Always alone, no one wants to play
					if (remove_file() == STATUS_CODE_FAILURE) return STATUS_CODE_FAILURE;
					if (send_no_opponet(t_socket) == STATUS_CODE_FAILURE) {
						free(AcceptedStr);
						return STATUS_CODE_FAILURE;
					}

				}
				else {
					printf("Waiting for versus event failed! Error is %d\n", GetLastError());
					free(AcceptedStr);
					return STATUS_CODE_FAILURE;
				}
			}
			else if (checking_res == 1)
			{
				// if yes: there is a player waiting to play. signal event
				// when event signaled, request moves from two clients.
				i_created_file = FALSE;
				There_are_two_players = ONE_RET_VAL;
				two_players_connected = TRUE;
				if (SetEvent(versus_event) == FALSE) {
					printf("cannot set a writing event for file!\n");
					return STATUS_CODE_FAILURE;
				}
				else {
					if (send_invite_and_setup_request(t_socket, client_id) == STATUS_CODE_FAILURE) {
						free(AcceptedStr);
						return STATUS_CODE_FAILURE;
					}

				}
			}
			else {
				printf("Error while checking file!\n");
				free(AcceptedStr);
				return STATUS_CODE_FAILURE;
			}
		}

		else if (is_client_setup(AcceptedStr, player_number))		//ADDEDHERE + CHANGEDHERE
		{
		if (send_player_move_request(t_socket, client_id) == STATUS_CODE_FAILURE) {
			free(AcceptedStr);
			return STATUS_CODE_FAILURE;
		}
		}

		else if (is_client_move(AcceptedStr, player_move))
		{
			int my_line_num;
			client_move:
			 if (There_are_two_players == ONE_RET_VAL)
			{
				my_line_num = write_move_to_file(player_move);
				if (my_line_num == 1) {
					strcpy_s(user2_name, MAX_USERNAME_LEN, client_id);
					if (SetEvent(finish_writing_event) == 0) {
						printf("cannot set a writing event for file!\n");
						free(AcceptedStr);
						return STATUS_CODE_FAILURE;
					}

					DWORD Wait_Res = WaitForSingleObject(oponent_name_event, INFINITE);
					if (handling_wait_code(Wait_Res) == STATUS_CODE_FAILURE) return (STATUS_CODE_FAILURE);
					printf("player_number before calculation :%s\n", player_number);
					reading_from_file_for_calacuation(op_move, player_number, my_line_num);
					char result[BULLS_COWS_STR_LEN];
					printf("1 we are calc cows and bulls\n");
					printf("player_number:%s\n", player_number);
					printf("op_move:%s\n", op_move);
					cows_and_bulls(player_number, op_move, result);
					printf("result:%s", result);
					send_results_server_plays(t_socket, op_move, player_number, user1_name, result, client_id, my_line_num);
					strcpy_s(op_id, MAX_USERNAME_LEN, user1_name);
					if (i_created_file) {
						if (remove_file() == STATUS_CODE_FAILURE) return STATUS_CODE_FAILURE;
					}
					printf("weeeee1\n");
				}
				
				else if (my_line_num == ZERO_RET_VAL) {
					strcpy_s(user1_name, MAX_USERNAME_LEN, client_id);
					if (SetEvent(oponent_name_event) == 0) {
						printf("cannot set a writing event for file!\n");
						free(AcceptedStr);
						return STATUS_CODE_FAILURE;
					}
					DWORD Wait_Res = WaitForSingleObject(finish_writing_event, INFINITE);
					if (handling_wait_code(Wait_Res) == STATUS_CODE_FAILURE) return (STATUS_CODE_FAILURE);

					reading_from_file_for_calacuation(op_move, player_number, my_line_num);
					char result[BULLS_COWS_STR_LEN];
					printf("0 we are calc cows and bulls\n");
					printf("player_number:%s\n", player_number);
					printf("op_move:%s\n", op_move);
					cows_and_bulls(player_number, op_move, result);
					printf("result:%s", result);
					send_results_server_plays(t_socket, op_move, player_number, user1_name, result, client_id, my_line_num);
					strcpy_s(op_id, MAX_USERNAME_LEN, user2_name);

					if (i_created_file) {
						if (remove_file() == STATUS_CODE_FAILURE) return STATUS_CODE_FAILURE;
					}
				printf("weeeee0\n");
				}
				else {
					printf("failed to write to file!\n");
					free(AcceptedStr);
					return STATUS_CODE_FAILURE;
				}
			}
			else {
				printf("Less than two clients in a game!\n");
				free(AcceptedStr);
				return STATUS_CODE_FAILURE;
			}
			while (win[0] == UNINITIALIZED || win[1] == UNINITIALIZED) { ; }
			if (win[0] == ZERO_RET_VAL && win[1] == ZERO_RET_VAL) // No one wins
			 {
				 if (send_player_move_request(t_socket, client_id) == STATUS_CODE_FAILURE) {
					 free(AcceptedStr);
					 return STATUS_CODE_FAILURE;
				 }
				 goto client_move;
			 }
			if (win[0] == ONE_RET_VAL && win[1] == ONE_RET_VAL)		//DRAW
			{
				if (send_server_draw(t_socket) == STATUS_CODE_FAILURE) {
					free(AcceptedStr);
					return STATUS_CODE_FAILURE;
				}
			}
			else if (win[my_line_num] == 1) // I won
			{
				if (send_server_win(t_socket, op_id, client_id) == STATUS_CODE_FAILURE) {
					free(AcceptedStr);
					return STATUS_CODE_FAILURE;
				}
			}
			else // Other Client Won
			{
				if (send_server_win(t_socket, op_id, client_id) == STATUS_CODE_FAILURE) {
					free(AcceptedStr);
					return STATUS_CODE_FAILURE;
				}
			}
			win[0] = UNINITIALIZED;
			win[1] = UNINITIALIZED;
		}

		else if (client_diconnection(AcceptedStr))
		{
			free(AcceptedStr);
			closesocket(*t_socket);
			return STATUS_CODE_SUCCESS;
		}
		free(AcceptedStr);
	}
	printf("Conversation ended.\n"); 
	closesocket(*t_socket);
	return STATUS_CODE_SUCCESS;
}
SOCKET* socket_alloc() {

	SOCKET* socket_ptr = (SOCKET*)malloc(sizeof(SOCKET));
	if (socket_ptr == NULL) 
	{
		printf("Error while allocating SOCKET!\n");
	}
	return socket_ptr;
}
static DWORD WaitForExit() {

	char in_str[5];

recheck:
	fgets(in_str, 5, stdin);
	if (STRINGS_ARE_EQUAL(in_str, "exit")) {
		Exit = TRUE; // set an event
	}
	else
		goto recheck;
}
int create_all_handles() {
	ExitHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)WaitForExit, NULL, 0, NULL);
	if (NULL == ExitHandle) {
		printf("Error: Couldn't create EXIT thread\n");
		return STATUS_CODE_FAILURE;
	}
	DWORD get_last_error = GetLastError();
	versus_event = CreateEvent(NULL, FALSE, FALSE, NULL);
	/*if the named event object existed before the function call, the
	function returns a handle to the existing object and GetLastError returns ERROR_ALREADY_EXISTS. */
	if (get_last_error == ERROR_ALREADY_EXISTS) {
		printf("Event already exists.\n");
		return STATUS_CODE_FAILURE;
	}
	else if (get_last_error != ERROR_SUCCESS) {
		printf("Error while creating thread, Error is %d\n", GetLastError());
		return STATUS_CODE_FAILURE;
	}
	finish_writing_event = CreateEvent(NULL, FALSE, FALSE, NULL);
	/*if the named event object existed before the function call, the
	function returns a handle to the existing object and GetLastError returns ERROR_ALREADY_EXISTS. */
	if (get_last_error == ERROR_ALREADY_EXISTS) {
		printf("Event already exists.\n");
		return STATUS_CODE_FAILURE;
	}
	else if (get_last_error != ERROR_SUCCESS) {
		printf("Error while creating thread, Error is %d\n", GetLastError());
		return STATUS_CODE_FAILURE;
	}
	oponent_name_event = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (get_last_error == ERROR_ALREADY_EXISTS) {
		printf("Event already exists.\n");
		return STATUS_CODE_FAILURE;
	}
	else if (get_last_error != ERROR_SUCCESS) {
		printf("Error while creating thread, Error is %d\n", GetLastError());
		return STATUS_CODE_FAILURE;
	}
	return STATUS_CODE_SUCCESS;
}
static DWORD DeniedThread(SOCKET* t_socket)
{
	// This func sends denied message when recieved a client request
	char* AcceptedStr;
	char  client_id[MAX_USERNAME_LEN];


	TransferResult_t SendRes;
	TransferResult_t RecvRes;

	while (1)
	{
		AcceptedStr = NULL;
		RecvRes = ReceiveMsg(&AcceptedStr, *t_socket);
		if (RecvRes == TRNS_FAILED)
		{
			printf("Service socket error while reading, closing thread.\n");
			closesocket(*t_socket);
			return STATUS_CODE_FAILURE;
		}
		else if (RecvRes == TRNS_DISCONNECTED)
		{
			printf("Connection closed while reading, closing thread.\n");
			closesocket(*t_socket);
			return STATUS_CODE_FAILURE;
		}

		if (is_client_request(AcceptedStr, client_id))
		{
			SEND_MSG_ARGS_params_t send_msg_args;
			strcpy_s(send_msg_args.message_type, MAX_MESSAGE_TYPE_LEN, "SERVER_DENIED");
			strcpy_s(send_msg_args.message_args, MAX_USERNAME_LEN + MAX_MOVE_LEN, "No Available slots in server.");
			if (send_msg(send_msg_args, *t_socket) != STATUS_CODE_SUCCESS) {
				closesocket(*t_socket);
				return STATUS_CODE_FAILURE;
			}
			closesocket(*t_socket);
			return STATUS_CODE_SUCCESS;
		}
		else
		{
			printf("Unexpected message! waiting for client request.\n");
		}
		free(AcceptedStr);
	}
	closesocket(*t_socket);
	free(t_socket);

}


int Main_server(char* port_str)
{
	int Ind;
	int Loop;
	SOCKET MainSocket = INVALID_SOCKET;
	unsigned long Address;
	SOCKADDR_IN service;
	int bindRes;
	int ListenRes;

	// Initialize Winsock.
	WSADATA wsaData;
	int StartupRes = WSAStartup(MAKEWORD(2, 2), &wsaData);

	if (StartupRes != NO_ERROR)
	{
		printf("error %ld at WSAStartup( ), ending program.\n", WSAGetLastError());
		// Tell the user that we could not find a usable WinSock DLL.                                  
		return;
	}

	/* The WinSock DLL is acceptable. Proceed. */

	// Create a socket.    
	MainSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (MainSocket == INVALID_SOCKET)
	{
		printf("Error at socket( ): %ld\n", WSAGetLastError());
		goto server_cleanup_1;
	}

	// Create a sockaddr_in object and set its values.
		// Declare variables

	Address = inet_addr(SERVER_ADDRESS_STR);
	if (Address == INADDR_NONE)
	{
		printf("The string \"%s\" cannot be converted into an ip address. ending program.\n",
			SERVER_ADDRESS_STR);
		goto server_cleanup_2;
	}

	service.sin_family = AF_INET;
	service.sin_addr.s_addr = Address;
	service.sin_port = htons((unsigned short)atoi(port_str)); //The htons function converts a u_short from host to TCP/IP network byte order 
									   //( which is big-endian ).
	/*
			The three lines following the declaration of sockaddr_in service are used to set up
			the sockaddr structure:
			AF_INET is the Internet address family.
			"127.0.0.1" is the local IP address to which the socket will be bound.
			2345 is the port number to which the socket will be bound.
		*/

		// Call the bind function, passing the created socket and the sockaddr_in structure as parameters. 
		// Check for general errors.

	bindRes = bind(MainSocket, (SOCKADDR*)&service, sizeof(service));
	if (bindRes == SOCKET_ERROR)
	{
		printf("bind( ) failed with error %ld. Ending program\n", WSAGetLastError());
		goto server_cleanup_2;
	}

	// Listen on the Socket.
	ListenRes = listen(MainSocket, SOMAXCONN);
	if (ListenRes == SOCKET_ERROR)
	{
		printf("Failed listening on socket, error %ld.\n", WSAGetLastError());
		goto server_cleanup_2;
	}

	// Initialize Hanldes' array
	g_num_of_hanldes = NUM_OF_WORKER_THREADS;
	g_workers_handles = (HANDLE*)malloc(NUM_OF_WORKER_THREADS * sizeof(HANDLE));
	if (g_workers_handles == NULL)
	{
		printf("Failed to Allocate memory!\n");
		goto server_cleanup_2;
	}

	// Initialize all thread handles to NULL, to mark that they have not been initialized
	for (Ind = 0; Ind < g_num_of_hanldes; Ind++)
		g_workers_handles[Ind] = NULL;

	//calling thread checking if wrriten exit
	if (create_all_handles() == STATUS_CODE_FAILURE) goto server_cleanup_2;
	if (CreateFileMutex() == STATUS_CODE_FAILURE) goto server_cleanup_2;
	if (CreateFileMutexCreatFile() == STATUS_CODE_FAILURE) goto server_cleanup_2;
	printf("Waiting for a client to connect...\n");
	int nReady;
	int biggets = 0;
	fd_set sfds;
	struct timeval timeout = { 0, 0 };

	FD_ZERO(&sfds);
	FD_SET(MainSocket, &sfds);

	timeout.tv_sec = 0;		//Time interval, in seconds.
	timeout.tv_usec = 1000; //Time interval, in microseconds.
accept_again:
	FD_SET(MainSocket, &sfds);
	if ((nReady = select(0, //Ignored. The nfds parameter is included only for compatibility with Berkeley sockets.
		&sfds,				//An optional pointer to a set of sockets to be checked for readability.
		NULL,				//An optional pointer to a set of sockets to be checked for writability.
		NULL,				//An optional pointer to a set of sockets to be checked for errors.
		&timeout			//The maximum time for select to wait, provided in the form of a TIMEVAL structure. 
	)) > 0)	//- Set the timeout parameter to null for blocking operations.
	{
		SOCKET AcceptSocket = accept(MainSocket, NULL, NULL);

		if (AcceptSocket == INVALID_SOCKET)
		{
			printf("INVALID SOCKET, Ending server.\n");
			closesocket(AcceptSocket);
			goto server_cleanup_3;
		}

		Ind = FindFirstUnusedThreadSlot(); // in a good server clients connect and disconnect all the time,this function makes 
										   // sure to find a place in our thread's array to start a thread and remove the disconnected thread
		SOCKET* socket_ptr = socket_alloc(); //SEARCHME : maybe we need to clean_up
		*socket_ptr = AcceptSocket;

		if (Ind >= NUM_OF_WORKER_THREADS)
		{
			if ((Ind >= g_num_of_hanldes)) {
				// allocating more memory
				g_workers_handles = (HANDLE*)realloc(g_workers_handles, (g_num_of_hanldes + 1) * sizeof(HANDLE));
				if (g_workers_handles == NULL)
				{
					printf("Error  allocating memory!\n");
				}
				g_workers_handles[g_num_of_hanldes] = NULL; // Init new handle
				g_num_of_hanldes++;
			}
			printf("%d", Ind); //DELETEME
			printf("No slots available for client, dening request.\n");
			g_workers_handles[Ind] = CreateThread(
				NULL,
				0,
				(LPTHREAD_START_ROUTINE)DeniedThread,
				socket_ptr,
				0,
				NULL
			);

		}
		else {
			printf("%d", Ind); //DELETEME
			g_workers_handles[Ind] = CreateThread(
				NULL,
				0,
				(LPTHREAD_START_ROUTINE)ServiceThread,
				socket_ptr,
				0,
				NULL
			);
		}
		goto accept_again;
	}
	else if (nReady == SOCKET_ERROR) {
		printf("Error in select(): %d\n", WSAGetLastError());
		goto server_cleanup_3;
	}
	else {
		if (WaitForSingleObject(ExitHandle, 0) == WAIT_OBJECT_0) goto server_cleanup_3; // Polling to check if got exit
		goto accept_again; // continue checking
	}
server_cleanup_3:
	CleanupWorkerThreads();
	close_all_handles();
	free(g_workers_handles);
server_cleanup_2:
	if (closesocket(MainSocket) == SOCKET_ERROR)
		printf("Failed to close MainSocket, error %ld. Ending program\n", WSAGetLastError());

server_cleanup_1:
	if (WSACleanup() == SOCKET_ERROR)
		printf("Failed to close Winsocket, error %ld. Ending program.\n", WSAGetLastError());
}
int CreateFileMutex(void)
{//creating mutex which will be responsible of writing to file
	MutexHandleForFile = CreateMutexA(

		NULL,
		FALSE,
		NULL
	);

	if (NULL == MutexHandleForFile)
	{
		printf("Error while creating mutex: %d\n", GetLastError());
		return STATUS_CODE_FAILURE;
	}
	return STATUS_CODE_SUCCESS;
}
int CreateFileMutexCreatFile(void)
{//creating mutex which will be responsible of creating to file for once
	MutexHandleForCreatingFile = CreateMutex(
		NULL,
		FALSE,
		NULL);

	if (NULL == MutexHandleForCreatingFile)
	{
		printf("Error while creating mutex: %d\n", GetLastError());
		return STATUS_CODE_FAILURE;
	}
	return STATUS_CODE_SUCCESS;
}
static int FindFirstUnusedThreadSlot()
{
	int Ind;

	for (Ind = 0; Ind < g_num_of_hanldes; Ind++)
	{
		if (g_workers_handles[Ind] == NULL)
			break;
		else
		{
			// poll to check if thread finished running:
			DWORD Res = WaitForSingleObject(g_workers_handles[Ind], 0);
			//SEARCHME : we did not check for all the options here (last recitation 30:00) 
			if (Res == WAIT_OBJECT_0) // this thread finished running
			{
				CloseHandle(g_workers_handles[Ind]);
				g_workers_handles[Ind] = NULL;
				break;
			}
		}
	}

	return Ind;
}