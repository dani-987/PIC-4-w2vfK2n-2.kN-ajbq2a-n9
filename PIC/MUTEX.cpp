#include <chrono>
#include <thread>
#include "MUTEX.H"

byte __try_lock(byte*);


MUTEX::MUTEX() { mtx = 0; }
void MUTEX::lock() {
	while (__try_lock(&mtx)) {
		std::this_thread::sleep_for(std::chrono::nanoseconds(1));
	}
}
void MUTEX::unlock() { mtx = 0; }


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

typedef class MUTEX MUTEX;
