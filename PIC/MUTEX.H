#pragma once

//Handwritten MUTEX, to locate errors faster

#ifndef  byte
typedef unsigned char byte;
#endif // ! byte

class MUTEX {
private:
	byte mtx;
public:
	MUTEX();
	void lock();
	void unlock();
};
