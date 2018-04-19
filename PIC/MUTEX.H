#pragma once

#include <chrono>

#ifndef  byte
typedef unsigned char byte;
#endif // ! byte

typedef class MUTEX {
private:
	byte mtx;
public:
	MUTEX();
	void lock();
	void unlock();
}MUTEX;
