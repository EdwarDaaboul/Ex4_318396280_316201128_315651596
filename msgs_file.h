#ifndef MENU_HEADER
#define MENU_HEADER
#include <stdbool.h>

int print_fail_menu(void);
int print_main_menu(void);
int print_gameover_menu(void);

bool is_results_msg(char* in_msg);
bool is_winner_msg(char* in_msg);
bool is_draw_msg(char* in_msg);
int main_menu_client_req(char* str_to_free);
int free_and_ret(char* str_to_free, int ret_status);
int got_approval(char* str_to_free);
int got_denied(char* str_to_free);
int got_main_menu(char* str_to_free);
int game_over_menu(char* str_to_free);
int requested_move(char* str_to_free);
int stup_move(char* str_to_free);
bool is_invite(char* in_msg);
bool is_denied(char* in_msg);

#endif