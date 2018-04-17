#pragma once

struct ASM_TEXT;
struct ASM_CODE;
struct ASM;
struct STACK;

typedef struct ASM_TEXT ASM_TEXT;
typedef struct ASM_CODE ASM_CODE;
typedef struct ASM ASM;
typedef struct STACK STACK;

#include "DEBUG.H"
#include "Backend.h"

typedef int(*instruction_t)(void*, void*, Backend*);

struct ASM_TEXT{
	char *bytecode, *lineOfCode, *label, *asmCode, *comment;
	int lineNumber;
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
	bool isInterupt;
};



namespace instructions {
	int ADDWF(void*f, void*d, Backend* call);
	int ANDWF(void*f, void*d, Backend* call);
	int CLRF(void*f, void*ign, Backend* call);
	int CLRW(void*ign1, void*ign2, Backend* call);
	int COMF(void*f, void*d, Backend* call);
	int DECF(void*f, void*d, Backend* call);
	int DECFSZ(void*f, void*d, Backend* call);
	int INCF(void*f, void*d, Backend* call);
	int INCFSZ(void*f, void*d, Backend* call);
	int IORWF(void*f, void*d, Backend* call);
	int MOVF(void*f, void*d, Backend* call);
	int MOVWF(void*f, void*ign, Backend* call);
	int NOP(void*ign1, void*ign2, Backend* call);
	int RLF(void*f, void*d, Backend* call);
	int RRF(void*f, void*d, Backend* call);
	int SUBWF(void*f, void*d, Backend* call);
	int SWAPF(void*f, void*d, Backend* call);
	int XORWF(void*f, void*d, Backend* call);
	
	int BCF(void*f, void*b, Backend* call);
	int BSF(void*f, void*b, Backend* call);
	int BTFSC(void*f, void*b, Backend* call);
	int BTFSS(void*f, void*b, Backend* call);

	int ADDLW(void*k, void*ign, Backend* call);
	int ANDLW(void*k, void*ign, Backend* call);
	int CALL(void*k, void*ign, Backend* call);
	int CLRWDT(void*ign1, void*ign2, Backend* call);
	int GOTO(void*k, void*ign, Backend* call);
	int IORLW(void*k, void*ign, Backend* call);
	int MOVLW(void*k, void*ign, Backend* call);
	int RETFIE(void*ign1, void*ign2, Backend* call);
	int RETLW(void*k, void*ign, Backend* call);
	int RETURN(void*ign1, void*ign2, Backend* call);
	int SLEEP(void*ign1, void*ign2, Backend* call);
	int SUBLW(void*k, void*ign, Backend* call);
	int XORLW(void*k, void*ign, Backend* call);
}