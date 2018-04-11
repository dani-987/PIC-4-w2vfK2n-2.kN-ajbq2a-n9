#pragma once
#include "DEBUG.H"
#include "ASM.h"
#include <mutex>
#include <thread>

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

	char regW;
	std::mutex m_regW;

	ASM* code;
	std::mutex m_text_code;
	std::mutex m_run_code;

	std::thread* uC;
	bool isRunning;
	std::mutex m_isRunning;
	bool isRunningLocked;
	std::mutex m_isRunningLocked;

	std::thread* fileLoader;
	bool isLoadingFile;
	std::mutex m_isLoadingFile;

	char** ram;
	std::mutex m_ram;

	char** eeprom;
	std::mutex m_eeprom;
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

