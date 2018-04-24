#pragma once

class Backend;

#include "DEBUG.H"
#include "ASM.h"
#include "GUI.h"

#ifdef USE_MY_MUTEX
#include "MUTEX.H"
#else // USE_MY_MUTEX
#include <mutex>
typedef std::mutex MUTEX;
#endif // !USE_MY_MUTEX

#include <thread>

#define UC_SIZE_RAM		94
#define UC_SIZE_PROGRAM	1024
#define UC_SIZE_EEPROM	64

#define MOD_STANDARD	0
#define MOD_STEP		1
#define MOD_STEP_OUT	2
#define MOD_STEP_OVER	3
#define MOD_IGNORE		4

#define STACK_SIZE		8

#define UC_STANDARD_SPEED	1000		//in ns

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
	LOCK_MUTEX(m_lastError);
	LOCK_MUTEX(m_regW);
	LOCK_MUTEX(m_text_code);
	LOCK_MUTEX(m_run_code);
	LOCK_MUTEX(m_isRunning);
	LOCK_MUTEX(m_terminated);
	LOCK_MUTEX(m_isRunningLocked);
	LOCK_MUTEX(m_ram);
	LOCK_MUTEX(m_eeprom);
	LOCK_MUTEX(m_callInOtherThread);
	LOCK_MUTEX(m_runtime);
	LOCK_MUTEX(m_wdt);

	<code>

	UNLOCK_MUTEX(m_wdt);
	UNLOCK_MUTEX(m_runtime);
	UNLOCK_MUTEX(m_callInOtherThread);
	UNLOCK_MUTEX(m_eeprom);
	UNLOCK_MUTEX(m_ram);
	UNLOCK_MUTEX(m_isRunningLocked);
	UNLOCK_MUTEX(m_terminated);
	UNLOCK_MUTEX(m_isRunning);
	UNLOCK_MUTEX(m_run_code);
	UNLOCK_MUTEX(m_text_code);
	UNLOCK_MUTEX(m_regW);
	UNLOCK_MUTEX(m_lastError);
	*/
	byte tmp;

	char* lastError;
	int lastErrorLen;
	bool errorInThreadHappend;
	MUTEX m_lastError;

	byte regW;
	MUTEX m_regW;

	ASM* code;
	MUTEX m_text_code;
	MUTEX m_run_code;

	ASM_CODE* aktCode;
	Breakpointlist* breakpoints;

	STACK* functionStack;
	size_t stackSize;
	size_t stopAtStackZero;

	std::thread* uC;
	bool isRunning;
	MUTEX m_isRunning;
	bool terminated;
	MUTEX m_terminated;
	bool isRunningLocked;
	MUTEX m_isRunningLocked;

	byte* ram;
	byte ram_rb_cpy;
	size_t sleeptime;
	bool sleep;
	size_t prescaler_timer;
	size_t eeprom_write_time;
	byte eeprom_write_state;
	unsigned long long time_eeprom_error_write;	//used for write errors at much write cycles
	byte eeprom_wr_addr;						//needed for wirte errors
	bool eeprom_wr, eeprom_rd;
	int lastInput;
	MUTEX m_ram;

	char* eeprom;
	MUTEX m_eeprom;

	GUI* gui;

	call_in_other_thread_s callInOtherThread;
	MUTEX m_callInOtherThread;

	unsigned int runtime;
	MUTEX m_runtime;

	size_t wdt_timer, wdt_prescaler, wdt_end;
	bool wdt_active, wdt_int_accured;
	MUTEX m_wdt;

	byte & getCell_unsafe(byte pos);
	void reset(byte resetType);
	void Stop_And_Wait();
	bool letRun(int modus);
	bool do_interrupts(int& needTime);
	bool do_timer();
	bool do_eeprom();
	bool do_wdt();
	void reset_wdt();
public:
	Backend(GUI* gui);
	~Backend();

	//thread-save functions for external usage in GUI:
	bool LoadProgramm(char* c);
	bool Start();
	bool Stop();
	bool Step();
	bool IsRunning();
	void EnableWatchdog();
	void DisableWatchdog();
	bool isWatchdogEnabled();
	long long ToggleBreakpoint(size_t textline);	//returns -1 on error, -2 if breakpoint was unsettet, -3 if breakpoints were unchanged, else the line, where the breakpoint was set
	Breakpointlist* GetBreakpoints();				//returns nullptr on empty list (NOT ERROR!), new! -> freeBreakpoints()
	void freeBreakpoints(Breakpointlist*& list);	//frees the datastructure returned by GetBreakpionts()
	void setCommandSpeed(size_t speed);				//standard speed: 'UC_STANDARD_SPEED' (in ns)
	bool Reset();
	int  GetByte(int reg, byte bank);				//bank: 0 or 1
	bool SetByte(int reg, byte bank, byte val);		//bank: 0 or 1
	int  GetBit(int b, byte bank, int pos);			//bool
	bool SetBit(int b, byte bank, int pos, bool val);
	int getRegW();									//byte
	bool setRegW(byte val);
	char* getErrorMSG();							//nullptr possible! Remember: malloc! -> free
	void Wait_For_End();							//joins all runnig threads. do not forget to call 'Stop' before!


	//following ist for internal use only and not thread-save!
	//asm-commands:
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

	//main loop of the uC!
	void run_in_other_thread(byte modus);
};

