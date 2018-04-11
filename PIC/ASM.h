#pragma once
#include "DEBUG.H"

struct ASM_TEXT;
struct ASM_CODE;
struct ASM;
struct STACK;

typedef struct ASM_TEXT ASM_TEXT;
typedef struct ASM_CODE ASM_CODE;
typedef struct ASM ASM;
typedef struct STACK STACK;

typedef int(*instruction_t)(void*, void*, ASM_CODE*, int&, char*, char*, ASM_CODE*&, STACK*&, STACK*&);

struct ASM_TEXT{
	char *bytecode, *lineOfCode, *label, *asmCode, *comment;
	ASM_TEXT* next;
};

 struct ASM_CODE{
	instruction_t function;
	void *param1, *param2;
	ASM_TEXT* guiText;
};

struct ASM{
	ASM_TEXT* text;
	ASM_CODE* code;
};

struct STACK {
	ASM_CODE* jumpTo;
	STACK* next;
};



namespace instructions {
	int ADDWF(void*f, void*d, ASM_CODE* code, int& w, char* ram, char* eeprom, ASM_CODE* &aktCode, STACK* &call, STACK* &interupt);
	int ANDWF(void*f, void*d, ASM_CODE* code, int& w, char* ram, char* eeprom, ASM_CODE* &aktCode, STACK* &call, STACK* &interupt);
	int CLRF(void*f, void*ign, ASM_CODE* code, int& w, char* ram, char* eeprom, ASM_CODE* &aktCode, STACK* &call, STACK* &interupt);
	int CLRW(void*ign1, void*ing2, ASM_CODE* code, int& w, char* ram, char* eeprom, ASM_CODE* &aktCode, STACK* &call, STACK* &interupt);
	int COMF(void*f, void*d, ASM_CODE* code, int& w, char* ram, char* eeprom, ASM_CODE* &aktCode, STACK* &call, STACK* &interupt);
	int DECF(void*f, void*d, ASM_CODE* code, int& w, char* ram, char* eeprom, ASM_CODE* &aktCode, STACK* &call, STACK* &interupt);
	int DECFSZ(void*f, void*d, ASM_CODE* code, int& w, char* ram, char* eeprom, ASM_CODE* &aktCode, STACK* &call, STACK* &interupt);
	int INCF(void*f, void*d, ASM_CODE* code, int& w, char* ram, char* eeprom, ASM_CODE* &aktCode, STACK* &call, STACK* &interupt);
	int INCFSZ(void*f, void*d, ASM_CODE* code, int& w, char* ram, char* eeprom, ASM_CODE* &aktCode, STACK* &call, STACK* &interupt);
	int IORWF(void*f, void*d, ASM_CODE* code, int& w, char* ram, char* eeprom, ASM_CODE* &aktCode, STACK* &call, STACK* &interupt);
	int MOVF(void*f, void*d, ASM_CODE* code, int& w, char* ram, char* eeprom, ASM_CODE* &aktCode, STACK* &call, STACK* &interupt);
	int MOVWF(void*f, void*ign, ASM_CODE* code, int& w, char* ram, char* eeprom, ASM_CODE* &aktCode, STACK* &call, STACK* &interupt);
	int NOP(void*ign1, void*ign2, ASM_CODE* code, int& w, char* ram, char* eeprom, ASM_CODE* &aktCode, STACK* &call, STACK* &interupt);
	int RLF(void*f, void*d, ASM_CODE* code, int& w, char* ram, char* eeprom, ASM_CODE* &aktCode, STACK* &call, STACK* &interupt);
	int RRF(void*f, void*d, ASM_CODE* code, int& w, char* ram, char* eeprom, ASM_CODE* &aktCode, STACK* &call, STACK* &interupt);
	int SUBWF(void*f, void*d, ASM_CODE* code, int& w, char* ram, char* eeprom, ASM_CODE* &aktCode, STACK* &call, STACK* &interupt);
	int SWAPF(void*f, void*d, ASM_CODE* code, int& w, char* ram, char* eeprom, ASM_CODE* &aktCode, STACK* &call, STACK* &interupt);
	int XORWF(void*f, void*d, ASM_CODE* code, int& w, char* ram, char* eeprom, ASM_CODE* &aktCode, STACK* &call, STACK* &interupt);
	
	int BCF(void*f, void*b, ASM_CODE* code, int& w, char* ram, char* eeprom, ASM_CODE* &aktCode, STACK* &call, STACK* &interupt);
	int BSF(void*f, void*b, ASM_CODE* code, int& w, char* ram, char* eeprom, ASM_CODE* &aktCode, STACK* &call, STACK* &interupt);
	int BTFSC(void*f, void*b, ASM_CODE* code, int& w, char* ram, char* eeprom, ASM_CODE* &aktCode, STACK* &call, STACK* &interupt);
	int BTFSS(void*f, void*b, ASM_CODE* code, int& w, char* ram, char* eeprom, ASM_CODE* &aktCode, STACK* &call, STACK* &interupt);

	int ADDLW(void*k, void*ign, ASM_CODE* code, int& w, char* ram, char* eeprom, ASM_CODE* &aktCode, STACK* &call, STACK* &interupt);
	int ANDLW(void*k, void*ign, ASM_CODE* code, int& w, char* ram, char* eeprom, ASM_CODE* &aktCode, STACK* &call, STACK* &interupt);
	int CALL(void*k, void*ign, ASM_CODE* code, int& w, char* ram, char* eeprom, ASM_CODE* &aktCode, STACK* &call, STACK* &interupt);
	int CLRWDT(void*ign1, void*ign2, ASM_CODE* code, int& w, char* ram, char* eeprom, ASM_CODE* &aktCode, STACK* &call, STACK* &interupt);
	int GOTO(void*k, void*ign, ASM_CODE* code, int& w, char* ram, char* eeprom, ASM_CODE* &aktCode, STACK* &call, STACK* &interupt);
	int IORLW(void*k, void*ign, ASM_CODE* code, int& w, char* ram, char* eeprom, ASM_CODE* &aktCode, STACK* &call, STACK* &interupt);
	int MOVLW(void*k, void*ign, ASM_CODE* code, int& w, char* ram, char* eeprom, ASM_CODE* &aktCode, STACK* &call, STACK* &interupt);
	int RETFIE(void*ign1, void*ign2, ASM_CODE* code, int& w, char* ram, char* eeprom, ASM_CODE* &aktCode, STACK* &call, STACK* &interupt);
	int RETLW(void*k, void*ign2, ASM_CODE* code, int& w, char* ram, char* eeprom, ASM_CODE* &aktCode, STACK* &call, STACK* &interupt);
	int RETURN(void*ign1, void*ign2, ASM_CODE* code, int& w, char* ram, char* eeprom, ASM_CODE* &aktCode, STACK* &call, STACK* &interupt);
	int SLEEP(void*ign1, void*ign2, ASM_CODE* code, int& w, char* ram, char* eeprom, ASM_CODE* &aktCode, STACK* &call, STACK* &interupt);
	int SUBLW(void*k, void*ign, ASM_CODE* code, int& w, char* ram, char* eeprom, ASM_CODE* &aktCode, STACK* &call, STACK* &interupt);
	int XORLW(void*k, void*ign, ASM_CODE* code, int& w, char* ram, char* eeprom, ASM_CODE* &aktCode, STACK* &call, STACK* &interupt);

	char* getLastError();
}