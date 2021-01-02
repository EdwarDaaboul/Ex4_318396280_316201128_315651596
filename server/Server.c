#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
//SEARCHME
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <io.h>
#include <errno.h>

#include "SocketExampleShared.h"
#include "SocketSendRecvTools.h"
#include "server.h"
//#include "FileHandlers.h"
#include "MessageDecoding.h"



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
/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

//HANDLE* dyn_handles;
//int num_of_hanldes;
//HANDLE ExitHandle = NULL;
//HANDLE versus_event = NULL;
//HANDLE MutexHandleForFile = NULL;
//HANDLE finish_writing_event = NULL;
//HANDLE  oponent_name_event = NULL;
BOOL two_players_connected = FALSE;
BOOL Exit = FALSE;
static int decode_number;
char* GameSessionFile = "GameSession.txt";
int last_played = LAST_PLAYER;
int lines_num = 0;
HANDLE MutexHandleForCreatingFile = NULL;
char user1_name[MAX_USERNAME_LEN];
char user2_name[MAX_USERNAME_LEN];

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/
/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

static int FindFirstUnusedThreadSlot();
//static void CleanupWorkerThreads();
static DWORD ServiceThread(SOCKET* t_socket);
//static DWORD DeniedThread(SOCKET* t_socket);
//SOCKET MainSocket;
/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/
static DWORD send_approval(SOCKET *t_socket) {
	char SendStr[SEND_STR_SIZE];
	TransferResult_t SendRes;
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
static DWORD ServiceThread(SOCKET* t_socket)
{
	// this func manages messages for a specific client
	char SendStr[SEND_STR_SIZE];
	char RecvStr[SEND_STR_SIZE];
	char* AcceptedStr = NULL;
	char client_id[MAX_USERNAME_LEN];
	char op_id[MAX_USERNAME_LEN];
	char op_move[15];
	char player_move[15];
	BOOL i_created_file = FALSE;
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


		//CHANGEME XStartXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
		else if (is_client_move(AcceptedStr, player_move))
		{
			if (last_played == 0) {
				int winner = decide_who_wins(op_move, player_move);
				if (send_results_server_plays(t_socket, op_move, player_move, "Server", winner, client_id) == STATUS_CODE_FAILURE) {
					free(AcceptedStr);
					return STATUS_CODE_FAILURE;
				}
			}
			else if (last_played == 1)
			{
				int my_line_num = write_move_to_file(player_move);
				if (my_line_num == 1) {
					strcpy_s(user2_name, MAX_USERNAME_LEN, client_id);
					if (SetEvent(finish_writing_event) == 0) {
						printf("cannot set a writing event for file!\n");
						free(AcceptedStr);
						return STATUS_CODE_FAILURE;
					}

					DWORD Wait_Res = WaitForSingleObject(oponent_name_event, INFINITE);
					if (handling_wait_code(Wait_Res) == STATUS_CODE_FAILURE) return (STATUS_CODE_FAILURE);
					reading_from_file_for_calacuation(op_move, player_move, my_line_num);
					int winner = decide_who_wins(op_move, player_move);
					send_results_server_plays(t_socket, op_move, player_move, user1_name, winner, client_id);
					strcpy_s(op_id, MAX_USERNAME_LEN, user1_name);
					if (i_created_file) {
						if (remove_file() == STATUS_CODE_FAILURE) return STATUS_CODE_FAILURE;
					}

				}
				else if (my_line_num == 0) {
					strcpy_s(user1_name, MAX_USERNAME_LEN, client_id);
					if (SetEvent(oponent_name_event) == 0) {
						printf("cannot set a writing event for file!\n");
						free(AcceptedStr);
						return STATUS_CODE_FAILURE;
					}
					DWORD Wait_Res = WaitForSingleObject(finish_writing_event, INFINITE);
					if (handling_wait_code(Wait_Res) == STATUS_CODE_FAILURE) return (STATUS_CODE_FAILURE);

					reading_from_file_for_calacuation(op_move, player_move, my_line_num);
					int winner = decide_who_wins(op_move, player_move);
					send_results_server_plays(t_socket, op_move, player_move, user2_name, winner, client_id);
					strcpy_s(op_id, MAX_USERNAME_LEN, user2_name);

					if (i_created_file) {
						if (remove_file() == STATUS_CODE_FAILURE) return STATUS_CODE_FAILURE;
					}
				}
				else {
					printf("failed to write to file!\n");
					free(AcceptedStr);
					return STATUS_CODE_FAILURE;
				}
			}
			else {
				printf("No more than two clients in a game!\n");
				free(AcceptedStr);
				return STATUS_CODE_FAILURE;
			}
			char SendStr[255];
			if (send_game_over_menu(t_socket) == STATUS_CODE_FAILURE) {
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
				// wait an event. if waited 15 sec and no one signaled event, send no_oponent
				// when event signaled, request moves from two clients.
				lines_num = 0;
				DWORD wait_res = WaitForSingleObject(versus_event, MAX_RESPONSE_WAITING_TIME);
				if (wait_res == WAIT_OBJECT_0) {
					// We have another player to play
					send_invite_and_request_moves(t_socket, client_id);
					last_played = ONE_RET_VAL;
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
				last_played = ONE_RET_VAL;
				two_players_connected = TRUE;
				if (SetEvent(versus_event) == FALSE) {
					printf("cannot set a writing event for file!\n");
					return STATUS_CODE_FAILURE;
				}
				else {
					if (send_invite_and_request_moves(t_socket, client_id) == STATUS_CODE_FAILURE) {
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
		//CHANGEME XENDXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX


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
	if (socket_ptr == NULL) //SEARCHME : we need to deal with this problem; i.e. to cleanup
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
		SOCKET* socket_ptr = socket_alloc();
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