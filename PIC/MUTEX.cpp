#include "MUTEX.H"

#include <chrono>
#include <thread>

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
void MUTEX::unlock() { mtx = 0; }
void MUTEX::lock() {
	while (__try_lock(&mtx) == 1) {
		std::this_thread::sleep_for(std::chrono::nanoseconds(1));
	}
}