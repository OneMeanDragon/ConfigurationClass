#pragma once
#include <windows.h>


class SCritSect
{
private:
	CRITICAL_SECTION sgMemCrit;
public:
	SCritSect(void) {
#ifdef _DEBUG
		OutputDebugString("Debug: Initializing critical section.\r\n");
#endif
		InitializeCriticalSection(&sgMemCrit);
	}
	~SCritSect(void) {
#ifdef _DEBUG
		OutputDebugString("Debug: Destructing critical section in progress.\r\n");
#endif
		DeleteCriticalSection(&sgMemCrit);
	}
	void enter(void) {
		EnterCriticalSection(&sgMemCrit);
#ifdef _DEBUG
		OutputDebugString("Debug: Critical section Entered.\r\n");
#endif
	}
	void leave(void) {
#ifdef _DEBUG
		OutputDebugString("Debug: Critical section Leaving.\r\n");
#endif
		LeaveCriticalSection(&sgMemCrit);
	}
};
