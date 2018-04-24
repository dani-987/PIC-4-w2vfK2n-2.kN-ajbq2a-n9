#pragma once
#ifdef USE_MY_MUTEX
//Handwritten MUTEX, to locate errors faster
//define USE_MY_MUTEX in DEBUG.H for use this file, else std::mutex will be used

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
#endif
