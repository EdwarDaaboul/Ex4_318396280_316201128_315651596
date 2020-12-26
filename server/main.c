#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

void Main_server()
{
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
}