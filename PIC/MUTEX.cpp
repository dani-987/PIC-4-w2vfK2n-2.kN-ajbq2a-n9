#include "DEBUG.H"
#ifdef USE_MY_MUTEX
#include "MUTEX.H"

#include <chrono>
#include <thread>
#include <Windows.h>

byte __try_lock(byte* mutex) {
	byte ret = 1;
	__asm {
		mov al, [ret];
		mov ebx, [mutex];
		xchg[ebx], al;
		mov[ret], al;
	};
	return ret;
}

MUTEX::MUTEX() { mtx = 0; }
void MUTEX::unlock() { 
#ifdef _DEBUG
	if(mtx == 1){
		if(threadid != GetCurrentThreadId()){
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 0x4C);
			printf("\n\n\nWARNING IN UNLOCK_MUTEX:\n\nLocked in thread %d and undlocking in thread %d\n\n\n", threadid, GetCurrentThreadId());
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 0x70);
		}
	}
	else {
		
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 0x4C);
		printf("\n\n\nWARNING IN UNLOCK_MUTEX:\n\nUnlocking not locked mutex in thread %d\n\n\n", GetCurrentThreadId());
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 0x70);
	}
#endif
	mtx = 0; 
}
void MUTEX::lock() {
	while (__try_lock(&mtx) == 1) {
		std::this_thread::sleep_for(std::chrono::nanoseconds(1));
	}
	threadid = GetCurrentThreadId();
}
#endif