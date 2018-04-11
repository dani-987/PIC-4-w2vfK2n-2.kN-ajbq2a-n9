#pragma once
#include "DEBUG.H"

typedef struct ASM_TEXT{
	char *bytecode, *lineOfCode, *label, *asmCode, *comment;
	ASM_TEXT* next;
}ASM_TEXT;

typedef struct{
	void (*function)(void*, void*), *param1, *param2;
	ASM_TEXT* guiText;
}ASM_CODE;

typedef struct{
	ASM_TEXT* text;
	ASM_CODE* code;
}ASM;

namespace instructions {
	void ADDWF(void*f, void*d);
	void ANDWF(void*f, void*d);
	void CLRF(void*f, void*ign);
	void CLRW(void*ign1, void*ing2);
	void COMF(void*f, void*d);
	void DECF(void*f, void*d);
	void DECFSZ(void*f, void*d);
	void INCF(void*f, void*d);
	void INCFSZ(void*f, void*d);
	void IORWF(void*f, void*d);
	void MOVF(void*f, void*d);
	void MOVWF(void*f, void*ign);
	void NOP(void*ign1, void*ign2);
	void RLF(void*f, void*d);
	void RRF(void*f, void*d);
	void SUBWF(void*f, void*d);
	void SWAPF(void*f, void*d);
	void XORWF(void*f, void*d);
	
	void BCF(void*f, void*b);
	void BSF(void*f, void*b);
	void BTFSC(void*f, void*b);
	void BTFSS(void*f, void*b);

	void ADDLW(void*k, void*ign);
	void ANDLW(void*k, void*ign);
	void CALL(void*k, void*ign);
	void CLRWDT(void*ign1, void*ign2);
	void GOTO(void*k, void*ign);
	void IORLW(void*k, void*ign);
	void MOVLW(void*k, void*ign);
	void RETFIE(void*ign1, void*ign2);
	void RETLW(void*k, void*ign2);
	void RETURN(void*ign1, void*ign2);
	void SLEEP(void*ign1, void*ign2);
	void SUBLW(void*k, void*ign);
	void XORLW(void*k, void*ign);
}