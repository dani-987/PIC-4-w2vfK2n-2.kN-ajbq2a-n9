#include "ASM.h"
#include <stdlib.h> 

//Carry = 9th Bit after ADD
#define BIT_C		0x01
#define BYTE_C		0x03
//DigitCarry = 5th Bit after ADD
#define BIT_DC		0x02
#define BYTE_DC		0x03
//Zerobit
#define BIT_Z		0x04
#define BYTE_Z		0x03

#define MEMORY_MISSING "Memory ist missing."
#define NOT_IMPLEMENTED "Not Implemented."

char* __asm__lastError = "Kein Fehler";

int instructions::ADDWF(void*f, void*d, ASM_CODE* code, int& w, char* ram, char* eeprom, ASM_CODE* &aktCode, STACK* &call, STACK* &interupt){
	int tmp = (w + ram[(int)f]);
	if (tmp & 0x0100)ram[BYTE_C] |= BIT_C;
	else ram[BYTE_C] &= ~BIT_C;
	if (tmp & 0x0010)ram[BYTE_DC] |= BIT_DC;
	else ram[BYTE_DC] &= ~BIT_DC;
	tmp &= 0xFF;
	if (tmp)ram[BYTE_Z] &= ~BIT_Z;
	else ram[BYTE_Z] |= BIT_Z;
	if (d)w = tmp;
	else ram[(int)f] = tmp;
	aktCode++;
	return 1;
}
int instructions::ANDWF(void*f, void*d, ASM_CODE* code, int& w, char* ram, char* eeprom, ASM_CODE* &aktCode, STACK* &call, STACK* &interupt){
	int tmp = (w & ram[(int)f]) & 0xFF;
	if (tmp)ram[BYTE_Z] &= ~BIT_Z;
	else ram[BYTE_Z] |= BIT_Z;
	if (d)w = tmp;
	else ram[(int)f] = tmp;
	aktCode++;
	return 1;
}
int instructions::CLRF(void*f, void*ign, ASM_CODE* code, int& w, char* ram, char* eeprom, ASM_CODE* &aktCode, STACK* &call, STACK* &interupt){__asm__lastError = NOT_IMPLEMENTED;return -1;}
int instructions::CLRW(void*ign1, void*ing2, ASM_CODE* code, int& w, char* ram, char* eeprom, ASM_CODE* &aktCode, STACK* &call, STACK* &interupt){__asm__lastError = NOT_IMPLEMENTED;return -1;}
int instructions::COMF(void*f, void*d, ASM_CODE* code, int& w, char* ram, char* eeprom, ASM_CODE* &aktCode, STACK* &call, STACK* &interupt){__asm__lastError = NOT_IMPLEMENTED;return -1;}
int instructions::DECF(void*f, void*d, ASM_CODE* code, int& w, char* ram, char* eeprom, ASM_CODE* &aktCode, STACK* &call, STACK* &interupt){__asm__lastError = NOT_IMPLEMENTED;return -1;}
int instructions::DECFSZ(void*f, void*d, ASM_CODE* code, int& w, char* ram, char* eeprom, ASM_CODE* &aktCode, STACK* &call, STACK* &interupt){__asm__lastError = NOT_IMPLEMENTED;return -1;}
int instructions::INCF(void*f, void*d, ASM_CODE* code, int& w, char* ram, char* eeprom, ASM_CODE* &aktCode, STACK* &call, STACK* &interupt){__asm__lastError = NOT_IMPLEMENTED;return -1;}
int instructions::INCFSZ(void*f, void*d, ASM_CODE* code, int& w, char* ram, char* eeprom, ASM_CODE* &aktCode, STACK* &call, STACK* &interupt){__asm__lastError = NOT_IMPLEMENTED;return -1;}
int instructions::IORWF(void*f, void*d, ASM_CODE* code, int& w, char* ram, char* eeprom, ASM_CODE* &aktCode, STACK* &call, STACK* &interupt){__asm__lastError = NOT_IMPLEMENTED;return -1;}
int instructions::MOVF(void*f, void*d, ASM_CODE* code, int& w, char* ram, char* eeprom, ASM_CODE* &aktCode, STACK* &call, STACK* &interupt){__asm__lastError = NOT_IMPLEMENTED;return -1;}
int instructions::MOVWF(void*f, void*ign, ASM_CODE* code, int& w, char* ram, char* eeprom, ASM_CODE* &aktCode, STACK* &call, STACK* &interupt){__asm__lastError = NOT_IMPLEMENTED;return -1;}
int instructions::NOP(void*ign1, void*ign2, ASM_CODE* code, int& w, char* ram, char* eeprom, ASM_CODE* &aktCode, STACK* &call, STACK* &interupt){__asm__lastError = NOT_IMPLEMENTED;return -1;}
int instructions::RLF(void*f, void*d, ASM_CODE* code, int& w, char* ram, char* eeprom, ASM_CODE* &aktCode, STACK* &call, STACK* &interupt){__asm__lastError = NOT_IMPLEMENTED;return -1;}
int instructions::RRF(void*f, void*d, ASM_CODE* code, int& w, char* ram, char* eeprom, ASM_CODE* &aktCode, STACK* &call, STACK* &interupt){__asm__lastError = NOT_IMPLEMENTED;return -1;}
int instructions::SUBWF(void*f, void*d, ASM_CODE* code, int& w, char* ram, char* eeprom, ASM_CODE* &aktCode, STACK* &call, STACK* &interupt){__asm__lastError = NOT_IMPLEMENTED;return -1;}
int instructions::SWAPF(void*f, void*d, ASM_CODE* code, int& w, char* ram, char* eeprom, ASM_CODE* &aktCode, STACK* &call, STACK* &interupt){__asm__lastError = NOT_IMPLEMENTED;return -1;}
int instructions::XORWF(void*f, void*d, ASM_CODE* code, int& w, char* ram, char* eeprom, ASM_CODE* &aktCode, STACK* &call, STACK* &interupt){__asm__lastError = NOT_IMPLEMENTED;return -1;}

int instructions::BCF(void*f, void*b, ASM_CODE* code, int& w, char* ram, char* eeprom, ASM_CODE* &aktCode, STACK* &call, STACK* &interupt){
	ram[(char)f] &= ~(1 << (char)b);
	aktCode++;
	return 1;
}
int instructions::BSF(void*f, void*b, ASM_CODE* code, int& w, char* ram, char* eeprom, ASM_CODE* &aktCode, STACK* &call, STACK* &interupt){
	ram[(char)f] |= (1 << (char)b);
	aktCode++;
	return 1;
}
int instructions::BTFSC(void*f, void*b, ASM_CODE* code, int& w, char* ram, char* eeprom, ASM_CODE* &aktCode, STACK* &call, STACK* &interupt){
	if (ram[(char)f] & (1 << (char)b)) {	//set -> no skip
		aktCode++;
		return 1;
	}
	else {	// skip
		aktCode+=2;
		return 2;
	}
}
int instructions::BTFSS(void*f, void*b, ASM_CODE* code, int& w, char* ram, char* eeprom, ASM_CODE* &aktCode, STACK* &call, STACK* &interupt){
	if (ram[(char)f] & (1 << (char)b)) {	//set -> skip
		aktCode += 2;
		return 2;
	}
	else {	// no skip
		aktCode++;
		return 1;
	}
}

int instructions::ADDLW(void*k, void*ign, ASM_CODE* code, int& w, char* ram, char* eeprom, ASM_CODE* &aktCode, STACK* &call, STACK* &interupt){
	w = (w + (char)k);
	if (w & 0x0100)ram[BYTE_C] |= BIT_C;
	else ram[BYTE_C] &= ~BIT_C;
	if (w & 0x0010)ram[BYTE_DC] |= BIT_DC;
	else ram[BYTE_DC] &= ~BIT_DC;
	w &= 0xFF;
	if (w)ram[BYTE_Z] &= ~BIT_Z;
	else ram[BYTE_Z] |= BIT_Z;
	aktCode++;
	return 1;
}
int instructions::ANDLW(void*k, void*ign, ASM_CODE* code, int& w, char* ram, char* eeprom, ASM_CODE* &aktCode, STACK* &call, STACK* &interupt){
	w = (w & (char)k) & 0xFF;
	if (w)ram[BYTE_Z] &= ~BIT_Z;
	else ram[BYTE_Z] |= BIT_Z;
	aktCode++;
	return 1;
}
int instructions::CALL(void*k, void*ign, ASM_CODE* code, int& w, char* ram, char* eeprom, ASM_CODE* &aktCode, STACK* &call, STACK* &interupt){
	STACK* newAdress = (STACK*)malloc(sizeof(STACK));
	if (newAdress == nullptr) { 
		__asm__lastError = MEMORY_MISSING;
		return -1;
	}
	newAdress->jumpTo = aktCode + 1;
	newAdress->next = call;
	call = newAdress;
	aktCode = &(code[(int)k]);
	return 2;
}
int instructions::CLRWDT(void*ign1, void*ign2, ASM_CODE* code, int& w, char* ram, char* eeprom, ASM_CODE* &aktCode, STACK* &call, STACK* &interupt){__asm__lastError = NOT_IMPLEMENTED;return -1;}
int instructions::GOTO(void*k, void*ign, ASM_CODE* code, int& w, char* ram, char* eeprom, ASM_CODE* &aktCode, STACK* &call, STACK* &interupt){__asm__lastError = NOT_IMPLEMENTED;return -1;}
int instructions::IORLW(void*k, void*ign, ASM_CODE* code, int& w, char* ram, char* eeprom, ASM_CODE* &aktCode, STACK* &call, STACK* &interupt){__asm__lastError = NOT_IMPLEMENTED;return -1;}
int instructions::MOVLW(void*k, void*ign, ASM_CODE* code, int& w, char* ram, char* eeprom, ASM_CODE* &aktCode, STACK* &call, STACK* &interupt){__asm__lastError = NOT_IMPLEMENTED;return -1;}
int instructions::RETFIE(void*ign1, void*ign2, ASM_CODE* code, int& w, char* ram, char* eeprom, ASM_CODE* &aktCode, STACK* &call, STACK* &interupt){__asm__lastError = NOT_IMPLEMENTED;return -1;}
int instructions::RETLW(void*k, void*ign2, ASM_CODE* code, int& w, char* ram, char* eeprom, ASM_CODE* &aktCode, STACK* &call, STACK* &interupt){__asm__lastError = NOT_IMPLEMENTED;return -1;}
int instructions::RETURN(void*ign1, void*ign2, ASM_CODE* code, int& w, char* ram, char* eeprom, ASM_CODE* &aktCode, STACK* &call, STACK* &interupt){__asm__lastError = NOT_IMPLEMENTED;return -1;}
int instructions::SLEEP(void*ign1, void*ign2, ASM_CODE* code, int& w, char* ram, char* eeprom, ASM_CODE* &aktCode, STACK* &call, STACK* &interupt){__asm__lastError = NOT_IMPLEMENTED;return -1;}
int instructions::SUBLW(void*k, void*ign, ASM_CODE* code, int& w, char* ram, char* eeprom, ASM_CODE* &aktCode, STACK* &call, STACK* &interupt){__asm__lastError = NOT_IMPLEMENTED;return -1;}
int instructions::XORLW(void*k, void*ign, ASM_CODE* code, int& w, char* ram, char* eeprom, ASM_CODE* &aktCode, STACK* &call, STACK* &interupt){__asm__lastError = NOT_IMPLEMENTED;return -1;}

char * instructions::getLastError()
{
	return __asm__lastError;
}
