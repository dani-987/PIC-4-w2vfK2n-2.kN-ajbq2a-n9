#pragma once
#include <mutex>

class Backend
{
	/*
	Return Values:

	bool: false -> Error
	void/int/.../char*: nullptr -> Error
	int: -1 -> Error (für bool: 1 = true; 0 = false, char alle pos. Zahlen)
	*/
private:
	char* lastmsg;
	int lastmsgLen;
	std::mutex m_lastmsg;
public:
	Backend();
	~Backend();

	bool LoadProgramm(char* c);
	bool Start();
	bool Stop();
	bool Step();
	bool Reset();
	bool SetMem(int from, int len, void* mem);
	bool SetBit(int byte, int pos, bool val);
	void* GetMem(int from, int len);
	int  GetBit(int byte, int pos);	//bool
	int getRegW();					//char
	bool setRegW(char val);
	char* getErrorMSG();			//nullptr möglich! BEDENKE malloc! -> free
};

