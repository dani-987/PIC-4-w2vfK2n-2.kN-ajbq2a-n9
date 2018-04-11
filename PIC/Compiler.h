#pragma once
#include "DEBUG.H"
#include "ASM.h"

ASM* compileFile(char* file, int memsize);
void freeASM(ASM* toFree);
char* getCompilerError();

char* functionPointerToName(void(*f)(void*, void*));