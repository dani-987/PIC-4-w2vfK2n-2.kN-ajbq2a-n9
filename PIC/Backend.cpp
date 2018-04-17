#include "Backend.h"
#include "Compiler.h"
#include <Windows.h>

#include <cstring>
#include <chrono>

#define NOT_IMPLEMENTED	"Nicht implementiert!"
#define MEMORY_MISSING "Memory is missing."

#define RESET_POWER_UP		0
#define RESET_CLRWDT		1
#define RESET_SLEEP			2
#define RESET_WDT_TIMEOUT	3

#define MSGLEN()	(lastErrorLen = (std::strlen(lastError)+1))

//set PC to ram[0x02] = 0;

void call_backup_in(void* _callInOtherThread) {
	struct call_in_other_thread_s* callInOtherThread = (struct call_in_other_thread_s*)_callInOtherThread;
	callInOtherThread->backend->run_in_other_thread(callInOtherThread->modus);
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
			m_lastError.lock();
			lastError = "Falsche Bank gewählt!";
			lastErrorLen = MSGLEN();
			m_lastError.unlock();
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
	ram[0x02] = 0x00;
	ram[0x03] &= 0x1F;
	ram[0x0A] = 0;
	ram[0x0B] &= 0xFE;

	ram[83] = 0xFF;
	ram[84] = 0x00;
	ram[87] = 0x1F;
	ram[88] = 0xFF;


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
	m_isRunningLocked.lock();
	isRunningLocked = true;
	m_isRunningLocked.unlock();
	Stop();
	m_terminated.lock();
	while (!terminated) {
		m_terminated.unlock();
		Sleep(50);
		m_terminated.lock();
	}
	m_terminated.unlock();
}

bool Backend::letRun(int modus)
{
	m_isRunningLocked.lock();
	if (isRunningLocked) {
		m_isRunningLocked.unlock();
		return false;
	}
	m_isRunningLocked.unlock();
	m_isRunning.lock();
	isRunning = true;
	m_isRunning.unlock();
	uC = new std::thread();
	return true;
}

Backend::Backend(GUI* gui)
{
	lastError = "Kein Fehler";
	lastErrorLen = MSGLEN();
	regW = 0;
	code = nullptr;
	aktCode = nullptr;
	functionStack = nullptr;
	uC = nullptr;
	isRunning = false;
	terminated = true;
	isRunningLocked = false;
	ram = (byte*)malloc(UC_SIZE_RAM);
	memset(ram,0,UC_SIZE_RAM);
	reset(RESET_POWER_UP);
	eeprom = (char*)malloc(UC_SIZE_EEPROM);
	memset(eeprom, 0, UC_SIZE_EEPROM);
}


Backend::~Backend()
{
	Stop_And_Wait();
	free(eeprom);
	free(ram);
	if (functionStack != nullptr)free(functionStack);
	if (code != nullptr) Compiler::freeASM(code);
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
	memset(ram, 0, UC_SIZE_RAM);
	reset(RESET_POWER_UP);
	m_isRunningLocked.lock();
	isRunningLocked = false;
	m_isRunningLocked.unlock();
	return ret;
}

bool Backend::Start()
{
	return letRun(MOD_STANDARD);
}

bool Backend::Stop()
{
	m_isRunning.lock();
	isRunning = false;
	m_isRunning.unlock();
	return true;
}

bool Backend::Step()
{
	return letRun(MOD_STEP);
}

bool Backend::Reset()
{
	m_lastError.lock();
	lastError = NOT_IMPLEMENTED;
	MSGLEN();
	m_lastError.unlock();
	return false;
}

bool Backend::SetMem(int from, int len, void * mem)
{
	m_lastError.lock();
	lastError = NOT_IMPLEMENTED;
	MSGLEN();
	m_lastError.unlock();
	return false;
}

bool Backend::SetBit(int byte, int pos, bool val)
{
	m_lastError.lock();
	lastError = NOT_IMPLEMENTED;
	MSGLEN();
	m_lastError.unlock();
	return false;
}

void * Backend::GetMem(int from, int len)
{
	m_lastError.lock();
	lastError = NOT_IMPLEMENTED;
	MSGLEN();
	m_lastError.unlock();
	return nullptr;
}

int Backend::GetBit(int byte, int pos)
{
	m_lastError.lock();
	lastError = NOT_IMPLEMENTED;
	MSGLEN();
	m_lastError.unlock();
	return -1;
}

int Backend::getRegW()
{
	m_lastError.lock();
	lastError = NOT_IMPLEMENTED;
	MSGLEN();
	m_lastError.unlock();
	return -1;
}

bool Backend::setRegW(char val)
{
	m_lastError.lock();
	lastError = NOT_IMPLEMENTED;
	MSGLEN();
	m_lastError.unlock();
	return false;
}

char * Backend::getErrorMSG()
{
	m_lastError.lock();
	if (lastError == nullptr)return nullptr;
	char* ret = (char*)malloc(lastErrorLen*sizeof(char));
	memcpy(ret, lastError, lastErrorLen * sizeof(char));
	m_lastError.unlock();
	return ret;
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

int Backend::ADDWF(void*f, void*d) {
	int tmp = ((regW & 0xFF) + (getCell_unsafe((int)f) & 0xFF));
	if (tmp & 0x0100)ram[BYTE_C] |= BIT_C;
	else ram[BYTE_C] &= ~BIT_C;
	if (tmp & 0x0010)ram[BYTE_DC] |= BIT_DC;
	else ram[BYTE_DC] &= ~BIT_DC;
	tmp &= 0xFF;
	if (tmp)ram[BYTE_Z] &= ~BIT_Z;
	else ram[BYTE_Z] |= BIT_Z;
	if (d)regW = tmp & 0xFF;
	else getCell_unsafe((int)f) = tmp & 0xFF;
	aktCode++;
	return 1;
}
int Backend::ANDWF(void*f, void*d) {
	int tmp = (regW & getCell_unsafe((int)f)) & 0xFF;
	if (tmp)ram[BYTE_Z] &= ~BIT_Z;
	else ram[BYTE_Z] |= BIT_Z;
	if (d)regW = tmp;
	else getCell_unsafe((int)f) = tmp;
	aktCode++;
	return 1;
}
int Backend::CLRF(void*f, void*ign) { lastError = NOT_IMPLEMENTED; return -1; }
int Backend::CLRW(void*ign1, void*ign2) { lastError = NOT_IMPLEMENTED; return -1; }
int Backend::COMF(void*f, void*d) { lastError = NOT_IMPLEMENTED; return -1; }
int Backend::DECF(void*f, void*d) { lastError = NOT_IMPLEMENTED; return -1; }
int Backend::DECFSZ(void*f, void*d) { lastError = NOT_IMPLEMENTED; return -1; }
int Backend::INCF(void*f, void*d) { lastError = NOT_IMPLEMENTED; return -1; }
int Backend::INCFSZ(void*f, void*d) { lastError = NOT_IMPLEMENTED; return -1; }
int Backend::IORWF(void*f, void*d) { lastError = NOT_IMPLEMENTED; return -1; }
int Backend::MOVF(void*f, void*d) { lastError = NOT_IMPLEMENTED; return -1; }
int Backend::MOVWF(void*f, void*ign) { lastError = NOT_IMPLEMENTED; return -1; }
int Backend::NOP(void*ign1, void*ign2) { lastError = NOT_IMPLEMENTED; return -1; }
int Backend::RLF(void*f, void*d) { lastError = NOT_IMPLEMENTED; return -1; }
int Backend::RRF(void*f, void*d) { lastError = NOT_IMPLEMENTED; return -1; }
int Backend::SUBWF(void*f, void*d) { lastError = NOT_IMPLEMENTED; return -1; }
int Backend::SWAPF(void*f, void*d) { lastError = NOT_IMPLEMENTED; return -1; }
int Backend::XORWF(void*f, void*d) { lastError = NOT_IMPLEMENTED; return -1; }

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
	regW = ((regW & 0xFF) + ((char)k & 0xFF));
	if (regW & 0x0100)ram[BYTE_C] |= BIT_C;
	else ram[BYTE_C] &= ~BIT_C;
	if (regW & 0x0010)ram[BYTE_DC] |= BIT_DC;
	else ram[BYTE_DC] &= ~BIT_DC;
	regW &= 0xFF;
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
	newAdress->isInterupt = false;
	functionStack = newAdress;
	aktCode = &(code->code[(int)k]);
	return 2;
}
int Backend::CLRWDT(void*ign1, void*ign2) { lastError = NOT_IMPLEMENTED; return -1; }
int Backend::GOTO(void*k, void*ign) { lastError = NOT_IMPLEMENTED; return -1; }
int Backend::IORLW(void*k, void*ign) { lastError = NOT_IMPLEMENTED; return -1; }
int Backend::MOVLW(void*k, void*ign) { lastError = NOT_IMPLEMENTED; return -1; }
int Backend::RETFIE(void*ign1, void*ign2) { lastError = NOT_IMPLEMENTED; return -1; }
int Backend::RETLW(void*k, void*ign) { lastError = NOT_IMPLEMENTED; return -1; }
int Backend::RETURN(void*ign1, void*ign2) { lastError = NOT_IMPLEMENTED; return -1; }
int Backend::SLEEP(void*ign1, void*ign2) { lastError = NOT_IMPLEMENTED; return -1; }
int Backend::SUBLW(void*k, void*ign) { lastError = NOT_IMPLEMENTED; return -1; }
int Backend::XORLW(void*k, void*ign) { lastError = NOT_IMPLEMENTED; return -1; }

void Backend::run_in_other_thread(byte modus)
{
	m_terminated.lock();
	terminated = false;
	m_terminated.unlock();
	if (modus != MOD_STEP) {
		m_isRunning.lock();
		isRunning = true;
		m_isRunning.unlock();
	}
	else {
		m_isRunning.lock();
		isRunning = false;
		m_isRunning.unlock();
	}
	int needTime;
	do {
		auto start_time = std::chrono::high_resolution_clock::now();
		needTime = aktCode->function(aktCode->param1, aktCode->param2, this);
		needTime = (needTime * 1000);
		auto end_time = std::chrono::high_resolution_clock::now();
		auto time = end_time - start_time;
		Sleep( - time.count());
		m_isRunning.lock();
	} while (needTime > 0 && isRunning);
	m_isRunning.unlock();

	while (functionStack != nullptr) {
		
	}

	m_terminated.lock();
	terminated = true;
	m_terminated.unlock();
}
