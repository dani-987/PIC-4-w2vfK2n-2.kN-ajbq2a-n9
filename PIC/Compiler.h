#pragma once
#include "ASM.h"

ASM* compileFile(char* file, int memsize);
char* getCompilerError();