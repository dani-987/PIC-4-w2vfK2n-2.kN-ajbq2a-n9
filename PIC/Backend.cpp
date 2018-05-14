#include "Backend.h"
#include "Compiler.h"

#include <cstring>
#include <chrono>

#include <stdlib.h>
#include <time.h>

#ifdef _DEBUG

#define DEBUG_NONE	0
#define DEBUG_LESS	1
#define DEBUG_NORM	2
#define DEBUG_MORE	3
#define DEBUG_RAM	4
#define DEBUG_ALL	5

VARDEF(int, DEBUG_LVL, DEBUG_NONE);

#include <Windows.h>	
HANDLE  hConsole;

//komment this out if using GUI....
//#define USE_BACKEND_WITHOUT_GUI

#define COLOR_RAM_HEAD	0xF0
#define COLOR_RAM_NORM	0x70
#define COLOR_RAM_DMGD	0x7C
#define COLOR_RAM_OTHER	0xB9

#define COLOR_STANDARD	0x07

void Backend::set_DEBUG_ONLY_TESTING(int state)
{
	printf("Setting DBG_LVL 2 %d\n", state);
	DEBUG_LVL = state;
}
#endif

#define DAMAGE_GET_BITMAP_BYTE(pos)	(pos >> 3)
#define DAMAGE_GET_BITMAP_BIT(pos)	(1 << (pos & 0x07))


//Komment following if not wanted....
#define USE_RANDOM_VALUES
//#define USE_EEPROM_WRITE_ERRORS		//have to be reworked if activated!

#define NOT_IMPLEMENTED	"Nicht implementiert!"
#define MEMORY_MISSING "Memory is missing."
#define FUNCTIONSTACK_EMPTY	"Fuctionstack is empty! CALL is missing before RETURN, RETLW or RETFIE!"
#define STACK_OVERFLOW "Stackoverflow!"

#define RESET_POWER_UP		0
#define RESET_CLRWDT		1
#define RESET_MCLR			2
#define RESET_INTERUPT		3

#define MSGLEN()	(lastErrorLen = (std::strlen(lastError)+1))

const static byte spiegelMap[] = {
0x99,	//0x08
0x3F,	//0x0F
0xFF,	//0x18
0xFF,	//0x1F
0xFF,	//0x28
0xFF,	//0x2F
0xFF,	//0x38
0xFF,	//0x3F
0xFF,	//0x48
0xFF,	//0x4F
0xFF,	//0x58
0xFF,	//0x5F
0xFF,	//0x68
0xFF,	//0x6F
0xFF,	//0x78
0xFF,	//0x7F
0xFF,	//0x88
0xFF,	//0x8F
0xFF,	//0x98
0xFF,	//0x9F
0xFF,	//0xA8
0xFF,	//0xAF
0xFF,	//0xB8
0xFF,	//0xBF
0xFF,	//0xC8
0xFF,	//0xCF
0xFF,	//0xD8
0xFF,	//0xDF
0xFF,	//0xE8
0xFF,	//0xEF
0xFF,	//0xF8
0xFF,	//0xFF
};

void call_backup_in_other_thread(void* _callInOtherThread) {
	struct call_in_other_thread_s* callInOtherThread = (struct call_in_other_thread_s*)_callInOtherThread;
	DOIF(callInOtherThread->modus > 4 || callInOtherThread->modus < 0)
		PRINTF1("WRONG MODUS! (%d)\n", callInOtherThread->modus);
	callInOtherThread->backend->run_in_other_thread(callInOtherThread->modus);
	delete(_callInOtherThread);
}

byte & Backend::getCell_unsafe(byte pos, bool dmg)
{
	if (pos == 0x00) {	//indirect addressing
		pos = ram[0x04];
	}
	//rest
	if (pos == 0x07 || pos > 0x4F || pos == 0x00 ) {
		tmp = 0;
		return tmp;
	}
	else if(pos == 0x02){
		if(dmg){
			damageByte(pos);
			written_to_PCL = true;
			ram[pos]++;
		}
		return ram[pos]; 
	}
	else if (pos >= 0x0A || (pos > 0x02 && pos <= 0x04)) { 
		if(dmg)damageByte(pos);
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
		else if (ram[0x03] & 0x20) {
			if(dmg)damageByte(82 + pos);
			return ram[82 + pos];
		}
		else {
			if(dmg)damageByte(pos);
			return ram[pos];
		}
	}
}

void Backend::reset(byte resetType)
{
	// if a POWER_UP_RESET accures, the uC cannot be in sleep, because power was off...
	if(!sleep && resetType == RESET_POWER_UP)sleep = false;

	if(!sleep){
		//interrupt will only wakeup, if not sleeping return here....
		if(resetType == RESET_INTERUPT) return;

		//clear stack!
		while (functionStack != nullptr) {
			STACK* tmp = functionStack;
			functionStack = functionStack->next;
			free(tmp);
	#ifdef _DEBUG
			tmp = (STACK*) (0xFF00000 | __LINE__);
	#endif
		}
		stackSize = 0;

		//reset wdt
		wdt_int_accured = false;
		reset_wdt();

		//reset neccessary bytes in ram
		byte tmp = ram[90];

		ram[0x02] = 0x00;
		if(resetType != RESET_MCLR)ram[0x03] &= 0x07;
		else ram[0x03] &= 0x1F;
		ram[0x05] &= 0x1F;
		ram[0x0A] = 0;
		ram[0x0B] &= 0x01 ;

		ram[83] = 0xFF;
		ram[84] = 0x00;
		ram[87] = 0x1F;
		ram[88] = 0xFF;
		ram[90] = 0x00;

		if(code)aktCode = &code->code[0];

		this->ram_rb_cpy = ram[0x06];
		lastInput = ram[0x05] & 0x10;

		if ((tmp & 0x04) && eeprom_wr) {
			ram[90] |= 0x80;
		}
		eeprom_wr = false;
	
		written_to_PCL = true;
		sleep = false;
		prescaler_timer = 0;
		time_eeprom_error_write = 0;
		
		switch (resetType) {
		case RESET_POWER_UP:
			ram[0x03] |= 0x18;
			break;
		case RESET_CLRWDT:
			ram[0x03] |= 0x08;
			break;
		case RESET_MCLR:
			break;
		default:
			break;
		}
	}
	else{
		sleep = false;

		ram[0x03] &= 0x07;
		switch (resetType) {
		case RESET_CLRWDT:
			ram[0x03] |= 0x08;
			break;
		case RESET_MCLR:
			ram[0x03] |= 0x08;
			break;
		case RESET_INTERUPT:
			ram[0x03] |= 0x10;
			break;
		default:
			break;
		}
	}
}

void Backend::stopAndWait()
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
		LOCK_MUTEX(m_run_code);
		LOCK_MUTEX(m_isRunning);
	}
	if (uC != nullptr) {
		if(uC->joinable())uC->join();
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
		reset(RESET_CLRWDT);
		if (sleep) {
			wdt_int_accured = false;

			goto DO_INTERRUPT;
		}
		needTime = 1;
		return true;
	}
	byte _tmpByte = ram[0x0B];
	//RBIF in INTCON
	if ((this->ram_rb_cpy & 0xF0 & ram[88]) != (ram[0x06] & 0xF0 & ram[88])) { ram[0x0B] |= 0x01; damageByte(0x0B); }
	//INTF if RB0/INT (check if on change or on set...)
	if (((this->ram_rb_cpy & 0x01) != (ram[0x06] & 0x01)) && ((ram[0x06] & 0x01) == ((ram[83] >> 6) & 0x01))) { ram[0x0B] |= 0x02;  damageByte(0x0B);}
	this->ram_rb_cpy = ram[0x06];
	_tmpByte = ram[0x0B];
	//if GIE or sleeping have to check interrupts....
	if (ram[0x0B] & 0x80 || sleep) {
		//T0IF & T0IE
		byte tam0x0B = ram[0x0B];
		if ((ram[0x0B] & 0x24) == 0x24) {
			reset(RESET_INTERUPT);
			goto DO_INTERRUPT;
		}
		//INTF & INTE
		if ((ram[0x0B] & 0x12) == 0x12) {
			reset(RESET_INTERUPT);
			goto DO_INTERRUPT;
		}
		//RBIF & RBIE
		if ((ram[0x0B] & 0x09) == 0x09) {
			reset(RESET_INTERUPT);
			goto DO_INTERRUPT;
		}
		//EEIF & EEIE
		if ((ram[0x0B] & 0x40) && (ram[90] & 0x10)) {
			reset(RESET_INTERUPT);
			goto DO_INTERRUPT;
		}
	}
	return true;
DO_INTERRUPT:
	DOIF(DEBUG_LVL >= DEBUG_RAM)PRINTF("Interrupting! GOING to Addr. 0x04\n");
	//wake-up if sleeping
	sleep = false;
	//if sleeping, GIE decises, if goto into interrupt routine or not and continue code; if not sleeping, GIE is enabled here
	if (ram[0x0B] & 0x80) {
		//unset GIE
		ram[0x0B] &= ~0x80;
		damageByte(0x0B);
		//returnAddr -> Stack
		STACK* newAdress = (STACK*)malloc(sizeof(STACK));
		if (newAdress == nullptr) {
			lastError = MEMORY_MISSING;
			MSGLEN();
			return -1;
		}
		stackSize++;
		newAdress->jumpTo = aktCode;
		newAdress->next = functionStack;
		functionStack = newAdress;
		//PC = 0x04
		aktCode = &(code->code[0x04]);
		if (stopAtStackZero >= 0)stopAtStackZero++;
		//delay
		needTime = 4;
	}
	return true;
}

bool Backend::do_timer()
{
	//sync at 2nd cycle, internal or external clock?
	if(timer_sync_written){
		byte last_timer_sync = 0;
		//sync
		if(timer_written){
			last_timer_sync = ram[0x01];
			timer_sync_written = true;
		}
		else timer_sync_written = false;
		ram[0x01] = timer_sync;
		timer_sync = last_timer_sync;
	}
	else {	
		//sync
		if(timer_written){
			timer_sync = ram[0x01];
			timer_sync_written = true;
		}
		else if(ram[83] & 0x10){//0x53
			//input changed
			if (lastInput != (ram[0x05] & 0x10)) {
				lastInput ^= 0x10;
				//test if correct change happend (rising / falling edge [see T0SE])
				if (lastInput != (ram[83] & 0x10)) {
					//with prescaler?
					if (ram[83] & 0x04) {
						ram[0x01]++;
						damageByte(0x01);
						if ((ram[0x01] == 0)) { ram[0x0B] |= 0x04;  damageByte(0x0B);}	//set interrupt
					}
					else {
						prescaler_timer++;
						int prescaler = (0x02 << (ram[83] & 0x07));
						if (prescaler_timer >= prescaler) {
							prescaler_timer = 0;
							ram[0x01]++;
							damageByte(0x01);
							if ((ram[0x01] == 0)) { ram[0x0B] |= 0x04;  damageByte(0x0B);}	//set interrupt
						}
					}
				}
			}
		}
		else {
			//with prescaler?
			if (ram[83] & 0x04) {
				ram[0x01]++; 
				damageByte(0x01);
				if ((ram[0x01] == 0)) { ram[0x0B] |= 0x20;  damageByte(0x0B); }	//set interrupt
			}
			else {
				prescaler_timer++;
				int prescaler = (0x02 << (ram[83] & 0x07));
				if (prescaler_timer >= prescaler) {
					prescaler_timer = 0;
					ram[0x01]++;
					damageByte(0x01);
					if ((ram[0x01] == 0)) { ram[0x0B] |= 0x04; damageByte(0x0B); }	//set interrupt
				}
			}
		}
	}
	timer_written = false;
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
		//if (ram[90] & 0x04) {
			eeprom_write_time-=sleeptime;
			if (eeprom_write_time <= 0) {
				//write eeprom
				ram[90] &= ~0x02;
				eeprom_wr = false;
				ram[90] |= 0x10;
				damageByte(90);
#ifdef USE_EEPROM_WRITE_ERRORS
				if (eeprom_write_time < 5) {	//cause a biterror at eeprom write because the eeprom is used much
					byte tmp = eeprom[eeprom_wr_addr];
					eeprom[eeprom_wr_addr] &= rand() & 0xFF;
					if (eeprom[eeprom_wr_addr] != tmp) {	//biterror at eeprom write was caused
						eeprom_write_time = 22;
						DOIF(DEBUG_LVL >= DEBUG_NORM)PRINTF("EEPROM WRITE BITERROR\n");
					}
					//else;	//biterror donot changed value....
				}
#endif
			}
		//}
	}
	else if (ram[90] & 0x04 && ram[90] & 0x02) {
		if (eeprom_write_state == 2) {
			eeprom_write_state = 0;
			eeprom_wr = true;
			if (time_eeprom_error_write > 22)time_eeprom_error_write = 0;
			else time_eeprom_error_write - 22;

			eeprom_wr_addr = ram[0x09];
			eeprom[eeprom_wr_addr] = ram[0x08];
			eeprom_write_time = 8000 + rand() % 12000;	//typical write time: 10ms, max: 20ms (one operation: 1ms)
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
		damageByte(90);

		//do eeprom read
		ram[0x08] = eeprom[ram[0x09]];
		damageByte(0x08);
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
			wdt_timer-=sleeptime;
		}
	}
	else {
		wdt_timer-=sleeptime;
	}
	if (wdt_timer <= 0) {
		wdt_int_accured = true;
		reset_wdt();
		 ram[89] &= 0xE7;
		 ram[89] |= 0x10;
	}
	return true;
}

void Backend::reset_wdt()
{
	wdt_prescaler = 0;
	wdt_timer = 175000 + (rand() % 10000);	//wdt is _ABOUT_ 18ms
}

void Backend::damageByte(size_t pos)
{
	DOIF(pos > UC_SIZE_RAM)PRINTF2("damageByte(size_t pos = %d), pos is too big (UC_SIZE_RAM = %d)\n", pos, UC_SIZE_RAM);
	if(pos == 0x01)timer_written = true;
	bitmap8_t tmpBitmap = DAMAGE_GET_BITMAP_BIT(pos);
	size_t tmpPos = DAMAGE_GET_BITMAP_BYTE(pos);
	if (reloadCalled) { 
		reloadCalled = false; 
		Fl::awake(gui_int_update, gui);
	}
	if (!(damage[tmpPos] & tmpBitmap)) {
		countDamaged++;
		damage[tmpPos] |= tmpBitmap;
	}
}

Backend::Backend(GUI* gui)
{
	lastError = "Kein Fehler";
	lastErrorLen = MSGLEN();
	srand(time(NULL));
	this->gui = gui;
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
	reloadCalled = false;
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
	reset(RESET_POWER_UP);
#ifdef _DEBUG
	hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
#endif
}

Backend::~Backend()
{
	stopAndWait();
	free(eeprom);
	free(ram);
#ifdef _DEBUG
		eeprom = (char*) (0xFF00000 | __LINE__);
		ram = (byte*) (0xFF00000 | __LINE__);
#endif
	//if (functionStack != nullptr)free(functionStack);//todo
	if (code != nullptr) Compiler::freeASM(code); 
}

void cpyStr(char*& dst, char* src){
	if(src != nullptr){
		size_t _strlen = strlen(src) + 1;
		dst = (char*)malloc(_strlen);
		memcpy(dst, src, _strlen);
	}
	else dst = nullptr;
}

ASM_TEXT * Backend::GetProgrammText(size_t& anzahl)
{
	LOCK_MUTEX(m_text_code);
	ASM_TEXT *tmp = code->text, *ret = nullptr, *_tmp, *start = nullptr;
	anzahl = 0;
	while(tmp != nullptr){
		_tmp = (ASM_TEXT*)malloc(sizeof(ASM_TEXT));
		if(ret != nullptr)ret->next = _tmp;
		if(start == nullptr) start = ret;
		ret = _tmp;
		cpyStr(ret->asmCode, tmp->asmCode);
		cpyStr(ret->bytecode, tmp->bytecode);
		cpyStr(ret->comment, tmp->comment);
		cpyStr(ret->label, tmp->label);
		cpyStr(ret->lineOfCode, tmp->lineOfCode);
		ret->lineNumber = tmp->lineNumber;
		ret->next = nullptr;
		tmp = tmp->next;
		anzahl++;
	}
	UNLOCK_MUTEX(m_text_code);
	return start;
}

void Backend::FreeProgrammText(ASM_TEXT *& prog)
{
	ASM_TEXT* tmp;
	while(prog != nullptr){
		tmp = prog;
		prog = prog->next;
		if(tmp->asmCode != nullptr)free(tmp->asmCode);
		if(tmp->comment != nullptr)free(tmp->comment);
		if(tmp->label != nullptr)free(tmp->label);
		if(tmp->bytecode != nullptr)free(tmp->bytecode);
		if(tmp->lineOfCode != nullptr)free(tmp->lineOfCode);
		free(tmp);
#ifdef _DEBUG
		tmp = (ASM_TEXT*) (0xFF00000 | __LINE__);
#endif
	}
}

bool Backend::GetNextChangedCell(int & reg, byte & bank)
{
	bitmap8_t tmpBitmap;
	size_t tmpPos;
	LOCK_MUTEX(m_ram);
	//damage of indirect register (is always 0)
	if(posReadingDamage == 0){
		damage[0] &= 0xFE;
		posReadingDamage++;
		byte tmp = getCell_unsafe(0, false);
		tmpBitmap = DAMAGE_GET_BITMAP_BIT(tmp);
		tmpPos = DAMAGE_GET_BITMAP_BYTE(tmp);
		if(damage[0x00] & 0x10 || damage[tmpPos] & tmpBitmap){
				reg = 0;
				bank = 0;
				UNLOCK_MUTEX(m_ram);
				return true;
		}
	}
	while (posReadingDamage <= UC_SIZE_RAM) {
		tmpBitmap = DAMAGE_GET_BITMAP_BIT(posReadingDamage);
		tmpPos = DAMAGE_GET_BITMAP_BYTE(posReadingDamage);
		if (damage[tmpPos] & tmpBitmap) {
			damage[tmpPos] &= ~tmpBitmap;
			if (posReadingDamage >= 82) {
				reg = posReadingDamage - 82;
				bank = 1;
				countDamaged--;
				posReadingDamage++;
				UNLOCK_MUTEX(m_ram);
				return true;
			}
			else {
				reg = posReadingDamage;
				bank = 0;
				countDamaged--;
				posReadingDamage++;
				UNLOCK_MUTEX(m_ram);
				return true;
			}
		}
		posReadingDamage++;
	}
	posReadingDamage = 0;
	UNLOCK_MUTEX(m_ram);
	return false;
}

void Backend::StartedUpdating()
{
	LOCK_MUTEX(m_ram);
	reloadCalled = true;
	UNLOCK_MUTEX(m_ram);
}

bool Backend::LoadProgramm(char * c)
{
	bool ret = false;
	LOCK_MUTEX(m_isRunningLocked);
	isRunningLocked = true;
	UNLOCK_MUTEX(m_isRunningLocked);
	stopAndWait();
	Compiler comp;
	ASM* prog = comp.compileFile(c, UC_SIZE_PROGRAM);
	if (prog != nullptr) {
		if (this->code)Compiler::freeASM(this->code);
		this->code = prog;
		this->aktCode = prog->code;
		runtime = 0;
		ret = true;
	}
	else{
		LOCK_MUTEX(m_lastError);
		lastError = comp.getCompilerError();
		MSGLEN();
		UNLOCK_MUTEX(m_lastError);
	}
	LOCK_MUTEX(m_ram);
	for (int i = 0; i < UC_SIZE_RAM; i++) {
#ifdef USE_RANDOM_VALUES
		ram[i] = (byte)(rand() & 0xFF);
#else
		ram[i] = 0;
#endif
	};
	reloadCalled = false;
	UNLOCK_MUTEX(m_ram);
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

bool Backend::StepOut()
{
	return letRun(MOD_STEP_OUT);
}

bool Backend::StepOver()
{
	return letRun(MOD_STEP_OVER);
}

bool Backend::WirdByteGespiegelt(byte pos)
{
	return ((spiegelMap[pos >> 3] & (0x01 << (pos & 0x07))) == 0)?false:true;
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

bool Backend::IsWatchdogEnabled()
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
	if(code == nullptr){
		UNLOCK_MUTEX(m_run_code);

		LOCK_MUTEX(m_lastError);
		lastError = "Code not loaded!";
		MSGLEN();
		PRINTF("Code was not loaded (code == nullptr)\n");
		UNLOCK_MUTEX(m_lastError);
		return -1;
	}
	for (; aktLine < UC_SIZE_PROGRAM && foundLine < textline; aktLine++) {
		if (code->code[aktLine].guiText != nullptr) {
			foundLine = code->code[aktLine].guiText->lineNumber;
		}
	}
	aktLine--;
	if (foundLine < textline) {
		if (code->code[0].guiText != nullptr) {
			code->code[0].breakpoint = true;
			UNLOCK_MUTEX(m_run_code);
			return code->code[0].guiText->lineNumber;
		}
		else {
			UNLOCK_MUTEX(m_run_code);
			return -3;
		}
	}
	else if (foundLine == textline) {
		if (!code->code[aktLine].breakpoint) {
			code->code[aktLine].breakpoint = true;
			UNLOCK_MUTEX(m_run_code);
			return foundLine;
		}
		else {
			code->code[aktLine].breakpoint = false;
			UNLOCK_MUTEX(m_run_code);
			return -2;
		}
	}
	else {
		if (!code->code[aktLine].breakpoint) {
			code->code[aktLine].breakpoint = true;
			UNLOCK_MUTEX(m_run_code);
			return foundLine;
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

void Backend::FreeBreakpoints(Breakpointlist*& list)
{
	Breakpointlist* tmp = list;
	while (tmp != nullptr) {
		tmp = list->next;
		delete(list);
		list = tmp;
	}
	list = nullptr;
}

void Backend::SetCommandSpeed(size_t speed)
{
	LOCK_MUTEX(m_ram);
	sleeptime = speed;
	UNLOCK_MUTEX(m_ram);
}

bool Backend::Reset()
{
	bool isSleeping;
	LOCK_MUTEX(m_ram);
	isSleeping = sleep;
	UNLOCK_MUTEX(m_ram);

	if(!isSleeping)stopAndWait();

	LOCK_MUTEX(m_run_code);
	LOCK_MUTEX(m_ram);
	LOCK_MUTEX(m_eeprom);
	LOCK_MUTEX(m_wdt);

	//changing ram is not necessary....
	reset(RESET_MCLR);

	UNLOCK_MUTEX(m_wdt);
	UNLOCK_MUTEX(m_eeprom);
	UNLOCK_MUTEX(m_ram);
	UNLOCK_MUTEX(m_run_code);
	
	if(!isSleeping){
		LOCK_MUTEX(m_isRunningLocked);
		isRunningLocked = false;
		UNLOCK_MUTEX(m_isRunningLocked);
	}
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
	if (reg < 0 || reg > 0xFF) {
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
	if ((reg & 0x7F) == 0x03) { UNLOCK_MUTEX(m_ram); return tmp; }
	if (bank == 0)ram[0x03] = ram[0x03] & (~0x20);
	else ram[0x03] = ram[0x03] | 0x20;
	ret = getCell_unsafe(reg, false);
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
	if (reg < 0 || reg > 0xFF) {
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
	if ((reg & 0x7F) == 0x03) { ram[0x03] = val; damageByte(0x03); UNLOCK_MUTEX(m_ram); return true; }
	tmp = ram[0x03];
	if (bank == 0)ram[0x03] &= (~0x20);
	else ram[0x03] |= 0x20;
	getCell_unsafe(reg) = val;
	ram[0x03] = tmp;
	
	if(written_to_PCL && code){
		int PC = ((ram[0x0A] & 0x1F) << 8) | (ram[0x02]);
		if (PC >= UC_SIZE_PROGRAM) {
			reset(RESET_POWER_UP);
			isRunning = false;
			lastError = "Programmcounter ist auﬂerhalb des Programmspeichers!";
			errorInThreadHappend = true;
			MSGLEN();
			PRINTF("Programmcounter ist auﬂerhalb des Programmspeichers!\n");
		}
		aktCode = &(code->code[PC]);
	}
	//clear "read as '0'"-bits...
	ram[0x05] &= 0x1F;
	ram[0x0A] &= 0x1F;
	ram[87] &= 0x1F;
	ram[90] &= 0x1F;
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
		return false;
	}
	if (bank > 1 || bank < 0) {
		LOCK_MUTEX(m_lastError);
		lastError = "'bank' muss 1 oder 0 sein!";
		MSGLEN();
		PRINTF1("bool Backend::SetBit(int reg, byte bank, int pos, bool val):\n\t'bank' muss 1 oder 0 sein und ist %d!\n", bank);
		UNLOCK_MUTEX(m_lastError);
		return false;
	}
	if (reg < 0 || reg > 0xFF) {
		LOCK_MUTEX(m_lastError);
		lastError = "'reg' muss zwischen (einschlieﬂlich) 0x00 und 0xFF sein!";
		MSGLEN();
		PRINTF1("bool Backend::SetBit(int reg, byte bank, int pos, bool val):\n\t'reg' muss zwischen (einschlieﬂlich) 0x00 und 0xFF sein und ist %d!\n", reg);
		UNLOCK_MUTEX(m_lastError);
		return false;
	}
#endif
	int tmp;
	LOCK_MUTEX(m_ram);
	if ((reg & 0x7F) == 0x03) { 
		if(val)ram[0x03] |= (1 << pos);
		else ram[0x03] &= ~(1 << pos);
		damageByte(0x03);
		UNLOCK_MUTEX(m_ram); 
		return true; 
	}
	tmp = ram[0x03];
	if (bank == 0)ram[0x03] = ram[0x03] & (~0x20);
	else ram[0x03] = ram[0x03] | 0x20;
	if(val)getCell_unsafe(reg) |= (1 << pos);
	else getCell_unsafe(reg) &= ~(1 << pos);
	ram[0x03] = tmp;
	
	if(written_to_PCL && code){
		int PC = ((ram[0x0A] & 0x1F) << 8) | (ram[0x02]);
		if (PC >= UC_SIZE_PROGRAM) {
			reset(RESET_POWER_UP);
			isRunning = false;
			lastError = "Programmcounter ist auﬂerhalb des Programmspeichers!";
			errorInThreadHappend = true;
			MSGLEN();
			PRINTF("Programmcounter ist auﬂerhalb des Programmspeichers!\n");
		}
		aktCode = &(code->code[PC]);
	}
	//clear "read as '0'"-bits...
	ram[0x05] &= 0x1F;
	ram[0x0A] &= 0x1F;
	ram[87] &= 0x1F;
	ram[90] &= 0x1F;
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
		return false;
	}
	if (bank > 1 || bank < 0) {
		LOCK_MUTEX(m_lastError);
		lastError = "'bank' muss 1 oder 0 sein!";
		MSGLEN();
		PRINTF1("int Backend::GetBit(int reg, byte bank, int pos):\n\t'bank' muss 1 oder 0 sein und ist %d!\n", bank);
		UNLOCK_MUTEX(m_lastError);
		return false;
	}
	if (reg < 0 || reg > 0xFF) {
		LOCK_MUTEX(m_lastError);
		lastError = "'reg' muss zwischen (einschlieﬂlich) 0x00 und 0xFF sein!";
		MSGLEN();
		PRINTF1("int Backend::GetBit(int reg, byte bank, int pos):\n\t'reg' muss zwischen (einschlieﬂlich) 0x00 und 0xFF sein und ist %d!\n", reg);
		UNLOCK_MUTEX(m_lastError);
		return false;
	}
#endif
	int tmp;
	bool val;
	LOCK_MUTEX(m_ram);
	if ((reg & 0x7F) == 0x03) {
		val = (ram[0x03] & (1 << pos));
		UNLOCK_MUTEX(m_ram);
		return val;
	}
	tmp = ram[0x03];
	if (bank == 0)ram[0x03] &= (~0x20);
	else ram[0x03] |= 0x20;
	val = (getCell_unsafe(reg, false) & (1 << pos));
	ram[0x03] = tmp;
	UNLOCK_MUTEX(m_ram);
	return val;
}

int Backend::GetRegW()
{
	int w;
	LOCK_MUTEX(m_regW);
	w = regW;
	UNLOCK_MUTEX(m_regW);
	return w;
}

bool Backend::SetRegW(byte val)
{
	LOCK_MUTEX(m_regW);
	regW = val;
	UNLOCK_MUTEX(m_regW);
	LOCK_MUTEX(m_ram);
	if (reloadCalled) { 
		reloadCalled = false; 
		Fl::awake(gui_int_update, gui);
	}
	UNLOCK_MUTEX(m_ram);
	return true;
}

char * Backend::GetErrorMSG()
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
	if (uC != nullptr && uC->joinable()) { uC->join(); delete(uC); uC = nullptr; }
}

unsigned long long Backend::GetRuntimeIn100ns()
{
	LOCK_MUTEX(m_ram);
	size_t ret = runtime;
	UNLOCK_MUTEX(m_ram);
	return runtime;
}

void Backend::ResetRuntime()
{
	LOCK_MUTEX(m_ram);
	runtime = 0;
	UNLOCK_MUTEX(m_ram);
}

size_t Backend::GetPC()
{
	size_t ret;
	LOCK_MUTEX(m_run_code);
	if(code == nullptr)ret = 0;
	else ret = (aktCode - &code->code[0]);
	UNLOCK_MUTEX(m_run_code);
	return ret;
}

int Backend::GetAktualCodePosition()
{
	if(code == nullptr || aktCode == nullptr || aktCode->guiText == nullptr) return 0;
	return aktCode->guiText->lineNumber;
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
	byte &cell = getCell_unsafe((int)f, d);
	byte tmpCell = cell;
	int tmp = ((regW & 0xFF) + (cell & 0xFF));
	if (tmp & 0x0100)ram[BYTE_C] |= BIT_C;
	else ram[BYTE_C] &= ~BIT_C;
	if (((regW & 0x0F) + (cell & 0x0F)) & 0x10)ram[BYTE_DC] |= BIT_DC;
	else ram[BYTE_DC] &= ~BIT_DC;
	tmp &= 0xFF;
	if (tmp)ram[BYTE_Z] &= ~BIT_Z;
	else ram[BYTE_Z] |= BIT_Z;
	if (!d) regW = tmp & 0xFF;
	else cell = tmp & 0xFF;
	damageByte(BYTE_Z);
	aktCode++;
	return 1;
}
int Backend::ANDWF(void*f, void*d) {
	byte &cell = getCell_unsafe((int)f, d);
	int tmp = (regW & cell) & 0xFF;
	if (tmp)ram[BYTE_Z] &= ~BIT_Z;
	else ram[BYTE_Z] |= BIT_Z;
	if (!d) regW = tmp;
	else cell = tmp;
	damageByte(BYTE_Z);
	aktCode++;
	return 1;
}
int Backend::CLRF(void*f, void*ign) { 
	ram[BYTE_Z] |= BIT_Z;
	damageByte(BYTE_Z);
	getCell_unsafe((byte)f) = 0;
	aktCode++;
	return 1;
}
int Backend::CLRW(void*ign1, void*ign2) {
	ram[BYTE_Z] |= BIT_Z;
	damageByte(BYTE_Z);
	regW = 0; 
	aktCode++;
	return 1;
}
int Backend::COMF(void*f, void*d) { 
	if (d == nullptr) {
		regW = ~(getCell_unsafe((int)f, false));
		if (regW == 0)ram[BYTE_Z] |= BIT_Z;
		else ram[BYTE_Z] &= ~BIT_Z;
		damageByte(BYTE_Z);
	}
	else {
		byte& cell = getCell_unsafe((int)f);
		cell = ~cell;
		if (cell == 0)ram[BYTE_Z] |= BIT_Z;
		else ram[BYTE_Z] &= ~BIT_Z;
		damageByte(BYTE_Z);
	}
	aktCode++;
	return 1;	
}
int Backend::DECF(void*f, void*d) {
	if (d == nullptr) {
		regW = getCell_unsafe((int)f, false) - 1;
		if (regW == 0)ram[BYTE_Z] |= BIT_Z;
		else ram[BYTE_Z] &= ~BIT_Z;
	}
	else { 
		byte& cell = getCell_unsafe((int)f); 
		cell--;
		if (cell == 0)ram[BYTE_Z] |= BIT_Z;
		else ram[BYTE_Z] &= ~BIT_Z;
	}
	damageByte(BYTE_Z);
	aktCode++;
	return 1;
}
int Backend::DECFSZ(void*f, void*d) { 
	if (d == nullptr) { 
		regW = getCell_unsafe((int)f, false) - 1; 
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
		regW = getCell_unsafe((int)f, false) + 1;
		if (regW == 0)ram[BYTE_Z] |= BIT_Z;
		else ram[BYTE_Z] &= ~BIT_Z;
	}
	else {
		byte& cell = getCell_unsafe((int)f);
		cell++;
		if (cell == 0)ram[BYTE_Z] |= BIT_Z;
		else ram[BYTE_Z] &= ~BIT_Z;
	}
	damageByte(BYTE_Z);
	aktCode++;
	return 1;
}
int Backend::INCFSZ(void*f, void*d) {
	if (d == nullptr) {
		regW = getCell_unsafe((int)f, false) + 1;
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
		regW |= getCell_unsafe((int)f, false);
		if (!regW)ram[BYTE_Z] |= BIT_Z;
		else ram[BYTE_Z] &= ~BIT_Z;
	}
	else {
		byte& cell = getCell_unsafe((int)f);
		cell |= regW;
		if (!cell)ram[BYTE_Z] |= BIT_Z;
		else ram[BYTE_Z] &= ~BIT_Z;
	}
	damageByte(BYTE_Z);
	aktCode++;
	return 1;
}
int Backend::MOVF(void*f, void*d) { 
	if (d == nullptr) {
		regW = getCell_unsafe((int)f, false);
		if (regW == 0)ram[BYTE_Z] |= BIT_Z;
		else ram[BYTE_Z] &= ~BIT_Z;
	}
	else if (getCell_unsafe((int)f) == 0)ram[BYTE_Z] |= BIT_Z;
	else ram[BYTE_Z] &= ~BIT_Z;
	damageByte(BYTE_Z);
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
	byte& cell = getCell_unsafe((int)f, d);
	int tmp = ram[BYTE_C] & BIT_C;
	if (cell & 0x80) ram[BYTE_C] |= BIT_C;
	else ram[BYTE_C] &= ~BIT_C;
	damageByte(BYTE_C);
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
	damageByte(BYTE_C);
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
	byte &cell = getCell_unsafe((int)f, d);
	byte tmpCell = cell;
	int tmp = (((int)(cell & 0xFF)) - ((int)(regW & 0xFF)));
	if ((tmp >= 0) && (regW != 0))ram[BYTE_C] |= BIT_C;
	else ram[BYTE_C] &= ~BIT_C;
	if (((((char)(tmpCell & 0x0F)) - ((char)(regW & 0x0F))) >= 0) && ((regW & 0xFF) != 0))ram[BYTE_DC] |= BIT_DC;
	else ram[BYTE_DC] &= ~BIT_DC;
	tmp &= 0xFF;
	if (tmp)ram[BYTE_Z] &= ~BIT_Z;
	else ram[BYTE_Z] |= BIT_Z;
	if (!d)regW = tmp;
	else cell = tmp;
	damageByte(BYTE_Z);
	aktCode++;
	return 1;
}
int Backend::SWAPF(void*f, void*d) { 
	byte& cell = getCell_unsafe((int)f,d);
	byte tmp = ((cell & 0xF0) >> 4) | ((cell & 0x0F) << 4);
	if (!d)regW = tmp;
	else cell = tmp;
	aktCode++;
	return 1;
}
int Backend::XORWF(void*f, void*d) {
	byte &cell = getCell_unsafe((int)f, d);
	int tmp = (regW ^ cell) & 0xFF;
	if (tmp)ram[BYTE_Z] &= ~BIT_Z;
	else ram[BYTE_Z] |= BIT_Z;
	if (!d)regW = tmp;
	else cell = tmp;
	damageByte(BYTE_Z);
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
	if (getCell_unsafe((char)f, false) & (1 << (char)b)) {	//set -> no skip
		aktCode++;
		return 1;
	}
	else {	// skip
		aktCode += 2;
		return 2;
	}
}
int Backend::BTFSS(void*f, void*b) {
	if (getCell_unsafe((char)f, false) & (1 << (char)b)) {	//set -> skip
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
	damageByte(BYTE_Z);
	aktCode++;
	return 1;
}
int Backend::ANDLW(void*k, void*ign) {
	regW = (regW & (char)k) & 0xFF;
	if (regW)ram[BYTE_Z] &= ~BIT_Z;
	else ram[BYTE_Z] |= BIT_Z;
	damageByte(BYTE_Z);
	aktCode++;
	return 1;
}
int Backend::CALL(void*k, void*ign) {
	if(stackSize < STACK_SIZE){
		STACK* newAdress = (STACK*)malloc(sizeof(STACK));
		if (newAdress == nullptr) {
			lastError = MEMORY_MISSING;
			MSGLEN();
			return -1;
		}
		newAdress->jumpTo = aktCode + 1;
		newAdress->next = functionStack;
		functionStack = newAdress;
		aktCode = &(code->code[((int)k & 0x07FF) | (((int)ram[0x0A] & 0x18) << 8)]);
		if (stopAtStackZero >= 0)stopAtStackZero++;
		stackSize++;
		return 2;
	}
	lastError = STACK_OVERFLOW;
	MSGLEN();
	return -1;
}
int Backend::CLRWDT(void*ign1, void*ign2) {
	//TO & PD
	ram[0x03] |= 0x18;
	damageByte(0x03);
	reset_wdt();
	aktCode++;
	return 1;
}
int Backend::GOTO(void*k, void*ign) { 
	aktCode = &(code->code[((int)k & 0x07FF) | (((int)ram[0x0A] & 0x0018) << 8)]);
	return 2;
}
int Backend::IORLW(void*k, void*ign) { 
	regW = ((regW & 0xFF) | ((char)k & 0xFF));
	if (regW)ram[BYTE_Z] &= ~BIT_Z;
	else ram[BYTE_Z] |= BIT_Z;
	damageByte(BYTE_Z);
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
	stackSize--;
#ifdef _DEBUG
		oldStack = (STACK*) (0xFF00000 | __LINE__);
#endif
	if (stopAtStackZero > 0)stopAtStackZero--;
	ram[0x0B] |= 0x80;
	damageByte(0x0B);
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
	stackSize--;
#ifdef _DEBUG
		oldStack = (STACK*) (0xFF00000 | __LINE__);
#endif
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
	stackSize--;
#ifdef _DEBUG
		oldStack = (STACK*) (0xFF00000 | __LINE__);
#endif
	if (stopAtStackZero > 0)stopAtStackZero--;
	return 2; 
}
int Backend::SLEEP(void*ign1, void*ign2) { 
	sleep = true;
	//TO & PD
	ram[0x03] |= 0x10;
	ram[0x03] &= 0xF7;
	damageByte(0x03);
	reset_wdt();
	aktCode++;
	return 1;
}
int Backend::SUBLW(void*k, void*ign) { 
	int tmpW = (((char)k & 0xFF) - (regW & 0xFF));
	if ((tmpW >= 0) && (regW != 0))ram[BYTE_C] |= BIT_C;
	else ram[BYTE_C] &= ~BIT_C;
	if (((((char)((char)k & 0x0F)) - ((char)(regW & 0x0F))) >= 0) && ((regW & 0x0F) != 0))ram[BYTE_DC] |= BIT_DC;
	else ram[BYTE_DC] &= ~BIT_DC;
	regW = tmpW & 0xFF;
	if (tmpW)ram[BYTE_Z] &= ~BIT_Z;
	else ram[BYTE_Z] |= BIT_Z;
	damageByte(BYTE_Z);
	aktCode++;
	return 1;
}
int Backend::XORLW(void*k, void*ign) { 
	regW = ((regW & 0xFF) ^ ((char)k & 0xFF));
	if (regW)ram[BYTE_Z] &= ~BIT_Z;
	else ram[BYTE_Z] |= BIT_Z;
	damageByte(BYTE_Z);
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

	//ignore first Breakpoint
	ignoreBreakpoint = true;

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
		DOIF(DEBUG_LVL >= DEBUG_ALL)PRINTF1("LOCKING MUTEXES in LINE %d\n", __LINE__);
		LOCK_MUTEX(m_lastError);
		DOIF(DEBUG_LVL >= DEBUG_ALL)PRINTF1("LOCKED MUTEX lastError in LINE %d\n", __LINE__);
		LOCK_MUTEX(m_regW);
		DOIF(DEBUG_LVL >= DEBUG_ALL)PRINTF1("LOCKED MUTEX regW in LINE %d\n", __LINE__);
		LOCK_MUTEX(m_run_code);
		DOIF(DEBUG_LVL >= DEBUG_ALL)PRINTF1("LOCKED MUTEX run_code in LINE %d\n", __LINE__);
		LOCK_MUTEX(m_isRunning);
		DOIF(DEBUG_LVL >= DEBUG_ALL)PRINTF1("LOCKED MUTEX isRunning in LINE %d\n", __LINE__);
		LOCK_MUTEX(m_ram);
		DOIF(DEBUG_LVL >= DEBUG_ALL)PRINTF1("LOCKED MUTEX ram in LINE %d\n", __LINE__);
		LOCK_MUTEX(m_eeprom);
		DOIF(DEBUG_LVL >= DEBUG_ALL)PRINTF1("LOCKED MUTEX eeprom in LINE %d\n", __LINE__);
		LOCK_MUTEX(m_runtime);
		DOIF(DEBUG_LVL >= DEBUG_ALL)PRINTF1("LOCKED MUTEX runtime in LINE %d\n", __LINE__);
		LOCK_MUTEX(m_wdt);
		DOIF(DEBUG_LVL >= DEBUG_ALL)PRINTF1("LOCKED MUTEXES in LINE %d\n", __LINE__);

		//Programmcounter -> Pointer
		if(written_to_PCL){
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
		}
		else{
			if(aktCode - &code->code[0] >= UC_SIZE_PROGRAM)aktCode = &code->code[0];
		}
		written_to_PCL = false;

		if(aktCode->breakpoint & !ignoreBreakpoint){
			DOIF(DEBUG_LVL >= DEBUG_ALL)PRINTF1("UNLOCKING MUTEXES in LINE %d\n", __LINE__);
			UNLOCK_MUTEX(m_wdt);
			UNLOCK_MUTEX(m_runtime);
			UNLOCK_MUTEX(m_eeprom);
			UNLOCK_MUTEX(m_ram);
			UNLOCK_MUTEX(m_isRunning);
			UNLOCK_MUTEX(m_run_code);
			UNLOCK_MUTEX(m_regW);
			UNLOCK_MUTEX(m_lastError);
			DOIF(DEBUG_LVL >= DEBUG_ALL)PRINTF1("UNLOCKED MUTEXES in LINE %d\n", __LINE__);

			LOCK_MUTEX(m_isRunning);
			isRunning = false;
			UNLOCK_MUTEX(m_isRunning);

			LOCK_MUTEX(m_isRunning);
			break;
		}
		else ignoreBreakpoint = false;

		DOIF(DEBUG_LVL >= DEBUG_RAM)PRINTF4("EXEC_ASM(%s %02x,%02x)\n\tsleeping: %d\n", Compiler::functionPointerToName(aktCode->function), aktCode->param1, aktCode->param2, sleep);
		//asm-execution
		if (!sleep) {
			needTime = aktCode->function(aktCode->param1, aktCode->param2, this);
		}
		else needTime = 1;
		DOIF(DEBUG_LVL >= DEBUG_RAM)PRINTF1("\tneeded cycles : %d\n", needTime);

		//interupts, timer, eeprom etc...
		//for-loop for imitating more cycle-operations (e.g. the timer has to count more than one time...)
		for (int tmp = 0; tmp < needTime; tmp++) {
			if (!sleep) if (!do_timer()) errorInThreadHappend = true;
			if (!errorInThreadHappend && do_eeprom() && do_wdt() && do_interrupts(needTime));
			else errorInThreadHappend = true;
		}

		//Pointer -> Programmcounter
		if(!written_to_PCL){
			int PC = (aktCode - &(code->code[0])) % UC_SIZE_PROGRAM;
			ram[PCL] = PC & 0xFF;
			damageByte(PCL);
		}

		//clear "read as '0'"-bits...
		ram[0x05] &= 0x1F;
		ram[0x0A] &= 0x1F;
		ram[87] &= 0x1F;
		ram[90] &= 0x1F;

		//stop if error in execution
		if (needTime < 1) {
			reset(RESET_POWER_UP);
			isRunning = false;
			errorInThreadHappend = true;
		}

#ifdef _DEBUG
		if (DEBUG_LVL >= DEBUG_RAM) {
			printf("\n");
			SetConsoleTextAttribute(hConsole, COLOR_RAM_HEAD);
			printf("    08  19  2A  3B  4C  5D  6E  7F  \n");
			for (int i = 0; i << 3 < UC_SIZE_RAM; i++) {
				printf("%02x  ", i<<3);
				int tmp = i << 3;
				for (int j = 0; j < 8 && tmp + j < UC_SIZE_RAM; j++) {
					if (damage[i] & (1 << j)) {
#ifdef USE_BACKEND_WITHOUT_GUI
						damage[i] &= ~(1 << j);
#endif
						SetConsoleTextAttribute(hConsole, COLOR_RAM_DMGD);
					}
					else SetConsoleTextAttribute(hConsole, COLOR_RAM_NORM);
					printf("%02x  ", ram[tmp + j]);
				}
				if (tmp + 8 > UC_SIZE_RAM){ SetConsoleTextAttribute(hConsole, COLOR_RAM_NORM); printf("        ");}
				SetConsoleTextAttribute(hConsole, COLOR_STANDARD);
				printf("\n");
				SetConsoleTextAttribute(hConsole, COLOR_RAM_HEAD);
			}
			SetConsoleTextAttribute(hConsole, COLOR_RAM_OTHER);
			printf("    W:  %02x  #:  %02x  r:  %02x          ", regW, stopAtStackZero & 0xFF, isRunning);
			SetConsoleTextAttribute(hConsole, COLOR_STANDARD);
			printf("\n");
			SetConsoleTextAttribute(hConsole, COLOR_RAM_OTHER);
			printf("    C   DC  Z                       ");
			SetConsoleTextAttribute(hConsole, COLOR_STANDARD);
			printf("\n");
			SetConsoleTextAttribute(hConsole, COLOR_RAM_OTHER); 
			printf("    %02x  %02x  %02x                      ", ram[0x03] & 0x01, ram[0x03] & 0x02, ram[0x03] & 0x04);
			SetConsoleTextAttribute(hConsole, COLOR_STANDARD);
			printf("\n\n");
		}
#endif

		//unlocks in correct order
		DOIF(DEBUG_LVL >= DEBUG_ALL)PRINTF1("UNLOCKING MUTEXES in LINE %d\n", __LINE__);
		UNLOCK_MUTEX(m_wdt);
		UNLOCK_MUTEX(m_runtime);
		UNLOCK_MUTEX(m_eeprom);
		UNLOCK_MUTEX(m_ram);
		UNLOCK_MUTEX(m_isRunning);
		UNLOCK_MUTEX(m_run_code);
		UNLOCK_MUTEX(m_regW);
		DOIF(DEBUG_LVL >= DEBUG_ALL)PRINTF1("UNLOCKED MUTEXES in LINE %d\n", __LINE__);

		//wait if asm could be executed
		if (!errorInThreadHappend && needTime >= 1) {
			UNLOCK_MUTEX(m_lastError);
			runtime += needTime * sleeptime;
			long long needTime_64 = ((long long)needTime * (long long)sleeptime * (long long)100);	//in ns

			//wait for the correct time
			auto end_time = std::chrono::high_resolution_clock::now();
			auto time = end_time - start_time;
			needTime_64 -= time.count();
			if (needTime_64 <= 0)needTime = 1;
			std::this_thread::sleep_for(std::chrono::nanoseconds(needTime_64));
			DOIF(DEBUG_LVL >= DEBUG_MORE)PRINTF1("Sleeping for %d us\n", needTime_64);
		}
		else UNLOCK_MUTEX(m_lastError);

		LOCK_MUTEX(m_isRunning);
	} while (isRunning && stopAtStackZero != 0 && !errorInThreadHappend);
	UNLOCK_MUTEX(m_isRunning);

	DOIF(DEBUG_LVL >= DEBUG_LESS)PRINTF("STOPPING\n");

	LOCK_MUTEX(m_lastError);
	if (errorInThreadHappend) Fl::awake(gui_handle_error, gui);
	UNLOCK_MUTEX(m_lastError);

	LOCK_MUTEX(m_terminated);
	terminated = true;
	UNLOCK_MUTEX(m_terminated);
}
