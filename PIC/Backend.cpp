#include "Backend.h"

#include <cstring>

#define NOT_IMPLEMENTED	"Nicht implementiert!"
#define MEMORY_MISSING "Memory is missing."

#define MSGLEN()	(lastErrorLen = (std::strlen(lastError)+1))

Backend::Backend()
{
}


Backend::~Backend()
{
}

bool Backend::LoadProgramm(char * c)
{
	m_lastError.lock();
	lastError = NOT_IMPLEMENTED;
	MSGLEN();
	m_lastError.unlock();
	return false;
}

bool Backend::Start()
{
	m_lastError.lock();
	lastError = NOT_IMPLEMENTED;
	MSGLEN();
	m_lastError.unlock();
	return false;
}

bool Backend::Stop()
{
	m_lastError.lock();
	lastError = NOT_IMPLEMENTED;
	MSGLEN();
	m_lastError.unlock();
	return false;
}

bool Backend::Step()
{
	m_lastError.lock();
	lastError = NOT_IMPLEMENTED;
	MSGLEN();
	m_lastError.unlock();
	return false;
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
	int tmp = (regW + ram[(int)f]);
	if (tmp & 0x0100)ram[BYTE_C] |= BIT_C;
	else ram[BYTE_C] &= ~BIT_C;
	if (tmp & 0x0010)ram[BYTE_DC] |= BIT_DC;
	else ram[BYTE_DC] &= ~BIT_DC;
	tmp &= 0xFF;
	if (tmp)ram[BYTE_Z] &= ~BIT_Z;
	else ram[BYTE_Z] |= BIT_Z;
	if (d)regW = tmp;
	else ram[(int)f] = tmp;
	aktCode++;
	return 1;
}
int Backend::ANDWF(void*f, void*d) {
	int tmp = (regW & ram[(int)f]) & 0xFF;
	if (tmp)ram[BYTE_Z] &= ~BIT_Z;
	else ram[BYTE_Z] |= BIT_Z;
	if (d)regW = tmp;
	else ram[(int)f] = tmp;
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
	ram[(char)f] &= ~(1 << (char)b);
	aktCode++;
	return 1;
}
int Backend::BSF(void*f, void*b) {
	ram[(char)f] |= (1 << (char)b);
	aktCode++;
	return 1;
}
int Backend::BTFSC(void*f, void*b) {
	if (ram[(char)f] & (1 << (char)b)) {	//set -> no skip
		aktCode++;
		return 1;
	}
	else {	// skip
		aktCode += 2;
		return 2;
	}
}
int Backend::BTFSS(void*f, void*b) {
	if (ram[(char)f] & (1 << (char)b)) {	//set -> skip
		aktCode += 2;
		return 2;
	}
	else {	// no skip
		aktCode++;
		return 1;
	}
}

int Backend::ADDLW(void*k, void*ign) {
	regW = (regW + (char)k);
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