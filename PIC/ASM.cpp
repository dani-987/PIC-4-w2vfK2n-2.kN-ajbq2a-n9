#include "ASM.h"

int instructions::ADDWF(void*f, void*d, Backend* call) { return call->ADDWF(f,d); }
int instructions::ANDWF(void*f, void*d, Backend* call){ return call->ANDWF(f,d); }
int instructions::CLRF(void*f, void*ign, Backend* call){return call->CLRF(f,ign);}
int instructions::CLRW(void*ign1, void*ign2, Backend* call){return call->CLRW(ign1,ign2);}
int instructions::COMF(void*f, void*d, Backend* call){return call->COMF(f,d);}
int instructions::DECF(void*f, void*d, Backend* call){return call->DECF(f,d);}
int instructions::DECFSZ(void*f, void*d, Backend* call){return call->DECFSZ(f,d);}
int instructions::INCF(void*f, void*d, Backend* call){return call->INCF(f,d);}
int instructions::INCFSZ(void*f, void*d, Backend* call){return call->INCFSZ(f,d);}
int instructions::IORWF(void*f, void*d, Backend* call){return call->IORWF(f,d);}
int instructions::MOVF(void*f, void*d, Backend* call){return call->MOVF(f,d);}
int instructions::MOVWF(void*f, void*ign, Backend* call){return call->MOVWF(f,ign);}
int instructions::NOP(void*ign1, void*ign2, Backend* call){return call->NOP(ign1,ign2);}
int instructions::RLF(void*f, void*d, Backend* call){return call->RLF(f,d);}
int instructions::RRF(void*f, void*d, Backend* call){return call->RRF(f,d);}
int instructions::SUBWF(void*f, void*d, Backend* call){return call->SUBWF(f,d);}
int instructions::SWAPF(void*f, void*d, Backend* call){return call->SWAPF(f,d);}
int instructions::XORWF(void*f, void*d, Backend* call){return call->XORWF(f,d);}

int instructions::BCF(void*f, void*b, Backend* call){ return call->BCF(f,b); }
int instructions::BSF(void*f, void*b, Backend* call){ return call->BSF(f,b); }
int instructions::BTFSC(void*f, void*b, Backend* call){ return call->BTFSC(f,b); }
int instructions::BTFSS(void*f, void*b, Backend* call){ return call->BTFSS(f,b); }

int instructions::ADDLW(void*k, void*ign, Backend* call){ return call->ADDLW(k,ign); }
int instructions::ANDLW(void*k, void*ign, Backend* call){ return call->ANDLW(k,ign); }
int instructions::CALL(void*k, void*ign, Backend* call){ return call->CALL(k,ign); }
int instructions::CLRWDT(void*ign1, void*ign2, Backend* call){return call->CLRWDT(ign1,ign2);}
int instructions::GOTO(void*k, void*ign, Backend* call){return call->GOTO(k,ign);}
int instructions::IORLW(void*k, void*ign, Backend* call){return call->IORLW(k,ign);}
int instructions::MOVLW(void*k, void*ign, Backend* call){return call->MOVLW(k,ign);}
int instructions::RETFIE(void*ign1, void*ign2, Backend* call){return call->RETFIE(ign1,ign2);}
int instructions::RETLW(void*k, void*ign, Backend* call){return call->RETLW(k,ign);}
int instructions::RETURN(void*ign1, void*ign2, Backend* call){return call->RETURN(ign1,ign2);}
int instructions::SLEEP(void*ign1, void*ign2, Backend* call){return call->SLEEP(ign1,ign2);}
int instructions::SUBLW(void*k, void*ign, Backend* call){return call->SUBLW(k,ign);}
int instructions::XORLW(void*k, void*ign, Backend* call){return call->XORLW(k,ign);}

