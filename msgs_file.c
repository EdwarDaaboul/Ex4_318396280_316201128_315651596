#include "SocketExampleClient.h"
#include "SocketExampleShared.h"
#include "SocketSendRecvTools.h"


// extern global variables
extern SOCKET m_socket;
extern BOOL server_approved;
extern BOOL server_denied;
extern BOOL req_move;
extern BOOL setup_move;
extern BOOL done;
extern HANDLE recvd_msg_event;
extern HANDLE recieving_thread;
extern char op_name[MAX_USERNAME_LEN];
extern int waiting_status;

// Print menus and got a choice
//print num 3 in client behavior
int print_fail_menu(void) {
	waiting_status = NO_WAIT;
	printf("Choose what to do next:\n");
	printf("	1. Try to reconnect\n");
	printf("	2. Exit\n");
	char SendStr[2];

	while (1) {
		gets_s(SendStr, sizeof(SendStr)); //Reading a string from the keyboard

		if (STRINGS_ARE_EQUAL(SendStr, "2"))
			return 2; // User shose 2 option
		else if (STRINGS_ARE_EQUAL(SendStr, "1"))
			return 1; // User shose 1 option
		else
			printf("Please choose a valid option!\n");
	}

}
int print_main_menu(void) {
	waiting_status = NO_WAIT;
	printf("Choose what to do next:\n");
	printf("	1. Play against another client\n");
	printf("	2. Quit\n");
	char SendStr[2];
	while (1) {
		gets_s(SendStr, sizeof(SendStr)); //Reading a string from the keyboard

		if (STRINGS_ARE_EQUAL(SendStr, "2"))
			return 2; // User shose 2 option
		else if (STRINGS_ARE_EQUAL(SendStr, "1"))
			return 1; // User shose 1 option
		else
			printf("Please choose a valid option!\n");
	}

}



// Check message types and dealing with it
//results
BOOL is_results_msg(char* in_msg) {
	char* results_msg = "SERVER_GAME_RESULTS";
	for (int i = 0; i < strlen(results_msg); i++) {
		if (in_msg[i] == results_msg[i]) {
			continue;
		}
		return FALSE;
	}
	char username[MAX_USERNAME_LEN];
	char op_move[MAX_MOVE_LEN];
	char bulls[LEN_BULLS_OR_COWS];
	char cows[LEN_BULLS_OR_COWS];
	int start_arg_idx = strlen(results_msg) + 1;
	int offset_idx = 0;


	while (offset_idx < LEN_BULLS_OR_COWS) {
		if (in_msg[start_arg_idx + offset_idx] == '\n') { // msg ends early
			printf("Error: Got results with wrong arguments\n");
			return FALSE;
		}
		if (in_msg[start_arg_idx + offset_idx] == ';') break; // Pass to next argument
		bulls[offset_idx] = in_msg[start_arg_idx + offset_idx];
		offset_idx++;
	}
	bulls[offset_idx] = '\0';
	start_arg_idx = start_arg_idx + offset_idx + 1;
	offset_idx = 0;

	while (offset_idx < LEN_BULLS_OR_COWS) {
		if (in_msg[start_arg_idx + offset_idx] == '\n') { // msg ends early
			printf("Error: Got results with wrong arguments\n");
			return FALSE;
		}
		if (in_msg[start_arg_idx + offset_idx] == ';') break; // Pass to next argument
		cows[offset_idx] = in_msg[start_arg_idx + offset_idx];
		offset_idx++;
	}
	cows[offset_idx] = '\0';
	start_arg_idx = start_arg_idx + offset_idx + 1;
	offset_idx = 0;

	while (offset_idx < MAX_USERNAME_LEN) {
		if (in_msg[start_arg_idx + offset_idx] == '\n') { // msg ends early
			printf("Error: Got results with wrong arguments\n");
			return FALSE;
		}
		if (in_msg[start_arg_idx + offset_idx] == ';') break; // Pass to next argument
		username[offset_idx] = in_msg[start_arg_idx + offset_idx];
		offset_idx++;
	}
	username[offset_idx] = '\0';
	start_arg_idx = start_arg_idx + offset_idx + 1;
	offset_idx = 0;

	while (offset_idx < MAX_MOVE_LEN) {
		if (in_msg[start_arg_idx + offset_idx] == '\n') { // msg ends early
			printf("Error: Got results with wrong arguments\n");
			return FALSE;
		}
		if (in_msg[start_arg_idx + offset_idx] == ';') break; // Pass to next argument
		op_move[offset_idx] = in_msg[start_arg_idx + offset_idx];
		offset_idx++;
	}
	op_move[offset_idx] = '\0';
	start_arg_idx = start_arg_idx + offset_idx + 1;
	offset_idx = 0;




	printf("Bulls: %s\n", bulls);
	printf("Cows: %s\n", cows);
	printf("%s played: %s\n", username, op_move);

	return TRUE;
}


BOOL is_winner_msg(char* in_msg) {
	char* results_msg = "WIN_SERVER";
	for (int i = 0; i < strlen(results_msg); i++) {
		if (in_msg[i] == results_msg[i]) {
			continue;
		}
		return FALSE;
	}
	char winner[MAX_USERNAME_LEN];
	char op_number[MAX_MOVE_LEN];
	int start_arg_idx = strlen(results_msg) + 1;
	int offset_idx = 0;

	while (offset_idx < MAX_USERNAME_LEN) {
		if (in_msg[start_arg_idx + offset_idx] == '\n') { // msg ends early
			printf("Error: Got results with wrong arguments\n");
			return FALSE;
		}
		if (in_msg[start_arg_idx + offset_idx] == ';') break; // Pass to next argument
		winner[offset_idx] = in_msg[start_arg_idx + offset_idx];
		offset_idx++;
	}
	winner[offset_idx] = '\0';
	start_arg_idx = start_arg_idx + offset_idx + 1;
	offset_idx = 0;

	while (offset_idx < MAX_MOVE_LEN) {
		if (in_msg[start_arg_idx + offset_idx] == '\n') { // msg ends early
			printf("Error: Got results with wrong arguments\n");
			return FALSE;
		}
		if (in_msg[start_arg_idx + offset_idx] == ';') break; // Pass to next argument
		op_number[offset_idx] = in_msg[start_arg_idx + offset_idx];
		offset_idx++;
	}
	op_number[offset_idx] = '\0';
	start_arg_idx = start_arg_idx + offset_idx + 1;
	offset_idx = 0;
	

	printf("%s won!\n", winner);
	printf("opponents number was %s\n", op_number);


	return TRUE;
}
BOOL is_draw_msg(char* in_msg) {
	char* results_msg = "DRAW_SERVER";
	for (int i = 0; i < strlen(results_msg); i++) {
		if (in_msg[i] == results_msg[i]) {
			continue;
		}
		return FALSE;
	}
	printf("it's a tie\n");
	return TRUE;
}




int free_and_ret(char* str_to_free, int ret_status) {
	free(str_to_free);
	return ret_status;
}

//update approved status
int got_approval(char* str_to_free) {
	server_approved = TRUE;
	return STATUS_CODE_SUCCESS;
}


//SERVER DENIED status
int got_denied(char* str_to_free) {
	server_denied = TRUE;
	return STATUS_CODE_SUCCESS;
}
BOOL is_denied(char* in_msg) {
	char* deny_msg = "SERVER_DENIED";
	for (int i = 0; i < strlen(deny_msg); i++) {
		if (in_msg[i] == deny_msg[i]) {
			continue;
		}
		return FALSE;
	}
	int start_idx = strlen(deny_msg) + 1;
	for (int i = 0; i < MAX_USERNAME_LEN; i++) {

		printf("%c", deny_msg[i + start_idx]);
		if (deny_msg[i + start_idx] == '\n') break;

	}
	return TRUE;

}

//main menu status so print it
int got_main_menu(char* str_to_free) {

	SEND_MSG_ARGS_params_t send_msg_args;
	int choice = print_main_menu();
	switch (choice) {
	case 1:
		// play with other user
		strcpy_s(send_msg_args.message_type, MAX_MESSAGE_TYPE_LEN, "CLIENT_VERSUS");
		strcpy_s(send_msg_args.message_args, MAX_USERNAME_LEN, " ");
		if (send_msg(send_msg_args) != STATUS_CODE_SUCCESS) return STATUS_CODE_FAILURE;
		waiting_status = WAIT_15;
		break;
	case 2:
		// quit
		strcpy_s(send_msg_args.message_type, MAX_MESSAGE_TYPE_LEN, "CLIENT_DISCONNECT");
		strcpy_s(send_msg_args.message_args, MAX_USERNAME_LEN, " ");
		if (send_msg(send_msg_args) != STATUS_CODE_SUCCESS) return STATUS_CODE_FAILURE;
		done = TRUE;
		break;
	}
	return STATUS_CODE_SUCCESS;
}

//request move
int requested_move(char* str_to_free) {
	waiting_status = NO_WAIT;
	printf("Choose your guess:\n");
	char SendStr[5];
	while (1) {
		gets_s(SendStr, sizeof(SendStr)); //Reading a string from the keyboard
		// send move to server
		SEND_MSG_ARGS_params_t send_msg_args;
		strcpy_s(send_msg_args.message_type, MAX_MESSAGE_TYPE_LEN, "CLIENT_PLAYER_MOVE");
		strcpy_s(send_msg_args.message_args, MAX_USERNAME_LEN, SendStr);
		if (send_msg(send_msg_args) == STATUS_CODE_FAILURE) return STATUS_CODE_FAILURE;
		break;	
	}

	req_move = TRUE;
	return STATUS_CODE_SUCCESS;
}

//setup move
int stup_move(char* str_to_free) {
	waiting_status = NO_WAIT;
	printf("Choose your 4 digits:\n");
	char SendStr[5];
	while (1) {
		BOOL legal_move = FALSE;
		gets_s(SendStr, sizeof(SendStr)); //Reading a string from the keyboard
		if ((SendStr[0] != SendStr[1]) && (SendStr[0] != SendStr[2]) && (SendStr[0] != SendStr[3]) && (SendStr[1] != SendStr[2]) && (SendStr[1] != SendStr[2]) && (SendStr[2] != SendStr[3])) legal_move = TRUE;
		if (legal_move) {
			// send move to server
			SEND_MSG_ARGS_params_t send_msg_args;
			strcpy_s(send_msg_args.message_type, MAX_MESSAGE_TYPE_LEN, "CLIENT_SETUP");
			strcpy_s(send_msg_args.message_args, MAX_USERNAME_LEN, SendStr);
			if (send_msg(send_msg_args) == STATUS_CODE_FAILURE) return STATUS_CODE_FAILURE;
			break;
		}
		else {
			printf("not a legal move\n");
			return STATUS_CODE_FAILURE;
		}
	}
	setup_move = TRUE;
	return STATUS_CODE_SUCCESS;
}

//invite
BOOL is_invite(char* in_msg) {
	char* invite_msg = "SERVER_INVITE";
	for (int i = 0; i < strlen(invite_msg); i++) {
		if (in_msg[i] == invite_msg[i]) {
			continue;
		}
		return FALSE;
	}
	int start_idx = strlen(invite_msg) + 1;
	for (int i = 0; i < MAX_USERNAME_LEN; i++) {
		if (invite_msg[i + start_idx] == '\n') {
			op_name[i] = '\0';
			break;
		}
		op_name[i] = invite_msg[i + start_idx];
	}
	return TRUE;
}

int main_menu_client_req(char* str_to_free) {

	SEND_MSG_ARGS_params_t send_msg_args;
	strcpy_s(send_msg_args.message_type, MAX_MESSAGE_TYPE_LEN, "CLIENT_MAIN_MENU");
	strcpy_s(send_msg_args.message_args, MAX_USERNAME_LEN, " ");
	if (send_msg(send_msg_args) != STATUS_CODE_SUCCESS) return STATUS_CODE_FAILURE;
	return STATUS_CODE_SUCCESS;
}

