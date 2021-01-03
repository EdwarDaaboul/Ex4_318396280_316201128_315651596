/*
# FILE Description:
# In this header file we have declared our functions that is
# related to handling files
*/

#ifndef __MESSAGE_DEC__
#define __MESSAGE_DEC__

// All functions here returns (FALSE) if failed and (TRUE) if succed


/*for message with type CLIENT_REQUEST*/
BOOL is_client_request(char* in_msg, char client_id[MAX_USERNAME_LEN]);
/*for message with type CLIENT_CPU*/
BOOL is_client_cpu(char* in_msg);
/*for message with type CLIENT_PLAYER_MOVE*/
BOOL is_client_move(char* in_msg, char player_move[15]);
/*for message with type CLIENT_MAIN_MENU*/
BOOL is_client_main_menu(char* in_msg);
/*for message with type CLIENT_QUIT*/
BOOL client_diconnection(char* in_msg);
/*for message with type CLIENT_VERSUS*/
BOOL is_client_versus(char* in_msg);
/*for message with type CLIENT_REPLAY*/
BOOL is_client_replay(char* in_msg);
/*for message with type CLIENT_SETUP*/
BOOL is_client_setup(char* in_msg, char player_game_numbers[15]);
#endif 