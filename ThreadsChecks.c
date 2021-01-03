#include <winsock2.h>
#include "SocketExampleShared.h"



int WaitForSingleObject_check(HANDLE thread_handle) {

	DWORD waiting_res = WaitForSingleObject(thread_handle, WAIT_TIME_TO_SEND);
	if (waiting_res == WAIT_TIMEOUT) {
		printf("TIMEOUT while waiting thread!\n");
		return WAIT_TIMEOUT;
	}
	else if (waiting_res == WAIT_OBJECT_0) {
		// Thread finished
		return WAIT_OBJECT_0;
	}
	else {
		// Waiting failed
		printf("Error while waiting thread!\n");
		return waiting_res;
	}
}

int SetEvent_check(HANDLE hEvent) {
	if (SetEvent(hEvent) == 0) {
		printf("Error while setting event!\n");
		return STATUS_CODE_FAILURE;
	}
	else return STATUS_CODE_SUCCESS;
}