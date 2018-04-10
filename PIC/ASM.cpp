#include "ASM.h"

void instructions::ADDWF(void*f, void*d) {}
void instructions::ANDWF(void*f, void*d) {}
void instructions::CLRF(void*f, void*ign){}
void instructions::CLRW(void*ign1, void*ing2){}
void instructions::COMF(void*f, void*d){}
void instructions::DECF(void*f, void*d){}
void instructions::DECFSZ(void*f, void*d){}
void instructions::INCF(void*f, void*d){}
void instructions::INCFSZ(void*f, void*d){}
void instructions::IORWF(void*f, void*d){}
void instructions::MOVF(void*f, void*d){}
void instructions::MOVWF(void*f, void*ign){}
void instructions::NOP(void*ign1, void*ign2){}
void instructions::RLF(void*f, void*d){}
void instructions::RRF(void*f, void*d){}
void instructions::SUBWF(void*f, void*d){}
void instructions::SWAPF(void*f, void*d){}
void instructions::XORWF(void*f, void*d){}

void instructions::BCF(void*f, void*b){}
void instructions::BSF(void*f, void*b){}
void instructions::BTFSC(void*f, void*b){}
void instructions::BTFSS(void*f, void*b){}

void instructions::ADDLW(void*k, void*ign){}
void instructions::ANDLW(void*k, void*ign){}
void instructions::CALL(void*k, void*ign){}
void instructions::CLRWDT(void*ign1, void*ign2){}
void instructions::GOTO(void*k, void*ign){}
void instructions::IORLW(void*k, void*ign){}
void instructions::MOVLW(void*k, void*ign){}
void instructions::RETFIE(void*ign1, void*ign2){}
void instructions::RETLW(void*k, void*ign2){}
void instructions::RETURN(void*ign1, void*ign2){}
void instructions::SLEEP(void*ign1, void*ign2){}
void instructions::SUBLW(void*k, void*ign){}
void instructions::XORLW(void*k, void*ign){}