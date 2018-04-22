#pragma once

class Compiler;
struct scannerstring;

typedef struct scannerstring scannerstring;

#include <fstream>
#include <Windows.h>
#include "DEBUG.H"
#include "ASM.h"

//datastruture for saving strings during compilertime as a pointerlist, to save time during adding signs 
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

	//loads a fileline into ASM_CODE and ASM_TEXT structures
	bool decodeLine(ASM_CODE** code, int* len, ASM_TEXT** text, int* pos, FILE* file);
	//codes an asm-command into a functionpointer
	bool decodeInstruction(int code, ASM_CODE* decoded);
	//appends a sign onto the string
	bool appendToString(scannerstring** string, scannerstring** aktPosInString, char sign, int& len);
	//converts scannerstring to char* and frees the string
	char* scannerString2PChar(scannerstring* string, int len);
	//only frees the scannerstring
	void freeScannerString(scannerstring* toFree);
	//gets the next sign of the file, the functions sets the flag 'fileReadedTilEnd'
	char getNextChar(FILE* file);
public:
	Compiler();

	//loads a .LST-file into a ASM-datastructure. Do not forget to call 'Compiler::freeASM' !
	ASM* compileFile(char* file, int memsize);
	//frees a ASM-datastructe
	static void freeASM(ASM* toFree);
	//gets the error, if compileFile returns 'nullptr'
	char* getCompilerError();

	//returns the name of a functionpointer, for debugging use only!
	static char* functionPointerToName(instruction_t);
};