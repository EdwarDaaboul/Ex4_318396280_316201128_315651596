#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

#define SERVER_ADDRESS_STR "127.0.0.1"
#define NUM_OF_WORKER_THREADS 2	
int g_num_of_hanldes;
HANDLE* g_workers_handles;

int Main_server(char* port_str)
{
int Ind;
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

server_cleanup_2:
if (closesocket(MainSocket) == SOCKET_ERROR)
printf("Failed to close MainSocket, error %ld. Ending program\n", WSAGetLastError());

server_cleanup_1:
if (WSACleanup() == SOCKET_ERROR)
printf("Failed to close Winsocket, error %ld. Ending program.\n", WSAGetLastError());
}
