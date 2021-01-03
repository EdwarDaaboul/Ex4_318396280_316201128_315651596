/*
# Description:
#	In this file we have defined the functions related to dealing
#	with txt files.
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
#include "FileHandlers.h"


extern char* GameSessionFile;
extern HANDLE MutexHandleForCreatingFile;
extern HANDLE MutexHandleForFile;
extern int lines_num;

/*dealing with files*/

/*this function opens file for writing moves*/
int open_file_for_first_time()
{
	// Open the file in the write mode
	FILE* file_p;
	errno_t resval = fopen_s(&file_p, GameSessionFile, "a");
	if (resval != 0)
	{
		printf("Failed to open %s file for appending!\n", GameSessionFile);
		return STATUS_CODE_FAILURE;
	}
	if (close_file(file_p) == STATUS_CODE_FAILURE) return STATUS_CODE_FAILURE;
	return STATUS_CODE_SUCCESS;

}
/*this file checks if file do exist and no need to open once more*/
int cheacking_if_file_exits(void) {

	errno_t exist;
	int ret_val;
	DWORD Wait_Res = WaitForSingleObject(MutexHandleForCreatingFile, INFINITE);
	if (handling_wait_code(Wait_Res) == STATUS_CODE_FAILURE) return (STATUS_CODE_FAILURE);
	// Check for existence.
	if ((exist = _access_s(GameSessionFile, 0)) != 0)
	{
		open_file_for_first_time();
		ret_val = ZERO_RET_VAL;

	}
	else ret_val = ONE_RET_VAL;

	if (ReleaseMutex(MutexHandleForCreatingFile) == FALSE) return STATUS_CODE_FAILURE;
	return ret_val;
}
/*this function closes file*/
int close_file(FILE* pfile) {

	int retval = -1;
	retval = fclose(pfile);
	if (0 != retval)
	{
		printf("Failed to close file!\n");
		return STATUS_CODE_FAILURE;
	}
	else {
		return STATUS_CODE_SUCCESS;
	}
}
/*this function writes moves into file*/
int write_move_to_file(char* player_move)
{
	DWORD WaitRes = WaitForSingleObject(MutexHandleForFile, INFINITE);
	if (handling_wait_code(WaitRes) == STATUS_CODE_FAILURE) return (STATUS_CODE_FAILURE);

	FILE* file_p;
	errno_t resval = fopen_s(&file_p, GameSessionFile, "a");
	if (resval != 0)
	{
		printf("Failed to open %s file for appending!\n", GameSessionFile);
		return STATUS_CODE_FAILURE;
	}
	fprintf(file_p, player_move);
	fprintf(file_p, "\n");
	lines_num++;
	if (close_file(file_p) == STATUS_CODE_FAILURE) {
		if (ReleaseMutex(MutexHandleForFile) == FALSE) return (STATUS_CODE_FAILURE);
		return STATUS_CODE_FAILURE;
	}
	else {
		if (ReleaseMutex(MutexHandleForFile) == FALSE) return (STATUS_CODE_FAILURE);
		return lines_num - 1;
	}
}
/*this function reads moves from file for calculation*/
int reading_from_file_for_calacuation(char op_move[15], char player_move[15], int my_line_num) {
	errno_t retval;
	// Open the file in the read mode
	FILE* file_p;
	char line1[MAX_MOVE_LEN];
	char line2[MAX_MOVE_LEN];

	retval = fopen_s(&file_p, GameSessionFile, "r");
	if (0 != retval)
	{
		printf("Failed to open file %s for reading!\n", GameSessionFile);
		return STATUS_CODE_FAILURE;
	}
	else {
		fgets(line1, MAX_MOVE_LEN, file_p);
		fgets(line2, MAX_MOVE_LEN, file_p);

	}
	// Matching moves
	if (my_line_num == 0) {
		strcpy_s(player_move, MAX_MOVE_LEN, line1);
		strcpy_s(op_move, MAX_MOVE_LEN, line2);
	}
	else {
		strcpy_s(player_move, MAX_MOVE_LEN, line2);
		strcpy_s(op_move, MAX_MOVE_LEN, line1);
	}
	// Closing file
	if (close_file(file_p) == STATUS_CODE_FAILURE) {
		return STATUS_CODE_FAILURE;
	}
	// clean \n
	for (int i = 0; i < strlen(op_move); i++) {
		if (op_move[i] == '\n') {
			op_move[i] = '\0';
			break;
		}
		if (op_move[i] == '\0') break;
	}
	for (int i = 0; i < strlen(player_move); i++) {
		if (player_move[i] == '\n') {
			player_move[i] = '\0';
			break;
		}
		if (player_move[i] == '\0') break;
	}
	return STATUS_CODE_SUCCESS;

}
/*deleting file from directory*/
int remove_file() {

	int removing_file = remove("GameSession.txt");
	if (removing_file != 0) {
		printf("Error occured: unable to delete file %s\n", GameSessionFile);
		return STATUS_CODE_FAILURE;
	}
	return STATUS_CODE_SUCCESS;
}

