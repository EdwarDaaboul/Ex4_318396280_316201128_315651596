/*

# Description:
#	In this main file we checked input arguments, and
#	called MainClient function that connects to server.
#
*/


#include "SocketExampleClient.h"
#include "SocketExampleShared.h"

int main(int argc, char* argv[])
{
	if (argc != 4) {
		printf("Please provide the right arguments: client.exe <server ip> <server port> <username>\n");
		return STATUS_CODE_FAILURE;
	}

	return MainClient(argv[1], argv[2], argv[3]);

}