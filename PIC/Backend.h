#pragma once

//HINWEISE FÜR DIE GUI:

//4 GUI: in GUI->run() call updateAll();
//GUI-funktion updateRam(): {StartedUpdating(); updateRegW(); while(getNextChangedCell(reg, bank))updateRamCell(reg, bank);}

//Du kannst nicht einfach in der Funktion GUI::callback_load_file() "free(CODE_table->getstyle());" aufrufen, vorher musst du jeglichen Speicher in der
//		Datenstruktur befreien.... also eine for-schleife über alle elemente und dort dann alle Strings befreien.... (Die Funktion free ist nicht rukursiv!)
//		Siehe als Beispiel in Backend.cpp Backend::FreeProgrammText(ASM_TEXT* &txt), Backend::freeBreakpoints(Breakpointlist*& list) oder im Compiler.cpp 
//		Compiler::freeASM(ASM* toFree). Dort werden die Datenstrukturen stück für Stück gefreit


class Backend;

#define PORT_A_BYTE		0x05
#define PORT_A_BANK		0x00
#define TRIS_A_BYTE		PORT_A_BYTE
#define TRIS_A_BANK		0x01

#define PORT_B_BYTE		0x06
#define PORT_B_BANK		0x00
#define TRIS_B_BYTE		PORT_B_BYTE
#define TRIS_B_BANK		0x01

#define PCL_BYTE		0x02
#define PCL_BANK		0x00
#define PCLATH_BYTE		0x0A
#define PCLATH_BANK		0x00

#define UC_SIZE_RAM		94
#define UC_SIZE_PROGRAM	1024
#define UC_SIZE_EEPROM	64

#define MOD_STANDARD	0
#define MOD_STEP		1
#define MOD_STEP_OUT	2
#define MOD_STEP_OVER	3
#define MOD_IGNORE		4

#define STACK_SIZE		8

#define UC_STANDARD_SPEED	10		//in 100*ns (min 4)

#if UC_SIZE_RAM % 8 == 0
#define UC_DAMAGE_SIZE		(UC_SIZE_RAM / 8)
#define UC_DAMAGE_LAST_SIZE 8
#else
#define UC_DAMAGE_SIZE		((UC_SIZE_RAM / 8)+1)
#define UC_DAMAGE_LAST_SIZE ((UC_SIZE_RAM % 8)+1)
#endif

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

#ifndef byte
typedef unsigned char byte;
#endif

typedef byte bitmap8_t;

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
	int stopAtStackZero;

	std::thread* uC;
	bool isRunning;
	MUTEX m_isRunning;
	bool terminated;
	MUTEX m_terminated;
	bool isRunningLocked;
	MUTEX m_isRunningLocked;

	byte* ram;
	byte ram_rb_cpy;
	bool written_to_PCL;
	bitmap8_t damage[UC_DAMAGE_SIZE];
	size_t countDamaged, posReadingDamage;
	bool reloadCalled, ignoreBreakpoint;
	size_t sleeptime;
	bool sleep;
	size_t prescaler_timer;
	int eeprom_write_time;
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

	unsigned long long runtime;
	MUTEX m_runtime;

	int wdt_timer, wdt_prescaler, wdt_end;
	byte timer_sync;
	bool timer_written, timer_sync_written;
	bool wdt_active, wdt_int_accured;
	MUTEX m_wdt;

	byte & getCell_unsafe(byte pos, bool damage = true);
	void reset(byte resetType);
	void stopAndWait();
	bool letRun(int modus);
	bool do_interrupts(int& needTime);
	bool do_timer();
	bool do_eeprom();
	bool do_wdt();
	void reset_wdt();
	void damageByte(size_t pos);
public:
	Backend(GUI* gui);
	~Backend();

	//calling SetByte, SetBit, setRegW will call Fl::awake(gui_int_update)! Do not change after updating -> values will be always displayed correct...

	//thread-save functions for external usage in GUI:
	ASM_TEXT* GetProgrammText(size_t& anzahl);		//remember malloc! -> freeProgrammText();
	void FreeProgrammText(ASM_TEXT*& prog);
	bool GetNextChangedCell(int &reg, byte &bank);	//get a possibly changed cell to update in gui_int_update(), if (WirdByteGespiegelt(reg) == true) the byte will be only returned for bank 0!
	bool WirdByteGespiegelt(byte pos);				//returns true if pos is same for bank 0 and bank 1, else false
	void StartedUpdating();							//always call before updating, otherwise it is possible that the update-function will not be called when necessary
	bool LoadProgramm(char* c);						//resets runtime if programm is loaded
	bool Start();
	bool Stop();
	bool Step();
	bool IsRunning();
	void EnableWatchdog();
	void DisableWatchdog();
	bool IsWatchdogEnabled();
	long long ToggleBreakpoint(size_t textline);	//returns -1 on error, -2 if breakpoint was unsettet, -3 if breakpoints were unchanged, else the line, where the breakpoint was set
	Breakpointlist* GetBreakpoints();				//returns nullptr on empty list (NOT ERROR!), new! -> freeBreakpoints()
	void FreeBreakpoints(Breakpointlist*& list);	//frees the datastructure returned by GetBreakpionts()
	void SetCommandSpeed(size_t speed);				//standard speed: 'UC_STANDARD_SPEED' (in 100*ns, min 4 * 100*ns)
	bool Reset();									// will do not reset runtime!
	int  GetByte(int reg, byte bank);				//bank: 0 or 1
	bool SetByte(int reg, byte bank, byte val);		//bank: 0 or 1
	int  GetBit(int b, byte bank, int pos);			//bool (= 0 or != 0 [e.g. 2 or 128])
	bool SetBit(int b, byte bank, int pos, bool val);
	int GetRegW();									//byte //always update Register W at updating ram!
	bool SetRegW(byte val);
	char* GetErrorMSG();							//nullptr possible! Remember: malloc! -> free
	void Wait_For_End();							//joins all runnig threads and returns after all threads terminated. Do not forget to call 'Stop' before, else this function will not return!
	unsigned long long GetRuntimeIn100ns();				// will never cause an error
	void ResetRuntime();							// will never cause an error
	size_t GetPC();									// in diff. to GetAktualCodePosition() it will return the Value of PC; not the Codeline incl. Komments
	int GetAktualCodePosition();					// get the Actual Codeline in the .LST-File for displaying it in the Programmtable, it differs form the PC!

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

	//main loop of the uC! (only for internal use)
	void run_in_other_thread(byte modus);

#ifdef _DEBUG
	ASM* get_ASM_ONLY_TESTING(){return code;}

	void set_DEBUG_ONLY_TESTING(int state);
#endif
};

void printProgramRam(Backend* b);