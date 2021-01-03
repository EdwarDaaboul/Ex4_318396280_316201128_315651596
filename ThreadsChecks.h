#ifndef THREADS_CHECK_HEADER
#define THREADS_CHECK_HEADER
#include <windows.h>

int WaitForSingleObject_check(HANDLE thread_handle);
int SetEvent_check(HANDLE hEvent);

#endif