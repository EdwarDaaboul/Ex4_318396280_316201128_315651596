/*
# Description:
#	In this file we have defined the functions related to dealing
#	text messages for communication.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <time.h>
#include <io.h>
#include <errno.h>

#include "SocketExampleShared.h"
#include "SocketSendRecvTools.h"
#include "server.h"
//#include "FileHandlers.h"
#include "MessageDecoding.h"


/*for message with type CLIENT_REQUEST*/
BOOL is_client_request(char* in_msg, char client_id[MAX_USERNAME_LEN])
{
	char* client_req = "CLIENT_REQUEST";
	for (int i = 0; i < strlen(client_req); i++) {
		if (in_msg[i] == client_req[i]) {
			continue;
		}
		return FALSE;
	}
	int start_idx = strlen(client_req) + 1;
	for (int i = 0; i < MAX_USERNAME_LEN; i++) {
		if (in_msg[start_idx + i] == '\n') {
			client_id[i] = '\0';
			break;
		}
		client_id[i] = in_msg[start_idx + i];
	}
	return TRUE;
}


/*for message with type CLIENT_PLAYER_MOVE*/
BOOL is_client_move(char* in_msg, char player_move[15]) {
	char* client_move = "CLIENT_PLAYER_MOVE";
	for (int i = 0; i < strlen(client_move); i++) {
		if (in_msg[i] == client_move[i]) {
			continue;
		}
		return FALSE;
	}
	int start_idx = strlen(client_move) + 1;
	for (int i = 0; i < MAX_USERNAME_LEN; i++) {
		if (in_msg[start_idx + i] == '\n') {
			player_move[i] = '\0';
			break;
		}
		player_move[i] = in_msg[start_idx + i];
	}

	return TRUE;
}
/*for message with type CLIENT_MAIN_MENU*/
BOOL is_client_main_menu(char* in_msg) {
	char* client_main_menu = "CLIENT_MAIN_MENU";
	for (int i = 0; i < strlen(client_main_menu); i++) {
		if (in_msg[i] == client_main_menu[i])
		{
			continue;
		}
		return FALSE;
	}
	return TRUE;
}
/*for message with type CLIENT_QUIT*/
BOOL client_diconnection(char* in_msg) {
	char* client_disconnection = "CLIENT_DISCONNECT";
	for (int i = 0; i < strlen(client_disconnection); i++) {
		if (in_msg[i] == client_disconnection[i])
		{
			continue;
		}
		return FALSE;
	}
	return TRUE;
}
/*for message with type CLIENT_VERSUS*/
BOOL is_client_versus(char* in_msg) {
	char* client_versus = "CLIENT_VERSUS";
	for (int i = 0; i < strlen(client_versus); i++) {
		if (in_msg[i] == client_versus[i])
		{
			continue;
		}
		return FALSE;
	}
	return TRUE;
}
/*for message with type CLIENT_REPLAY*/
BOOL is_client_replay(char* in_msg) {
	char* client_replay = "CLIENT_REPLAY";
	for (int i = 0; i < strlen(client_replay); i++) {
		if (in_msg[i] == client_replay[i])
		{
			continue;
		}
		return FALSE;
	}
	return TRUE;
}


