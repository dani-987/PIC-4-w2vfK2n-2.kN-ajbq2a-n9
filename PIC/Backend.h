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

//Die Datei "TPicSim1.LST" sollte lauffähig sein...

class Backend
{
	/*
	Return Values:

	bool: false -> Error
	void/int/.../char*: nullptr -> Error
	int: -1 -> Error (für bool: 1 = true; 0 = false, char all pos. Integers)
	*/
private:
	/*
	if more than one mutex must be locked, only lock in following order:
	m_lastError.lock();
	m_regW.lock();
	m_text_code.lock();
	m_run_code.lock();
	m_isRunning.lock();
	m_terminated.lock();
	m_isRunningLocked.lock();
	m_ram.lock();
	m_eeprom.lock();
	m_callInOtherThread.lock();
	m_runtime.lock();

	<code>

	m_runtime.unlock();
	m_callInOtherThread.unlock();
	m_eeprom.unlock();
	m_ram.unlock();
	m_isRunningLocked.unlock();
	m_terminated.unlock();
	m_isRunning.unlock();
	m_run_code.unlock();
	m_text_code.unlock();
	m_regW.unlock();
	m_lastError.unlock();
	*/
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
	size_t stopAtStackZero;

	std::thread* uC;
	bool isRunning;
	std::mutex m_isRunning;
	bool terminated;
	std::mutex m_terminated;
	bool isRunningLocked;
	std::mutex m_isRunningLocked;

	byte* ram;
	bool sleep;
	byte posDamaged;
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

	//thread-save functions for external usage:
	bool LoadProgramm(char* c);
	bool Start();	//do not call -> not all asm-funcs implemented
	bool Stop();
	bool Step();	//do not call -> not all asm-funcs implemented
	bool Reset();	//not implemented
	int  GetByte(int reg, byte bank);			//bank: 0 or 1
	bool SetByte(int reg, byte bank, byte val);	//bank: 0 or 1
	int  GetBit(int b, byte bank, int pos);	//bool
	bool SetBit(int b, byte bank, int pos, bool val);
	int getRegW();					//char
	bool setRegW(byte val);
	char* getErrorMSG();			//nullptr possible! Remember: malloc! -> free


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

