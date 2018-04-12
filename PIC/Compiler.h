#pragma once

class Compiler;
struct scannerstring;

typedef struct scannerstring scannerstring;

#include <fstream>
#include <Windows.h>
#include "DEBUG.H"
#include "ASM.h"

struct scannerstring {
	char sign;
	scannerstring* next;
};

class Compiler {
private:
	char* lastError;

	char* puffer;
	int bytesReaded, aktPufferPosition;
	DWORD pufferSize;
	bool fileReadedTilEnd;

	bool decodeLine(ASM_CODE** code, int* len, ASM_TEXT** text, int* pos, FILE* file);
	bool decodeInstruction(int code, ASM_CODE* decoded);
	bool appendToString(scannerstring** string, scannerstring** aktPosInString, char sign, int& len);
	char* scannerString2PChar(scannerstring* string, int len);
	void freeScannerString(scannerstring* toFree);
	char getNextChar(FILE* file);
public:
	Compiler();
	ASM* compileFile(char* file, int memsize);
	void freeASM(ASM* toFree);
	char* getCompilerError();

	static char* functionPointerToName(instruction_t);
};