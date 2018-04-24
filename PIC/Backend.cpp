#include "Backend.h"
#include "Compiler.h"

#include <cstring>
#include <chrono>

#include <stdlib.h>
#include <time.h>

//Komment following if not wanted....
#define USE_RANDOM_VALUES
//TODO:

//GUI-Funkionen, damage
//Interrupts

#define NOT_IMPLEMENTED	"Nicht implementiert!"
#define MEMORY_MISSING "Memory is missing."
#define FUNCTIONSTACK_EMPTY	"Fuctionstack is empty!"

#define RESET_POWER_UP		0
#define RESET_CLRWDT		1
#define RESET_SLEEP			2
#define RESET_WDT_TIMEOUT	3

#define MSGLEN()	(lastErrorLen = (std::strlen(lastError)+1))

#define DEBUG_NONE	0
#define DEBUG_LESS	1
#define DEBUG_NORM	2
#define DEBUG_MORE	3
#define DEBUG_ALL	4

VARDEF(int, DEBUG_LVL, DEBUG_ALL);

void call_backup_in_other_thread(void* _callInOtherThread) {
	struct call_in_other_thread_s* callInOtherThread = (struct call_in_other_thread_s*)_callInOtherThread;
	DOIF(callInOtherThread->modus > 4 || callInOtherThread->modus < 0)
		PRINTF1("WRONG MODUS! (%d)\n", callInOtherThread->modus);
	callInOtherThread->backend->run_in_other_thread(callInOtherThread->modus);
	delete(_callInOtherThread);
}

byte & Backend::getCell_unsafe(byte pos)
{
	if (pos == 0x00) {	//indirect addressing
		pos = ram[0x04];
	}
	//rest
	if (pos == 0x07 || pos > 0x4F || pos == 0x00 ) {
		tmp = 0;
		return tmp;
	}
	else if (pos >= 0x0A || (pos >= 0x02 && pos <= 0x04)) { 
		return ram[pos]; 
	}
	else {
		if (ram[0x03] & 0xC0) {
			LOCK_MUTEX(m_lastError);
			lastError = "Falsche Bank gew‰hlt!";
			lastErrorLen = MSGLEN();
			UNLOCK_MUTEX(m_lastError);
			tmp = 0;
			return tmp;
		}
		else if (ram[0x03] & 0x40) {
			return ram[82 + pos];
		}
		else {
			return ram[pos];
		}
	}
}

void Backend::reset(byte resetType)
{
	//clear stack!
	while (functionStack != nullptr) {
		STACK* tmp = functionStack;
		functionStack = functionStack->next;
		free(tmp);
	}
	stackSize = 0;

	//reset wdt
	wdt_int_accured = false;
	reset_wdt();

	//reset neccessary bytes in ram
	byte tmp = ram[90];

	ram[0x02] = 0x00;
	ram[0x03] &= 0x1F;
	ram[0x0A] = 0;
	ram[0x0B] &= 0xFE;

	ram[83] = 0xFF;
	ram[84] = 0x00;
	ram[87] = 0x1F;
	ram[88] = 0xFF;
	ram[90] = 0x00;

	ram_rb_cpy = ram[0x06];

	sleep = false;
	prescaler_timer = 0;
	time_eeprom_error_write = 0;
	lastInput = ram[0x06] & 0x01;

	if ((tmp & 0x04) && eeprom_wr) {
		ram[90] |= 0x80;
	}
	eeprom_wr = false;

	switch (resetType) {
	case RESET_SLEEP:
		ram[0x03] |= 0x10;
		break;
	case RESET_WDT_TIMEOUT:
		ram[0x03] |= 0x08;
		break;
	case RESET_CLRWDT:
	case RESET_POWER_UP:
	default:
		ram[0x03] |= 0x18;
		ram[89] |= 0x10;
		break;
	}
}

void Backend::Stop_And_Wait()
{
	LOCK_MUTEX(m_isRunningLocked);
	isRunningLocked = true;
	UNLOCK_MUTEX(m_isRunningLocked);
	Stop();
	Wait_For_End();
}

bool Backend::letRun(int modus)
{
	LOCK_MUTEX(m_isRunningLocked);
	if (isRunningLocked) {
		UNLOCK_MUTEX(m_isRunningLocked);
		LOCK_MUTEX(m_lastError);
		lastError = "Der Start ist gesperrt!";
		MSGLEN();
		UNLOCK_MUTEX(m_lastError);
		PRINTF("Start is locked\n");
		return false;
	}
	UNLOCK_MUTEX(m_isRunningLocked);
	LOCK_MUTEX(m_run_code);
	LOCK_MUTEX(m_isRunning);
	if (isRunning) {
		UNLOCK_MUTEX(m_isRunning);
		UNLOCK_MUTEX(m_run_code);
		LOCK_MUTEX(m_terminated);
		if (!terminated) {
			UNLOCK_MUTEX(m_terminated);
			LOCK_MUTEX(m_lastError);
			lastError = "Der Kontroller l‰uft aktuell!";
			PRINTF("Kontroller is running already\n");
			MSGLEN();
			UNLOCK_MUTEX(m_lastError);
			return false;
		}
		UNLOCK_MUTEX(m_terminated);
		LOCK_MUTEX(m_isRunning);
	}
	if (uC != nullptr) {
		uC->join();
		delete(uC);
		uC = nullptr;
	}
	if (code == nullptr || code->code == nullptr) {
		UNLOCK_MUTEX(m_isRunning);
		UNLOCK_MUTEX(m_run_code);
		LOCK_MUTEX(m_lastError);
		lastError = "Kein Programm geladen!";
		PRINTF("Kein Programm geladen\n");
		MSGLEN();
		UNLOCK_MUTEX(m_lastError);
		return false;
	}
	isRunning = true;
	UNLOCK_MUTEX(m_isRunning);
	call_in_other_thread_s* data = new call_in_other_thread_s{ this, modus };
	uC = new std::thread(call_backup_in_other_thread, data);
	UNLOCK_MUTEX(m_run_code);
	return true;
}

bool Backend::do_interrupts(int& needTime)
{
	//if wdt_int
	if (wdt_int_accured) {
		if (sleep) {
			wdt_int_accured = false;

			goto DO_INTERRUPT;
		}
		reset(RESET_WDT_TIMEOUT);
		needTime = 1;
		return true;
	}
	//RBIF in INTCON
	if (ram_rb_cpy & 0xF0 != ram[0x06] & 0xF0)ram[0x0B] |= 0x01;
	//INTF if RB0/INT (check if on change or on set...)
	if (ram_rb_cpy & 0x01 != ram[0x06] & 0x01)ram[0x0B] |= 0x02;
	ram_rb_cpy = ram[0x06];
	//if GIE or sleeping have to check interrupts....
	if (ram[0x0B] & 0x80 || sleep) {
		if (ram_rb_cpy != ram[0x06]) {

			//T0IF & T0IE
			if ((ram[0x0B] & 0x24) == 0x24) {
				goto DO_INTERRUPT;
			}
			//INTF & INTE
			if ((ram[0x0B] & 0x12) == 0x12) {
				goto DO_INTERRUPT;
			}
			//RBIF & RBIE
			if ((ram[0x0B] & 0x09) == 0x09) {
				goto DO_INTERRUPT;
			}
			//EEIF & EEIE
			if ((ram[0x0B] & 0x40) && (ram[90] & 0x10)) {
				goto DO_INTERRUPT;
			}
		}
	}
	return true;
DO_INTERRUPT:
	//wake-up if sleeping
	sleep = false;
	//if sleeping, GIE decises, if goto into interrupt routine or not and continue code; if not sleeping, GIE is enabled here
	if (ram[0x0B] & 0x80) {
		//unset GIE
		ram[0x0B] &= ~0x80;
		//returnAddr -> Stack
		STACK* newAdress = (STACK*)malloc(sizeof(STACK));
		if (newAdress == nullptr) {
			lastError = MEMORY_MISSING;
			return -1;
		}
		newAdress->jumpTo = aktCode;
		newAdress->next = functionStack;
		functionStack = newAdress;
		//PC = 0x04
		aktCode = &(code->code[0x04]);
		if (stopAtStackZero >= 0)stopAtStackZero++;
		//delay
		needTime = 3;
	}
	return true;
}

bool Backend::do_timer()
{
	//internal or external clock?
	if(ram[83] & 0x10){
		//input changed
		if (lastInput != ram[0x06] & 0x01) {
			lastInput ^= 0x01;
			//test if correct change happend
			if (lastInput << 6 == ram[83] & 0x40) {
				//with prescaler?
				if (ram[83] & 0x04) {
					ram[0x01]++;
					if (ram[0x01] == 0)ram[0x08] |= 0x20;	//do interrupt
				}
				else {
					prescaler_timer++;
					int prescaler = (2 << (ram[83] & 0x07));
					if (prescaler_timer > prescaler) {
						prescaler_timer = 0;
						ram[0x01]++;
						if (ram[0x01] == 0)ram[0x08] |= 0x20;	//do interrupt
					}
				}
			}
		}
	}
	else {
		//with prescaler?
		if (ram[83] & 0x04) {
			ram[0x01]++;
			if (ram[0x01] == 0)ram[0x08] |= 0x20;	//do interrupt
		}
		else {
			prescaler_timer++;
			int prescaler = (2 << (ram[83] & 0x07));
			if (prescaler_timer > prescaler) {
				prescaler_timer = 0;
				ram[0x01]++;
				if (ram[0x01] == 0)ram[0x08] |= 0x20;	//do interrupt
			}
		}
	}
	return true;
}

bool Backend::do_eeprom()
{
	//eedata 08h
	//eeaddress 09h
	//eecon1 88h
	//eecon2 89h (not phys)
	if (time_eeprom_error_write < 100) {
		time_eeprom_error_write++;
	}
	if (ram[90] & 0x04 && ram[91] == 0x55) {
		eeprom_write_state = 1;
	}
	else if (ram[90] & 0x04 && ram[91] == 0xAA) {
		if (eeprom_write_state == 1)eeprom_write_state = 2;
	}
	if (eeprom_wr) {
		ram[90] |= 0x20;
		if (ram[90] & 0x04) {
			eeprom_write_time--;
			if (eeprom_write_time) {
				//write eeprom
				ram[90] &= ~0x20;
				eeprom_wr = false;
				ram[90] |= 0x10;
				if (eeprom_write_time < 5) {	//cause a biterror at eeprom write because the eeprom is used much
					byte tmp = eeprom[eeprom_wr_addr];
					eeprom[eeprom_wr_addr] &= rand() & 0xFF;
					if (eeprom[eeprom_wr_addr] != tmp) {	//biterror at eeprom write was caused
						eeprom_write_time = 22;
						DOIF(DEBUG_LVL >= DEBUG_NORM)PRINTF("EEPROM WRITE BITERROR\n");
					}
					//else;	//biterror donot changed value....
				}
			}
		}
	}
	else if (ram[90] & 0x20) {
		if (eeprom_write_state == 2) {
			eeprom_write_state = 0;
			eeprom_wr = true;
			if (time_eeprom_error_write > 22)time_eeprom_error_write = 0;
			else time_eeprom_error_write - 22;

			eeprom_wr_addr = ram[0x09];
			eeprom[eeprom_wr_addr] = ram[0x08];
			eeprom_write_time = 8 + rand() % 13;	//typical write time: 10ms, max: 20ms (one operation: 1ms)
		}
		else {
			lastError = "To write EEPROM first set WREN bit in EECON1, then write 0x55 and later 0xAA into EECON2 before setting WR bit in EECON1!";
			MSGLEN();
			PRINTF1("To write EEPROM first set WREN bit in EECON1, then write 0x55 and later 0xAA into EECON2 before setting WR bit in EECON1 (status == %d)!\n", eeprom_write_state);
			return false;
		}
	}
	else if (ram[90] & 0x01) {
		ram[90] &= ~0x01;

		//do eeprom read
		ram[0x08] = eeprom[ram[0x09]];
	}


	return true;
}

bool Backend::do_wdt()
{
	if (!wdt_active)return true;
	//PSA is set -> Prescaller is assigned 2 wdt
	if (ram[83] & 0x80) {
		wdt_prescaler++;
		//if wdt_prescaler >= settet prescaler
		if (wdt_prescaler >= (1 << (ram[83] & 0x07))) {
			wdt_prescaler = 0;
			wdt_timer--;
		}
	}
	else {
		wdt_timer--;
	}
	if (wdt_timer <= 0) {
		wdt_int_accured = true;
		reset_wdt();
	}
	return true;
}

void Backend::reset_wdt()
{
	wdt_prescaler = 0;
	wdt_timer = 17 + rand() % 3;	//wdt is _ABOUT_ 18ms
}

Backend::Backend(GUI* gui)
{
	lastError = "Kein Fehler";
	lastErrorLen = MSGLEN();
	srand(time(NULL));
#ifdef USE_RANDOM_VALUES
	regW = rand() & 0xFF;
#else
	regW = 0;
#endif
	code = nullptr;
	aktCode = nullptr;
	functionStack = nullptr;
	uC = nullptr;
	isRunning = false;
	terminated = true;
	isRunningLocked = false;
	ram = (byte*)malloc(UC_SIZE_RAM);
	for (int i = 0; i < UC_SIZE_RAM; i++) {
#ifdef USE_RANDOM_VALUES
		ram[i] = rand() & 0xFF;
#else
		ram[i] = 0;
#endif
	};
	reset(RESET_POWER_UP);
	eeprom = (char*)malloc(UC_SIZE_EEPROM);
	for (int i = 0; i < UC_SIZE_EEPROM; i++) {
#ifdef USE_RANDOM_VALUES
		eeprom[i] = rand() & 0xFF;
#else
		eeprom[i] = 0;
#endif
	}
	eeprom_wr = false;
	eeprom_rd = false;
	uC = nullptr;
	sleeptime = UC_STANDARD_SPEED;
}

Backend::~Backend()
{
	Stop_And_Wait();
	free(eeprom);
	free(ram);
	if (functionStack != nullptr)free(functionStack);//todo
	if (code != nullptr) 
	Compiler::freeASM(code); 
}

bool Backend::LoadProgramm(char * c)
{
	bool ret = false;
	Stop_And_Wait();
	Compiler comp;
	ASM* prog = comp.compileFile(c, UC_SIZE_PROGRAM);
	if (prog != nullptr) {
		if (this->code)Compiler::freeASM(this->code);
		this->code = prog;
		this->aktCode = prog->code;
		ret = true;
	}
	for (int i = 0; i < UC_SIZE_RAM; i++) {
#ifdef USE_RANDOM_VALUES
		ram[i] = rand() & 0xFF;
#else
		ram[i] = 0;
#endif
	};
	reset(RESET_POWER_UP);
	LOCK_MUTEX(m_isRunningLocked);
	isRunningLocked = false;
	UNLOCK_MUTEX(m_isRunningLocked);
	return ret;
}

bool Backend::Start()
{
	return letRun(MOD_STANDARD);
}

bool Backend::Stop()
{
	LOCK_MUTEX(m_isRunning);
	isRunning = false;
	UNLOCK_MUTEX(m_isRunning);
	return true;
}

bool Backend::Step()
{
	return letRun(MOD_STEP);
}

bool Backend::IsRunning()
{
	bool ret; 
	LOCK_MUTEX(m_isRunning);
	ret = isRunning;
	UNLOCK_MUTEX(m_isRunning);
	return false;
}

void Backend::EnableWatchdog()
{
	LOCK_MUTEX(m_wdt);
	wdt_active = true;
	UNLOCK_MUTEX(m_wdt);
}

void Backend::DisableWatchdog()
{
	LOCK_MUTEX(m_wdt);
	wdt_active = false;
	UNLOCK_MUTEX(m_wdt);
}

bool Backend::isWatchdogEnabled()
{

	LOCK_MUTEX(m_wdt);
	bool isEnabled = wdt_active;
	UNLOCK_MUTEX(m_wdt);
	return isEnabled;
}

long long Backend::ToggleBreakpoint(size_t textline)
{
	size_t foundLine = 0, aktLine = 0;
	LOCK_MUTEX(m_run_code);
	for (; aktLine < UC_SIZE_PROGRAM && foundLine < textline; aktLine++) {
		if (code[aktLine].code != nullptr) {
			foundLine = code[aktLine].code->guiText->lineNumber;
		}
	}
	if (foundLine < textline) {
		for (int i = 0; i < UC_SIZE_PROGRAM; i++) {
			if (code[i].code != nullptr) {
				if (!code[i].code->breakpoint) {
					code[i].code->breakpoint = true;
					UNLOCK_MUTEX(m_run_code);
					return i;
				}
				else {
					UNLOCK_MUTEX(m_run_code);
					return -3;
				}
			}
		}
	}
	else if (foundLine == textline) {
		if (!code[aktLine].code->breakpoint) {
			code[aktLine].code->breakpoint = true;
			UNLOCK_MUTEX(m_run_code);
			return aktLine;
		}
		else {
			code[aktLine].code->breakpoint = false;
			UNLOCK_MUTEX(m_run_code);
			return -2;
		}
	}
	else {
		if (!code[aktLine].code->breakpoint) {
			code[aktLine].code->breakpoint = true;
			UNLOCK_MUTEX(m_run_code);
			return aktLine;
		}
		else {
			UNLOCK_MUTEX(m_run_code);
			return -3;
		}
	}
	UNLOCK_MUTEX(m_run_code);
	return -1;
}

Breakpointlist * Backend::GetBreakpoints()
{
	Breakpointlist* list = nullptr, *tmp = breakpoints;
	LOCK_MUTEX(m_run_code);
	while (tmp != nullptr) {
		Breakpointlist* tmpList = new Breakpointlist();
		tmpList->line = tmp->line;
		tmpList->next = list;
		list = tmpList;
	}
	UNLOCK_MUTEX(m_run_code);
	return list;
}

void Backend::freeBreakpoints(Breakpointlist*& list)
{
	Breakpointlist* tmp = list;
	while (tmp != nullptr) {
		tmp = list->next;
		delete(list);
		list = tmp;
	}
	list = nullptr;
}

void Backend::setCommandSpeed(size_t speed)
{
	LOCK_MUTEX(m_ram);
	sleeptime = speed;
	UNLOCK_MUTEX(m_ram);
}

bool Backend::Reset()
{
	Stop_And_Wait();

	LOCK_MUTEX(m_run_code);
	LOCK_MUTEX(m_ram);
	LOCK_MUTEX(m_eeprom);
	LOCK_MUTEX(m_wdt);

	//changing ram is not necessary....
	reset(RESET_POWER_UP);

	UNLOCK_MUTEX(m_wdt);
	UNLOCK_MUTEX(m_eeprom);
	UNLOCK_MUTEX(m_ram);
	UNLOCK_MUTEX(m_run_code);
	return true;
}

int Backend::GetByte(int reg, byte bank)
{
#ifdef _DEBUG
	if (bank > 1 || bank < 0) {
		LOCK_MUTEX(m_lastError);
		lastError = "'bank' muss 1 oder 0 sein!";
		MSGLEN();
		PRINTF1("int Backend::GetByte(int reg, byte bank):\n\t'bank' muss 1 oder 0 sein und ist %d!\n", bank);
		UNLOCK_MUTEX(m_lastError);
		return -1;
	}
	if (reg < 0 || reg > 0x0F) {
		LOCK_MUTEX(m_lastError);
		lastError = "'reg' muss zwischen (einschlieﬂlich) 0x00 und 0xFF sein!";
		MSGLEN();
		PRINTF1("int Backend::GetByte(int reg, byte bank):\n\t'reg' muss zwischen (einschlieﬂlich) 0x00 und 0xFF sein und ist %d!\n", reg);
		UNLOCK_MUTEX(m_lastError);
		return -1;
	}
#endif
	int ret, tmp;
	LOCK_MUTEX(m_ram);
	tmp = ram[0x03];
	if ((reg = 0x0F) == 0x03) { UNLOCK_MUTEX(m_ram); return tmp; }
	if (bank == 0)ram[0x03] = ram[0x03] & (~0x20);
	else ram[0x03] = ram[0x03] | 0x20;
	ret = getCell_unsafe(reg);
	ram[0x03] = tmp;
	UNLOCK_MUTEX(m_ram);
	return ret;
}

bool Backend::SetByte(int reg, byte bank, byte val)
{
#ifdef _DEBUG
	if (bank > 1 || bank < 0) {
		LOCK_MUTEX(m_lastError);
		lastError = "'bank' muss 1 oder 0 sein!";
		MSGLEN();
		PRINTF1("bool Backend::SetByte(int reg, byte bank, byte val):\n\t'bank' muss 1 oder 0 sein und ist %d!\n", bank);
		UNLOCK_MUTEX(m_lastError);
		return -1;
	}
	if (reg < 0 || reg > 0x0F) {
		LOCK_MUTEX(m_lastError);
		lastError = "'reg' muss zwischen (einschlieﬂlich) 0x00 und 0xFF sein!";
		MSGLEN();
		PRINTF1("bool Backend::SetByte(int reg, byte bank, byte val):\n\t'reg' muss zwischen (einschlieﬂlich) 0x00 und 0xFF sein und ist %d!\n", reg);
		UNLOCK_MUTEX(m_lastError);
		return -1;
	}
#endif
	int tmp;
	LOCK_MUTEX(m_ram);
	if ((reg = 0x0F) == 0x03) { ram[0x03] = val; UNLOCK_MUTEX(m_ram); return true; }
	tmp = ram[0x03];
	if (bank == 0)ram[0x03] = ram[0x03] & (~0x20);
	else ram[0x03] = ram[0x03] | 0x20;
	getCell_unsafe(reg) = val;
	ram[0x03] = tmp;
	UNLOCK_MUTEX(m_ram);
	return true;
}

bool Backend::SetBit(int reg, byte bank, int pos, bool val)
{
#ifdef _DEBUG
	if (pos > 7 || pos < 0) {
		LOCK_MUTEX(m_lastError);
		lastError = "'pos' muss zwischen (einschlieﬂlich) 0 und 7 sein!";
		MSGLEN();
		PRINTF1("bool Backend::SetBit(int reg, byte bank, int pos, bool val):\n\t'pos' muss zwischen (einschlieﬂlich) 0 und 7 sein und ist %d!\n", pos);
		UNLOCK_MUTEX(m_lastError);
		return -1;
	}
	if (bank > 1 || bank < 0) {
		LOCK_MUTEX(m_lastError);
		lastError = "'bank' muss 1 oder 0 sein!";
		MSGLEN();
		PRINTF1("bool Backend::SetBit(int reg, byte bank, int pos, bool val):\n\t'bank' muss 1 oder 0 sein und ist %d!\n", bank);
		UNLOCK_MUTEX(m_lastError);
		return -1;
	}
	if (reg < 0 || reg > 0x0F) {
		LOCK_MUTEX(m_lastError);
		lastError = "'reg' muss zwischen (einschlieﬂlich) 0x00 und 0xFF sein!";
		MSGLEN();
		PRINTF1("bool Backend::SetBit(int reg, byte bank, int pos, bool val):\n\t'reg' muss zwischen (einschlieﬂlich) 0x00 und 0xFF sein und ist %d!\n", reg);
		UNLOCK_MUTEX(m_lastError);
		return -1;
	}
#endif
	int tmp;
	LOCK_MUTEX(m_ram);
	if ((reg = 0x0F) == 0x03) { 
		if(val)ram[0x03] |= (1 << pos);
		else ram[0x03] &= ~(1 << pos);
		UNLOCK_MUTEX(m_ram); 
		return true; 
	}
	tmp = ram[0x03];
	if (bank == 0)ram[0x03] = ram[0x03] & (~0x20);
	else ram[0x03] = ram[0x03] | 0x20;
	if(val)getCell_unsafe(reg) |= (1 << pos);
	else getCell_unsafe(reg) &= ~(1 << pos);
	ram[0x03] = tmp;
	UNLOCK_MUTEX(m_ram);
	return true;
}

int Backend::GetBit(int reg, byte bank, int pos)
{
#ifdef _DEBUG
	if (pos > 7 || pos < 0) {
		LOCK_MUTEX(m_lastError);
		lastError = "'pos' muss zwischen (einschlieﬂlich) 0 und 7 sein!";
		MSGLEN();
		PRINTF1("int Backend::GetBit(int reg, byte bank, int pos):\n\t'pos' muss zwischen (einschlieﬂlich) 0 und 7 sein und ist %d!\n", pos);
		UNLOCK_MUTEX(m_lastError);
		return -1;
	}
	if (bank > 1 || bank < 0) {
		LOCK_MUTEX(m_lastError);
		lastError = "'bank' muss 1 oder 0 sein!";
		MSGLEN();
		PRINTF1("int Backend::GetBit(int reg, byte bank, int pos):\n\t'bank' muss 1 oder 0 sein und ist %d!\n", bank);
		UNLOCK_MUTEX(m_lastError);
		return -1;
	}
	if (reg < 0 || reg > 0x0F) {
		LOCK_MUTEX(m_lastError);
		lastError = "'reg' muss zwischen (einschlieﬂlich) 0x00 und 0xFF sein!";
		MSGLEN();
		PRINTF1("int Backend::GetBit(int reg, byte bank, int pos):\n\t'reg' muss zwischen (einschlieﬂlich) 0x00 und 0xFF sein und ist %d!\n", reg);
		UNLOCK_MUTEX(m_lastError);
		return -1;
	}
#endif
	int tmp;
	bool val;
	LOCK_MUTEX(m_ram);
	if ((reg = 0x0F) == 0x03) {
		val = (ram[0x03] & (1 << pos));
		UNLOCK_MUTEX(m_ram);
		return val;
	}
	tmp = ram[0x03];
	if (bank == 0)ram[0x03] = ram[0x03] & (~0x20);
	else ram[0x03] = ram[0x03] | 0x20;
	val = (getCell_unsafe(reg) & (1 << pos));
	ram[0x03] = tmp;
	UNLOCK_MUTEX(m_ram);
	return true;
}

int Backend::getRegW()
{
	int w;
	LOCK_MUTEX(m_regW);
	w = regW;
	UNLOCK_MUTEX(m_regW);
	return w;
}

bool Backend::setRegW(byte val)
{
	LOCK_MUTEX(m_regW);
	regW = val;
	UNLOCK_MUTEX(m_regW);
	return true;
}

char * Backend::getErrorMSG()
{
	LOCK_MUTEX(m_lastError);
	if (lastError == nullptr)return nullptr;
	char* ret = (char*)malloc(lastErrorLen*sizeof(char));
	memcpy(ret, lastError, lastErrorLen * sizeof(char));
	UNLOCK_MUTEX(m_lastError);
	return ret;
}

void Backend::Wait_For_End()
{
	LOCK_MUTEX(m_terminated);
	while (!terminated) {
		UNLOCK_MUTEX(m_terminated);
		std::this_thread::sleep_for(std::chrono::nanoseconds(1));
		LOCK_MUTEX(m_terminated);
	}
	UNLOCK_MUTEX(m_terminated);
}


//#######################################################################################
//#######################################################################################
//##################	M	I	C	R	O	C	O	D	E	S	#########################
//#######################################################################################
//#######################################################################################

//Carry = 9th Bit after ADD
#define BIT_C		0x01
#define BYTE_C		0x03
//DigitCarry = 5th Bit after ADD
#define BIT_DC		0x02
#define BYTE_DC		0x03
//Zerobit
#define BIT_Z		0x04
#define BYTE_Z		0x03
//Programmcounter
#define PCL			0x02
#define PCH			0x0A

int Backend::ADDWF(void*f, void*d) {
	byte &cell = getCell_unsafe((int)f);
	byte tmpCell = cell;
	int tmp = ((regW & 0xFF) + (cell & 0xFF));
	if (tmp & 0x0100)ram[BYTE_C] |= BIT_C;
	else ram[BYTE_C] &= ~BIT_C;
	if (((regW & 0x0F) + (cell & 0x0F)) & 0x10)ram[BYTE_DC] |= BIT_DC;
	else ram[BYTE_DC] &= ~BIT_DC;
	tmp &= 0xFF;
	if (tmp)ram[BYTE_Z] &= ~BIT_Z;
	else ram[BYTE_Z] |= BIT_Z;
	if (!d)regW = tmp & 0xFF;
	else cell = tmp & 0xFF;
	aktCode++;
	return 1;
}
int Backend::ANDWF(void*f, void*d) {
	byte &cell = getCell_unsafe((int)f);
	int tmp = (regW & cell) & 0xFF;
	if (tmp)ram[BYTE_Z] &= ~BIT_Z;
	else ram[BYTE_Z] |= BIT_Z;
	if (!d)regW = tmp;
	else cell = tmp;
	aktCode++;
	return 1;
}
int Backend::CLRF(void*f, void*ign) { 
	ram[BYTE_Z] |= BIT_Z;
	getCell_unsafe((byte)f) = 0;
	aktCode++;
	return 1;
}
int Backend::CLRW(void*ign1, void*ign2) {
	ram[BYTE_Z] |= BIT_Z;
	regW = 0;
	aktCode++;
	return 1;
}
int Backend::COMF(void*f, void*d) { 
	if (d == nullptr) {
		regW = ~(getCell_unsafe((int)f));
		if (regW == 0)ram[BYTE_Z] |= BIT_Z;
		else ram[BYTE_Z] &= ~BIT_Z;
	}
	else {
		byte& cell = getCell_unsafe((int)f);
		cell = ~cell;
		if (cell == 0)ram[BYTE_Z] |= BIT_Z;
		else ram[BYTE_Z] &= ~BIT_Z;
	}
	aktCode++;
	return 1;	
}
int Backend::DECF(void*f, void*d) {
	if (d == nullptr) {
		regW = getCell_unsafe((int)f) - 1;
		if (regW == 0)ram[BYTE_Z] |= BIT_Z;
		else ram[BYTE_Z] &= ~BIT_Z;
	}
	else { 
		byte& cell = getCell_unsafe((int)f); 
		cell--;
		if (cell == 0)ram[BYTE_Z] |= BIT_Z;
		else ram[BYTE_Z] &= ~BIT_Z;
	}
	aktCode++;
	return 1;
}
int Backend::DECFSZ(void*f, void*d) { 
	if (d == nullptr) { 
		regW = getCell_unsafe((int)f) - 1; 
		if (regW == 0) {
			aktCode += 2;
			return 2;
		}
		aktCode++;
		return 1;
	}
	else {
		byte& cell = getCell_unsafe((int)f);
		cell--;
		if (cell == 0) {
			aktCode += 2;
			return 2;
		}
		aktCode++;
		return 1;
	}
}
int Backend::INCF(void*f, void*d) {
	if (d == nullptr) {
		regW = getCell_unsafe((int)f) + 1;
		if (regW == 0)ram[BYTE_Z] |= BIT_Z;
		else ram[BYTE_Z] &= ~BIT_Z;
	}
	else {
		byte& cell = getCell_unsafe((int)f);
		cell++;
		if (cell == 0)ram[BYTE_Z] |= BIT_Z;
		else ram[BYTE_Z] &= ~BIT_Z;
	}
	aktCode++;
	return 1;
}
int Backend::INCFSZ(void*f, void*d) {
	if (d == nullptr) {
		regW = getCell_unsafe((int)f) + 1;
		if (regW == 0) {
			aktCode += 2;
			return 2;
		}
		aktCode++;
		return 1;
	}
	else {
		byte& cell = getCell_unsafe((int)f);
		cell++;
		if (cell == 0) {
			aktCode += 2;
			return 2;
		}
		aktCode++;
		return 1;
	}
}
int Backend::IORWF(void*f, void*d) {

	if (d == nullptr) {
		regW |= getCell_unsafe((int)f);
		if (!regW)ram[BYTE_Z] |= BIT_Z;
		else ram[BYTE_Z] &= ~BIT_Z;
	}
	else {
		byte& cell = getCell_unsafe((int)f);
		cell |= regW;
		if (!cell)ram[BYTE_Z] |= BIT_Z;
		else ram[BYTE_Z] &= ~BIT_Z;
	}
	aktCode++;
	return 1;
}
int Backend::MOVF(void*f, void*d) { 
	if (d == nullptr) {
		regW = getCell_unsafe((int)f);
		if (regW == 0)ram[BYTE_Z] |= BIT_Z;
		else ram[BYTE_Z] &= ~BIT_Z;
	}
	else if (getCell_unsafe((int)f) == 0)ram[BYTE_Z] |= BIT_Z;
	else ram[BYTE_Z] &= ~BIT_Z;
	aktCode++;
	return 1;
}
int Backend::MOVWF(void*f, void*ign) {
	getCell_unsafe((int)f) = regW;
	aktCode++;
	return 1;
}
int Backend::NOP(void*ign1, void*ign2) {
	aktCode++;
	return 1;
}
int Backend::RLF(void*f, void*d) { 
	byte& cell = getCell_unsafe((int)f);
	int tmp = ram[BYTE_C] & BIT_C;
	if (cell & 0x80) ram[BYTE_C] |= BIT_C;
	else ram[BYTE_C] &= ~BIT_C;
	if (d == nullptr) {
		regW = cell << 1;
		if (tmp) regW |= 0x01;
		else regW &= ~0x01;
	}
	else {
		cell <<= 1;
		if (tmp) cell |= 0x01;
		else cell &= ~0x01;
	}
	aktCode++;
	return 1;
}
int Backend::RRF(void*f, void*d) {
	byte& cell = getCell_unsafe((int)f);
	int tmp = ram[BYTE_C] & BIT_C;
	if (cell & 0x01) ram[BYTE_C] |= BIT_C;
	else ram[BYTE_C] &= ~BIT_C;
	if (d == nullptr) {
		regW = cell >> 1;
		if (tmp) regW |= 0x80;
		else regW &= ~0x80;
	}
	else {
		cell >>= 1;
		if (tmp) cell |= 0x80;
		else cell &= ~0x80;
	}
	aktCode++;
	return 1;
}
int Backend::SUBWF(void*f, void*d) {
	byte &cell = getCell_unsafe((int)f);
	byte tmpCell = cell;
	int tmp = (((int)(cell & 0xFF)) - ((int)(regW & 0xFF)));
	if (tmp >= 0)ram[BYTE_C] |= BIT_C;
	else ram[BYTE_C] &= ~BIT_C;
	if ((((char)(tmpCell & 0x0F)) - ((char)(regW & 0x0F))) >= 0)ram[BYTE_DC] |= BIT_DC;
	else ram[BYTE_DC] &= ~BIT_DC;
	tmp &= 0xFF;
	if (tmp)ram[BYTE_Z] &= ~BIT_Z;
	else ram[BYTE_Z] |= BIT_Z;
	if (!d)regW = tmp;
	else cell = tmp;
	aktCode++;
	return 1;
}
int Backend::SWAPF(void*f, void*d) { 
	byte& cell = getCell_unsafe((int)f);
	byte tmp = ((cell & 0xF0) >> 4) | ((cell & 0x0F) << 4);
	if (!d)regW = tmp;
	else cell = tmp;
	aktCode++;
	return 1;
}
int Backend::XORWF(void*f, void*d) {
	byte &cell = getCell_unsafe((int)f);
	int tmp = (regW ^ cell) & 0xFF;
	if (tmp)ram[BYTE_Z] &= ~BIT_Z;
	else ram[BYTE_Z] |= BIT_Z;
	if (!d)regW = tmp;
	else cell = tmp;
	aktCode++;
	return 1;
}

int Backend::BCF(void*f, void*b) {
	getCell_unsafe((char)f) &= ~(1 << (char)b);
	aktCode++;
	return 1;
}
int Backend::BSF(void*f, void*b) {
	getCell_unsafe((char)f) |= (1 << (char)b);
	aktCode++;
	return 1;
}
int Backend::BTFSC(void*f, void*b) {
	if (getCell_unsafe((char)f) & (1 << (char)b)) {	//set -> no skip
		aktCode++;
		return 1;
	}
	else {	// skip
		aktCode += 2;
		return 2;
	}
}
int Backend::BTFSS(void*f, void*b) {
	if (getCell_unsafe((char)f) & (1 << (char)b)) {	//set -> skip
		aktCode += 2;
		return 2;
	}
	else {	// no skip
		aktCode++;
		return 1;
	}
}

int Backend::ADDLW(void*k, void*ign) {
	int tmpW = ((regW & 0xFF) + ((char)k & 0xFF));
	if (tmpW & 0x0100)ram[BYTE_C] |= BIT_C;
	else ram[BYTE_C] &= ~BIT_C;
	if (((regW & 0x0F) + ((char)k & 0x0F)) & 0x10)ram[BYTE_DC] |= BIT_DC;
	else ram[BYTE_DC] &= ~BIT_DC;
	regW = tmpW & 0xFF;
	if (regW)ram[BYTE_Z] &= ~BIT_Z;
	else ram[BYTE_Z] |= BIT_Z;
	aktCode++;
	return 1;
}
int Backend::ANDLW(void*k, void*ign) {
	regW = (regW & (char)k) & 0xFF;
	if (regW)ram[BYTE_Z] &= ~BIT_Z;
	else ram[BYTE_Z] |= BIT_Z;
	aktCode++;
	return 1;
}
int Backend::CALL(void*k, void*ign) {
	STACK* newAdress = (STACK*)malloc(sizeof(STACK));
	if (newAdress == nullptr) {
		lastError = MEMORY_MISSING;
		return -1;
	}
	newAdress->jumpTo = aktCode + 1;
	newAdress->next = functionStack;
	functionStack = newAdress;
	aktCode = &(code->code[(int)k]);
	if (stopAtStackZero >= 0)stopAtStackZero++;
	return 2;
}
int Backend::CLRWDT(void*ign1, void*ign2) {
	//TO & PD
	ram[0x03] |= 0x18;
	reset_wdt();
	aktCode++;
	return 1;
}
int Backend::GOTO(void*k, void*ign) { 
	aktCode = &(code->code[(int)k]);
	return 2;
}
int Backend::IORLW(void*k, void*ign) { 
	regW = ((regW & 0xFF) | ((char)k & 0xFF));
	if (!regW)ram[BYTE_Z] &= ~BIT_Z;
	else ram[BYTE_Z] |= BIT_Z;
	aktCode++;
	return 1;
}
int Backend::MOVLW(void*k, void*ign) { 
	regW = ((char)k & 0xFF);
	aktCode++;
	return 1;
}
int Backend::RETFIE(void*ign1, void*ign2) {
	if (functionStack == nullptr) {
		lastError = FUNCTIONSTACK_EMPTY;
		MSGLEN();
		return -1;
	}
	STACK* oldStack = functionStack;
	aktCode = functionStack->jumpTo;
	functionStack = functionStack->next;
	free(oldStack);
	if (stopAtStackZero > 0)stopAtStackZero--;
	ram[0x0B] |= 0x80;
	return 2;
}
int Backend::RETLW(void*k, void*ign) {
	if (functionStack == nullptr) {
		lastError = FUNCTIONSTACK_EMPTY;
		MSGLEN();
		return -1;
	}
	STACK* oldStack = functionStack;
	aktCode = functionStack->jumpTo;
	functionStack = functionStack->next;
	free(oldStack);
	if (stopAtStackZero > 0)stopAtStackZero--;
	regW = ((char)k);
	return 2;
}
int Backend::RETURN(void*ign1, void*ign2) {
	if (functionStack == nullptr) {
		lastError = FUNCTIONSTACK_EMPTY;
		MSGLEN();
		return -1;
	}
	STACK* oldStack = functionStack;
	aktCode = functionStack->jumpTo;
	functionStack = functionStack->next;
	free(oldStack);
	if (stopAtStackZero > 0)stopAtStackZero--;
	return 2; 
}
int Backend::SLEEP(void*ign1, void*ign2) { 
	sleep = true;
	//TO & PD
	ram[0x03] |= 0x10;
	ram[0x03] &= 0xF7;
	reset_wdt();
	aktCode++;
	return 1;
}
int Backend::SUBLW(void*k, void*ign) { 
	int tmpW = (((char)k & 0xFF) - (regW & 0xFF));
	if (tmpW >= 0)ram[BYTE_C] |= BIT_C;
	else ram[BYTE_C] &= ~BIT_C;
	if ((((char)((char)k & 0x0F)) - ((char)(regW & 0x0F))) >= 0)ram[BYTE_DC] |= BIT_DC;
	else ram[BYTE_DC] &= ~BIT_DC;
	regW = tmpW & 0xFF;
	if (tmpW)ram[BYTE_Z] &= ~BIT_Z;
	else ram[BYTE_Z] |= BIT_Z;
	aktCode++;
	return 1;
}
int Backend::XORLW(void*k, void*ign) { 
	regW = ((regW & 0xFF) ^ ((char)k & 0xFF));
	if (regW)ram[BYTE_Z] &= ~BIT_Z;
	else ram[BYTE_Z] |= BIT_Z;
	aktCode++;
	return 1;
}

void Backend::run_in_other_thread(byte modus)
{
	Backend *b=this;
	LOCK_MUTEX(m_terminated);
	terminated = false;
	UNLOCK_MUTEX(m_terminated);

	LOCK_MUTEX(m_lastError);
	errorInThreadHappend = false;
	UNLOCK_MUTEX(m_lastError);

	if (modus != MOD_STEP) {
		LOCK_MUTEX(m_isRunning);
		isRunning = true;
		UNLOCK_MUTEX(m_isRunning);

		switch (modus){
		case MOD_STEP_OVER:
			stopAtStackZero = 0;
			break;
		case MOD_STEP_OUT:
			stopAtStackZero = 1;
			break;
		case MOD_STANDARD:
			stopAtStackZero = -1;
			break;
#ifdef _DEBUG
		default:
			PRINTF1("MODUS (= %d ) ist nicht definiert!\n", modus);
#endif
		}
		
	}
	else {
		LOCK_MUTEX(m_isRunning);
		isRunning = false;
		UNLOCK_MUTEX(m_isRunning);
	}
	int needTime;

	LOCK_MUTEX(m_isRunning);
	do {
		UNLOCK_MUTEX(m_isRunning);


		auto start_time = std::chrono::high_resolution_clock::now();
		//locks in correct order
		LOCK_MUTEX(m_lastError);
		LOCK_MUTEX(m_regW);
		LOCK_MUTEX(m_run_code);
		LOCK_MUTEX(m_isRunning);
		LOCK_MUTEX(m_ram);
		LOCK_MUTEX(m_eeprom);
		LOCK_MUTEX(m_runtime);
		LOCK_MUTEX(m_wdt);

		//Programmcounter -> Pointer
		int PC = ((ram[PCH] & 0x1F) << 8) | (ram[PCL]);
		if (PC >= UC_SIZE_PROGRAM) {
			reset(RESET_POWER_UP);
			isRunning = false;
			lastError = "Programmcounter ist auﬂerhalb des Programmspeichers!";
			errorInThreadHappend = true;
			MSGLEN();
			PRINTF("Programmcounter ist auﬂerhalb des Programmspeichers!\n");
		}
		aktCode = &(code->code[PC]);

		if(aktCode->breakpoint){
			UNLOCK_MUTEX(m_wdt);
			UNLOCK_MUTEX(m_runtime);
			UNLOCK_MUTEX(m_eeprom);
			UNLOCK_MUTEX(m_ram);
			UNLOCK_MUTEX(m_isRunning);
			UNLOCK_MUTEX(m_run_code);
			UNLOCK_MUTEX(m_regW);

			LOCK_MUTEX(m_isRunning);
			isRunning = false;
			UNLOCK_MUTEX(m_isRunning);
			break;
		}

		DOIF(DEBUG_LVL >= DEBUG_ALL)PRINTF4("EXEC_ASM(%s %02x,%02x)\n\tsleeping: %d\n", Compiler::functionPointerToName(aktCode->function), aktCode->param1, aktCode->param2, sleep);
		//asm-execution
		if (!sleep) {
			needTime = aktCode->function(aktCode->param1, aktCode->param2, this);
		}
		else needTime = 1;
		DOIF(DEBUG_LVL >= DEBUG_ALL)PRINTF1("\tneeded cycles : %d\n", needTime);

		//interupts, timer, eeprom etc...
		//for-loop for imitating more cycle-operations (e.g. the timer has to count more than one time...)
		for (int tmp = 0; tmp < needTime; tmp++) {
			if (!sleep) if (!do_timer()) errorInThreadHappend = true;
			if (!errorInThreadHappend && do_eeprom() && do_interrupts(needTime) && do_wdt());
			else errorInThreadHappend = true;
		}

		//Pointer -> Programmcounter
		PC = (aktCode - &(code->code[0])) % UC_SIZE_PROGRAM;
		ram[PCH] = (PC >> 8) & 0x1F;
		ram[PCL] = PC & 0xFF;

		//clear "read as '0'"-bits...
		ram[0x05] &= 0x1F;
		ram[0x0A] &= 0x1F;
		ram[83] &= 0x1F;
		ram[86] &= 0x1F;

		//stop if error in execution
		if (needTime < 1) {
			reset(RESET_POWER_UP);
			isRunning = false;
			errorInThreadHappend = true;
		}

#ifdef _DEBUG
		if (DEBUG_LVL >= DEBUG_ALL) {
			printf("\n\t00  01  02  03  04  05  06  07\n");
			for (int i = 0; i << 3 < UC_SIZE_RAM; i++) {
				int tmp = i << 3;
				if (tmp + 8 < UC_SIZE_RAM)printf("%02x\t%02x  %02x  %02x  %02x  %02x  %02x  %02x  %02x\n", i,
					ram[tmp], ram[tmp + 1], ram[tmp + 2], ram[tmp + 3], ram[tmp + 4], ram[tmp + 5], ram[tmp + 6], ram[tmp + 7]);
				else printf("%02x\t%02x  %02x  %02x  %02x  %02x  %02x\n\tW:  %02x  #:  %02x  r:  %02x\n\tC   DC  Z\n\t%02x  %02x  %02x\n", i,
					ram[tmp], ram[tmp + 1], ram[tmp + 2], ram[tmp + 3], ram[tmp + 4], ram[tmp + 5], regW, stopAtStackZero & 0xFF, isRunning,
					ram[0x03] & 0x01, ram[0x03] & 0x02, ram[0x03] & 0x04);
			}
			printf("\n\n");
		}
#endif

		//unlocks in correct order
		UNLOCK_MUTEX(m_wdt);
		UNLOCK_MUTEX(m_runtime);
		UNLOCK_MUTEX(m_eeprom);
		UNLOCK_MUTEX(m_ram);
		UNLOCK_MUTEX(m_isRunning);
		UNLOCK_MUTEX(m_run_code);
		UNLOCK_MUTEX(m_regW);

		//wait for 1000us if asm could be executed
		if (!errorInThreadHappend && needTime >= 1) {
			UNLOCK_MUTEX(m_lastError);
			runtime += needTime;
			needTime = (needTime * sleeptime);

			//wait for the correct time
			auto end_time = std::chrono::high_resolution_clock::now();
			auto time = end_time - start_time;
			needTime -= (time.count() / 1000);
			if (needTime <= 0)needTime = 1;
			std::this_thread::sleep_for(std::chrono::microseconds(needTime));
			DOIF(DEBUG_LVL >= DEBUG_MORE)PRINTF1("Sleeping for %d us\n", needTime);
		}
		else UNLOCK_MUTEX(m_lastError);

		LOCK_MUTEX(m_isRunning);
	} while (isRunning && stopAtStackZero != 0);
	UNLOCK_MUTEX(m_isRunning);

	DOIF(DEBUG_LVL >= DEBUG_LESS)PRINTF("STOPPING\n");

	LOCK_MUTEX(m_lastError);
	if (errorInThreadHappend);	//TODO: msg 2 gui that error happened
	UNLOCK_MUTEX(m_lastError);

	LOCK_MUTEX(m_terminated);
	terminated = true;
	UNLOCK_MUTEX(m_terminated);
}
