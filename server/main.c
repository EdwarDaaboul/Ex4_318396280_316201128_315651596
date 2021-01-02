/*
# Authors:
#	
#	
# Project:
#	
# Description:
#	In this main file we checked input arguments, and
#	called Main_server function that manages connections to clients.
#
*/
#include "server.h"
#include "SocketExampleShared.h"


int main(int argc, char* argv[])
{

	if (argc != 2) {
		printf("Invalid arguments!\nPlease provide the right arguments: server.exe <port>\n");
		return STATUS_CODE_FAILURE;
	}

	return Main_server(argv[1]);
}