#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

#define SERVER_ADDRESS_STR "127.0.0.1"

void Main_server()
{
 SOCKET MainSocket = INVALID_SOCKET;
 unsigned long Address;
 SOCKADDR_IN service;

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
service.sin_port = htons(SERVER_PORT); //The htons function converts a u_short from host to TCP/IP network byte order 
								   //( which is big-endian ).
server_cleanup_2:
if (closesocket(MainSocket) == SOCKET_ERROR)
printf("Failed to close MainSocket, error %ld. Ending program\n", WSAGetLastError());

server_cleanup_1:
if (WSACleanup() == SOCKET_ERROR)
printf("Failed to close Winsocket, error %ld. Ending program.\n", WSAGetLastError());
}