#include "Backend.h"
#include "Compiler.h"
#include <Windows.h>

#include <cstring>
#include <chrono>

#define NOT_IMPLEMENTED	"Nicht implementiert!"
#define MEMORY_MISSING "Memory is missing."
#define FUNCTIONSTACK_EMPTY	"Fuctionstack is empty!"

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
		posDamaged = pos;
		return ram[pos]; 
	}
	else {
		if (ram[0x03] & 0xC0) {
			m_lastError.lock();
			lastError = "Falsche Bank gew‰hlt!";
			lastErrorLen = MSGLEN();
			m_lastError.unlock();
			tmp = 0;
			return tmp;
		}
		else if (ram[0x03] & 0x40) {
			posDamaged = 82 + pos;
			return ram[82 + pos];
		}
		else {
			posDamaged = pos;
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

	sleep = false;

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
		m_lastError.lock();
		lastError = "Der Start ist gesperrt!";
		MSGLEN();
		m_lastError.unlock();
		return false;
	}
	m_isRunningLocked.unlock();
	m_isRunning.lock();
	if (isRunning) {
		m_isRunning.unlock();
		m_lastError.lock();
		lastError = "Der Kontroller l‰uft aktuell!";
		MSGLEN();
		m_lastError.unlock();
		return false;
	}
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

int Backend::GetByte(int reg, byte bank)
{
#ifdef _DEBUG
	if (bank > 1 || bank < 0) {
		m_lastError.lock();
		lastError = "'bank' muss 1 oder 0 sein!";
		MSGLEN();
		PRINTF1("int Backend::GetByte(int reg, byte bank):\n\t'bank' muss 1 oder 0 sein und ist %d!\n", bank);
		m_lastError.unlock();
		return -1;
	}
	if (reg < 0 || reg > 0x0F) {
		m_lastError.lock();
		lastError = "'reg' muss zwischen (einschlieﬂlich) 0x00 und 0xFF sein!";
		MSGLEN();
		PRINTF1("int Backend::GetByte(int reg, byte bank):\n\t'reg' muss zwischen (einschlieﬂlich) 0x00 und 0xFF sein und ist %d!\n", reg);
		m_lastError.unlock();
		return -1;
	}
#endif
	int ret, tmp;
	m_ram.lock();
	tmp = ram[0x03];
	if ((reg = 0x0F) == 0x03) { m_ram.unlock(); return tmp; }
	if (bank == 0)ram[0x03] = ram[0x03] & (~0x20);
	else ram[0x03] = ram[0x03] | 0x20;
	ret = getCell_unsafe(reg);
	ram[0x03] = tmp;
	m_ram.unlock();
	return ret;
}

bool Backend::SetByte(int reg, byte bank, byte val)
{
#ifdef _DEBUG
	if (bank > 1 || bank < 0) {
		m_lastError.lock();
		lastError = "'bank' muss 1 oder 0 sein!";
		MSGLEN();
		PRINTF1("bool Backend::SetByte(int reg, byte bank, byte val):\n\t'bank' muss 1 oder 0 sein und ist %d!\n", bank);
		m_lastError.unlock();
		return -1;
	}
	if (reg < 0 || reg > 0x0F) {
		m_lastError.lock();
		lastError = "'reg' muss zwischen (einschlieﬂlich) 0x00 und 0xFF sein!";
		MSGLEN();
		PRINTF1("bool Backend::SetByte(int reg, byte bank, byte val):\n\t'reg' muss zwischen (einschlieﬂlich) 0x00 und 0xFF sein und ist %d!\n", reg);
		m_lastError.unlock();
		return -1;
	}
#endif
	int tmp;
	m_ram.lock();
	if ((reg = 0x0F) == 0x03) { ram[0x03] = val; m_ram.unlock(); return true; }
	tmp = ram[0x03];
	if (bank == 0)ram[0x03] = ram[0x03] & (~0x20);
	else ram[0x03] = ram[0x03] | 0x20;
	getCell_unsafe(reg) = val;
	ram[0x03] = tmp;
	m_ram.unlock();
	return true;
}

bool Backend::SetBit(int reg, byte bank, int pos, bool val)
{
#ifdef _DEBUG
	if (pos > 7 || pos < 0) {
		m_lastError.lock();
		lastError = "'pos' muss zwischen (einschlieﬂlich) 0 und 7 sein!";
		MSGLEN();
		PRINTF1("bool Backend::SetBit(int reg, byte bank, int pos, bool val):\n\t'pos' muss zwischen (einschlieﬂlich) 0 und 7 sein und ist %d!\n", pos);
		m_lastError.unlock();
		return -1;
	}
	if (bank > 1 || bank < 0) {
		m_lastError.lock();
		lastError = "'bank' muss 1 oder 0 sein!";
		MSGLEN();
		PRINTF1("bool Backend::SetBit(int reg, byte bank, int pos, bool val):\n\t'bank' muss 1 oder 0 sein und ist %d!\n", bank);
		m_lastError.unlock();
		return -1;
	}
	if (reg < 0 || reg > 0x0F) {
		m_lastError.lock();
		lastError = "'reg' muss zwischen (einschlieﬂlich) 0x00 und 0xFF sein!";
		MSGLEN();
		PRINTF1("bool Backend::SetBit(int reg, byte bank, int pos, bool val):\n\t'reg' muss zwischen (einschlieﬂlich) 0x00 und 0xFF sein und ist %d!\n", reg);
		m_lastError.unlock();
		return -1;
	}
#endif
	int tmp;
	m_ram.lock();
	if ((reg = 0x0F) == 0x03) { 
		if(val)ram[0x03] |= (1 << pos);
		else ram[0x03] &= ~(1 << pos);
		m_ram.unlock(); 
		return true; 
	}
	tmp = ram[0x03];
	if (bank == 0)ram[0x03] = ram[0x03] & (~0x20);
	else ram[0x03] = ram[0x03] | 0x20;
	if(val)getCell_unsafe(reg) |= (1 << pos);
	else getCell_unsafe(reg) &= ~(1 << pos);
	ram[0x03] = tmp;
	m_ram.unlock();
	return true;
}

int Backend::GetBit(int reg, byte bank, int pos)
{
#ifdef _DEBUG
	if (pos > 7 || pos < 0) {
		m_lastError.lock();
		lastError = "'pos' muss zwischen (einschlieﬂlich) 0 und 7 sein!";
		MSGLEN();
		PRINTF1("int Backend::GetBit(int reg, byte bank, int pos):\n\t'pos' muss zwischen (einschlieﬂlich) 0 und 7 sein und ist %d!\n", pos);
		m_lastError.unlock();
		return -1;
	}
	if (bank > 1 || bank < 0) {
		m_lastError.lock();
		lastError = "'bank' muss 1 oder 0 sein!";
		MSGLEN();
		PRINTF1("int Backend::GetBit(int reg, byte bank, int pos):\n\t'bank' muss 1 oder 0 sein und ist %d!\n", bank);
		m_lastError.unlock();
		return -1;
	}
	if (reg < 0 || reg > 0x0F) {
		m_lastError.lock();
		lastError = "'reg' muss zwischen (einschlieﬂlich) 0x00 und 0xFF sein!";
		MSGLEN();
		PRINTF1("int Backend::GetBit(int reg, byte bank, int pos):\n\t'reg' muss zwischen (einschlieﬂlich) 0x00 und 0xFF sein und ist %d!\n", reg);
		m_lastError.unlock();
		return -1;
	}
#endif
	int tmp;
	bool val;
	m_ram.lock();
	if ((reg = 0x0F) == 0x03) {
		val = (ram[0x03] & (1 << pos));
		m_ram.unlock();
		return val;
	}
	tmp = ram[0x03];
	if (bank == 0)ram[0x03] = ram[0x03] & (~0x20);
	else ram[0x03] = ram[0x03] | 0x20;
	val = (getCell_unsafe(reg) & (1 << pos));
	ram[0x03] = tmp;
	m_ram.unlock();
	return true;
}

int Backend::getRegW()
{
	int w;
	m_regW.lock();
	w = regW;
	m_regW.unlock();
	return w;
}

bool Backend::setRegW(byte val)
{
	m_regW.lock();
	regW = val;
	m_regW.unlock();
	return true;
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
	byte &cell = getCell_unsafe((int)f);
	byte tmpCell = cell;
	int tmp = ((regW & 0xFF) + (cell & 0xFF));
	if ((tmp ^ tmpCell) & 0x0100)ram[BYTE_C] |= BIT_C;
	else ram[BYTE_C] &= ~BIT_C;
	if ((tmp ^ tmpCell) & 0x0010)ram[BYTE_DC] |= BIT_DC;
	else ram[BYTE_DC] &= ~BIT_DC;
	tmp &= 0xFF;
	if (tmp)ram[BYTE_Z] &= ~BIT_Z;
	else ram[BYTE_Z] |= BIT_Z;
	if (d)regW = tmp & 0xFF;
	else cell = tmp & 0xFF;
	aktCode++;
	return 1;
}
int Backend::ANDWF(void*f, void*d) {
	byte &cell = getCell_unsafe((int)f);
	int tmp = (regW & cell) & 0xFF;
	if (tmp)ram[BYTE_Z] &= ~BIT_Z;
	else ram[BYTE_Z] |= BIT_Z;
	if (d)regW = tmp;
	else cell = tmp;
	aktCode++;
	return 1;
}
int Backend::CLRF(void*f, void*ign) { lastError = NOT_IMPLEMENTED; MSGLEN();  return -1; }
int Backend::CLRW(void*ign1, void*ign2) { lastError = NOT_IMPLEMENTED; MSGLEN();  return -1; }
int Backend::COMF(void*f, void*d) { lastError = NOT_IMPLEMENTED; MSGLEN();  return -1; }
int Backend::DECF(void*f, void*d) { lastError = NOT_IMPLEMENTED; MSGLEN();  return -1; }
int Backend::DECFSZ(void*f, void*d) { lastError = NOT_IMPLEMENTED; MSGLEN();  return -1; }
int Backend::INCF(void*f, void*d) { lastError = NOT_IMPLEMENTED; MSGLEN();  return -1; }
int Backend::INCFSZ(void*f, void*d) { lastError = NOT_IMPLEMENTED; MSGLEN();  return -1; }
int Backend::IORWF(void*f, void*d) { lastError = NOT_IMPLEMENTED; MSGLEN();  return -1; }
int Backend::MOVF(void*f, void*d) { lastError = NOT_IMPLEMENTED; MSGLEN();  return -1; }
int Backend::MOVWF(void*f, void*ign) { lastError = NOT_IMPLEMENTED; MSGLEN();  return -1; }
int Backend::NOP(void*ign1, void*ign2) { lastError = NOT_IMPLEMENTED; MSGLEN();  return -1; }
int Backend::RLF(void*f, void*d) { lastError = NOT_IMPLEMENTED; MSGLEN();  return -1; }
int Backend::RRF(void*f, void*d) { lastError = NOT_IMPLEMENTED; MSGLEN();  return -1; }
int Backend::SUBWF(void*f, void*d) { lastError = NOT_IMPLEMENTED; MSGLEN();  return -1; }
int Backend::SWAPF(void*f, void*d) { lastError = NOT_IMPLEMENTED; MSGLEN();  return -1; }
int Backend::XORWF(void*f, void*d) { lastError = NOT_IMPLEMENTED; MSGLEN();  return -1; }

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
	char tmpW = regW;
	regW = ((regW & 0xFF) + ((char)k & 0xFF));
	if ((regW ^ tmpW) & 0x0100)ram[BYTE_C] |= BIT_C;
	else ram[BYTE_C] &= ~BIT_C;
	if ((regW ^ tmpW) & 0x0010)ram[BYTE_DC] |= BIT_DC;
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
	functionStack = newAdress;
	aktCode = &(code->code[(int)k]);
	if (stopAtStackZero >= 0)stopAtStackZero++;
	return 2;
}
int Backend::CLRWDT(void*ign1, void*ign2) {
	//TODO: wo sind die entsprechenden regs?
	ram[0x03] |= 0x18;
	ram[83] &= 0xF0;
	return 1;
}
int Backend::GOTO(void*k, void*ign) { 
	aktCode = &(code->code[(int)k]);
	return 2;
}
int Backend::IORLW(void*k, void*ign) { 
	regW = ((regW & 0xFF) | ((char)k & 0xFF));
	if (regW)ram[BYTE_Z] &= ~BIT_Z;
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
	//TODO: richtige regs?
	ram[0x03] |= 0x10;
	ram[0x03] &= 0xF7;
	ram[83] &= 0xF0;
	return 1;
}
int Backend::SUBLW(void*k, void*ign) { 
	char tmpW = regW;
	regW = (((char)k & 0xFF) - (regW & 0xFF));
	if ((regW ^ tmpW) & 0x0100)ram[BYTE_C] |= BIT_C;
	else ram[BYTE_C] &= ~BIT_C;
	if ((regW ^ tmpW) & 0x0010)ram[BYTE_DC] |= BIT_DC;
	else ram[BYTE_DC] &= ~BIT_DC;
	regW &= 0xFF;
	if (regW)ram[BYTE_Z] &= ~BIT_Z;
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
	m_terminated.lock();
	terminated = false;
	m_terminated.unlock();
	if (modus != MOD_STEP) {
		m_isRunning.lock();
		isRunning = true;
		m_isRunning.unlock();
		switch (modus){
		case MOD_STEP_OVER:
			stopAtStackZero = 0;
			break;
		case MOD_STEP_OUT:
			stopAtStackZero = 1;
		case MOD_STANDARD:
			stopAtStackZero = -1;
#ifdef _DEBUG
		default:
			PRINTF1("MODUS (= %d ) ist nicht definiert!\n", modus);
#endif
		}
		
	}
	else {
		m_isRunning.lock();
		isRunning = false;
		m_isRunning.unlock();
	}
	int needTime;
	do {
		//locks in correct order
		m_lastError.lock();
		m_regW.lock();
		m_run_code.lock();
		m_isRunning.lock();
		m_ram.lock();
		m_eeprom.lock();
		m_runtime.lock();

		auto start_time = std::chrono::high_resolution_clock::now();

		//asm-execution
		posDamaged = 0;
		if (!sleep) {
			needTime = aktCode->function(aktCode->param1, aktCode->param2, this);
		}
		else needTime = 1;

		//interupts, timers, etc...


		//clear "read as '0'"-bits...
		switch (posDamaged) {
		case 0x05: case 0x0A: case 83: case 86:
			ram[posDamaged] &= 0x1F;
		}

		//wait for 1000us if asm could be executed
		if (needTime < 1) {
			reset(RESET_POWER_UP);
			isRunning = false;
		}
		else {
			runtime += needTime;
			needTime = (needTime * 1000);

			//wait for the correct time
			auto end_time = std::chrono::high_resolution_clock::now();
			auto time = end_time - start_time;
			Sleep(needTime - time.count());
		}

		//unlocks in correct order
		m_runtime.unlock();
		m_eeprom.unlock();
		m_ram.unlock();
		m_isRunning.unlock();
		m_run_code.unlock();
		m_regW.unlock();
		m_lastError.unlock();

		m_isRunning.lock();
	} while (isRunning);
	m_isRunning.unlock();

	while (functionStack != nullptr) {
		
	}

	m_terminated.lock();
	terminated = true;
	m_terminated.unlock();
}
