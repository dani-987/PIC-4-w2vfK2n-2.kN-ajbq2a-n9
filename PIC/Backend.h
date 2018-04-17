#pragma once

class Backend;

#include "DEBUG.H"
#include "ASM.h"
#include "GUI.h"
#include <mutex>
#include <thread>

#define UC_SIZE_RAM		94
#define UC_SIZE_PROGRAM	1024
#define UC_SIZE_EEPROM	64

#define MOD_STANDARD	0
#define MOD_STEP		1
#define MOD_STEP_OUT	2
#define MOD_STEP_OVER	3
#define MOD_IGNORE		4

#ifndef byte
typedef unsigned char byte;
#endif

struct call_in_other_thread_s {
	Backend* backend;
	int modus;
};

class Backend
{
	/*
	Return Values:

	bool: false -> Error
	void/int/.../char*: nullptr -> Error
	int: -1 -> Error (für bool: 1 = true; 0 = false, char alle pos. Zahlen)
	*/
private:
	byte tmp;

	char* lastError;
	int lastErrorLen;
	std::mutex m_lastError;

	char regW;
	std::mutex m_regW;

	ASM* code;
	std::mutex m_text_code;
	std::mutex m_run_code;

	ASM_CODE* aktCode;

	STACK* functionStack;

	std::thread* uC;
	bool isRunning;
	std::mutex m_isRunning;
	bool terminated;
	std::mutex m_terminated;
	bool isRunningLocked;
	std::mutex m_isRunningLocked;

	byte* ram;
	std::mutex m_ram;

	char* eeprom;
	std::mutex m_eeprom;

	GUI* gui;

	call_in_other_thread_s callInOtherThread;
	std::mutex m_callInOtherThread;

	unsigned int runtime;
	std::mutex m_runtime;

	byte & getCell_unsafe(byte pos);
	void reset(byte resetType);
	void Stop_And_Wait();
	bool letRun(int modus);
public:
	Backend(GUI* gui);
	~Backend();

	bool LoadProgramm(char* c);
	bool Start();
	bool Stop();
	bool Step();
	bool Reset();
	bool SetMem(int from, int len, void* mem);//es wird kein free aufgerufen
	bool SetBit(int byte, int pos, bool val);
	void* GetMem(int from, int len);//nullptr möglich! DEDENKE malloc! -> free
	int  GetBit(int byte, int pos);	//bool
	int getRegW();					//char
	bool setRegW(char val);
	char* getErrorMSG();			//nullptr möglich! BEDENKE malloc! -> free


	//following ist for internal use only and not thread-save!
	int ADDWF(void*f, void*d);
	int ANDWF(void*f, void*d);
	int CLRF(void*f, void*ign);
	int CLRW(void*ign1, void*ign2);
	int COMF(void*f, void*d);
	int DECF(void*f, void*d);
	int DECFSZ(void*f, void*d);
	int INCF(void*f, void*d);
	int INCFSZ(void*f, void*d);
	int IORWF(void*f, void*d);
	int MOVF(void*f, void*d);
	int MOVWF(void*f, void*ign);
	int NOP(void*ign1, void*ign2);
	int RLF(void*f, void*d);
	int RRF(void*f, void*d);
	int SUBWF(void*f, void*d);
	int SWAPF(void*f, void*d);
	int XORWF(void*f, void*d);

	int BCF(void*f, void*b);
	int BSF(void*f, void*b);
	int BTFSC(void*f, void*b);
	int BTFSS(void*f, void*b);

	int ADDLW(void*k, void*ign);
	int ANDLW(void*k, void*ign);
	int CALL(void*k, void*ign);
	int CLRWDT(void*ign1, void*ign2);
	int GOTO(void*k, void*ign);
	int IORLW(void*k, void*ign);
	int MOVLW(void*k, void*ign);
	int RETFIE(void*ign1, void*ign2);
	int RETLW(void*k, void*ign);
	int RETURN(void*ign1, void*ign2);
	int SLEEP(void*ign1, void*ign2);
	int SUBLW(void*k, void*ign);
	int XORLW(void*k, void*ign);

	void run_in_other_thread(byte modus);
};

