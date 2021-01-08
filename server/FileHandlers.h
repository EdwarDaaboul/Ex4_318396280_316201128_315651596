//
// 
/*
//# FILE Description:
//# In this header file we have declared our functions that is
//# related to handling files
*/

#ifndef __FILES_HNDL_H__
#define __FILES_HNDL_H__

// All functions here returns (-1) if failed and (0) if succeed

/*this function opens file for writing moves*/
int open_file_for_first_time();
/*this file checks if file do exist and no need to open once more*/
int cheacking_if_file_exits(void);
/*this function closes file*/
int close_file(FILE* pfile);
/*this function writes moves into file*/
int write_move_to_file(char* player_move);
/*this function reads moves from file for calculation*/
int reading_from_file_for_calacuation(char op_move[MAX_MOVE_LEN], char player_move[MAX_MOVE_LEN], int my_line_num);
/*deleting file from directory*/
int remove_file();

#endif 