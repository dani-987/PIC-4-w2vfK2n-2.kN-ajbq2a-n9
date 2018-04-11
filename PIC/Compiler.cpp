#include "Compiler.h"
#include <fstream>
#include <Windows.h>
#include <AtlConv.h>
#include <atlbase.h>

#define CANNOT_OPEN_FILE	"Kann Datei nicht öffnen."
#define MEMORY_MISSING		"Es fehlt an Arbeitsspeicher."

#define DEBUGLVL_NONE	0
#define DEBUGLVL_NORMAL	1
#define DEBUGLVL_MUCH	2
#define DEBUGLVL_ALL	3
VARDEF(int, DEBUGLVL, DEBUGLVL_NONE);

typedef struct scannerstring {
	char sign;
	scannerstring* next;
}scannerstring;

#define STATUS_START					0
#define STATUS_READING_CODE1			1
#define STATUS_READING_BEFORE_CODE2		2
#define STATUS_READING_CODE2			3
#define STATUS_READING_BEFORE_LINE		4
#define STATUS_READING_LINE				5
#define STATUS_READING_BEFORE_LABEL1	6
#define STATUS_READING_BEFORE_LABEL2	7
#define STATUS_READING_LABEL			8
#define STATUS_READING_BEFORE_ASM		9
#define STATUS_READING_ASM				10
#define STATUS_READING_KOMMENT			11


char* __compiler__lastError = "Kein Fehler aufgetreten!";

char* puffer = nullptr;
int bytesReaded = 0, aktPufferPosition = 0;
DWORD pufferSize = 0;
bool fileReadedTilEnd = false;

bool decodeLine(ASM_CODE** code, int* len, ASM_TEXT** text, int* pos, FILE* file);
bool decodeInstruction(int code, ASM_CODE* decoded);
bool appendToString(scannerstring** string, scannerstring** aktPosInString, char sign, int& len);
char* scannerString2PChar(scannerstring* string, int len);
void freeScannerString(scannerstring* toFree);
char getNextChar(FILE* file);

ASM * compileFile(char * file, int memsize)
{
	FILE* f = fopen(file, "r");
	DOIF(DEBUGLVL >= DEBUGLVL_MUCH)PRINTF2("Function: compileFile((char*):'%s', (int):%d)\n", file, memsize);
	if (!f) { 
		__compiler__lastError = CANNOT_OPEN_FILE;
		return nullptr;
	}
	bytesReaded = 0;
	aktPufferPosition = 0;
	if (!GetDiskFreeSpace(CA2A(file), nullptr, &pufferSize, nullptr, nullptr)) {
		pufferSize = 64 * 1024;
	}
	DOIF(DEBUGLVL >= DEBUGLVL_MUCH)PRINTF1("PUFFERSIZE is set to: %d\n", pufferSize);
	puffer = (char*)malloc(pufferSize * sizeof(char));
	if (puffer == nullptr) {
		__compiler__lastError = MEMORY_MISSING;
		fclose(f);
		return nullptr;
	}
	fileReadedTilEnd = false;
	ASM_CODE* asmcode = (ASM_CODE*)malloc(sizeof(ASM_CODE)*memsize);
	memset(asmcode, 0, sizeof(ASM_CODE)*memsize);
	if (asmcode == nullptr) {
		free(puffer);
		__compiler__lastError = MEMORY_MISSING;
		fclose(f);
		return nullptr;
	}
	ASM_TEXT* aktLine = nullptr, *startLine = nullptr;
	ASM* retASM = (ASM*)malloc(sizeof(ASM));
	int aktPosInCode = 0, codeLen = 0;
	if (retASM == nullptr) {
		free(puffer);
		free(asmcode);
		__compiler__lastError = MEMORY_MISSING;
		fclose(f);
		return nullptr;
	}
	retASM->code = asmcode;
	retASM->text = nullptr;
	bytesReaded = -1;
	DOIF(DEBUGLVL >= DEBUGLVL_NORMAL)PRINTF("START COMPILING...\n");
	while (!fileReadedTilEnd) {
		if (!decodeLine(&asmcode, &codeLen, &aktLine, &aktPosInCode, f)) {
			PRINTF("ERROR DURING COMPILING!\n");
			goto ERROR_END;
		}
		else if(startLine == nullptr){
			startLine = aktLine;
		}
		DOIF(DEBUGLVL >= DEBUGLVL_NORMAL && aktLine != nullptr)
			PRINTF6("\tLINE: '%s'\n\tASM: '%s'\n\tCOM: '%s'\n\tLABEL: '%s'\n\tBytecode: '%s'\n\tFunctionpointer: '%s'\n\n", aktLine->lineOfCode, aktLine->asmCode, aktLine->comment, aktLine->label, aktLine->bytecode, (aktLine->bytecode != nullptr && asmcode != nullptr)?functionPointerToName(asmcode->function):nullptr);
		DOIF(DEBUGLVL >= DEBUGLVL_MUCH)PRINTF2("\tPOS: %d\n\tCODE-LEN: %d\n", aktPosInCode, codeLen);
		if (codeLen > memsize) {
			__compiler__lastError = "Programm to long.";
			goto ERROR_END;
		}
	}
	free(puffer);
	retASM->text = startLine;
	fclose(f);
	return retASM;
ERROR_END:
	free(puffer);
	PRINTF1("ERROR_END in compiler; __compiler__lastError: '%s'\n\n", __compiler__lastError);
	retASM->text = startLine;
	freeASM(retASM);
	fclose(f);
	return nullptr;
}

bool decodeLine(ASM_CODE** code, int* len, ASM_TEXT** text, int* pos, FILE* file) {
	DOIF(DEBUGLVL >= DEBUGLVL_MUCH)PRINTF1("Function: decodeLine(code, (int):%d, text, file)\n", *pos);
	char sign, status = 0;
	ASM_TEXT* newText = nullptr;
	scannerstring* string = nullptr, *aktPosInString = nullptr;
	int codeVal = 0, stringLen;
	while (1) {
		sign = getNextChar(file);
		DOIF(DEBUGLVL >= DEBUGLVL_ALL)PRINTF3("READED SIGN: '%c' in STATUS: %d and EOF: %d\n", sign, status, fileReadedTilEnd);
		if (fileReadedTilEnd || sign == '\n') {
			if (status == STATUS_READING_ASM) {
				newText->asmCode = scannerString2PChar(string, stringLen);
				string = nullptr;
				if (newText->asmCode == nullptr) {
					goto ERROR_END;
				}
			}
			else if (status == STATUS_READING_KOMMENT) {
				newText->comment = scannerString2PChar(string, stringLen);
				string = nullptr;
				if (newText->comment == nullptr) {
					goto ERROR_END;
				}
			}
			else if (status == STATUS_READING_CODE1 || 
				status == STATUS_READING_BEFORE_CODE2 ||
				status == STATUS_READING_CODE2) {
				if(fileReadedTilEnd)
					__compiler__lastError = "Unerwartetes Dateiende während des Lesens des Maschinencodes.";
				else __compiler__lastError = "Unerwartetes Zeilenende während des Lesens des Maschinencodes.";
				goto ERROR_END; 
			}
			else if((*code)->function != nullptr){
				if (status == STATUS_READING_BEFORE_LINE ||
					status == STATUS_READING_BEFORE_LABEL1 ||
					status == STATUS_READING_BEFORE_LABEL2 ||
					status == STATUS_READING_LABEL ||
					status == STATUS_READING_BEFORE_ASM) {
					if (fileReadedTilEnd)
						__compiler__lastError = "Unerwartetes Dateiende, der Assemblercode fehlt.";
					else __compiler__lastError = "Unerwartetes Zeilenende, der Assemblercode fehlt.";
					goto ERROR_END;
				}
				goto ERRORLESS_END;
			}
			goto ERRORLESS_END;
		}
		switch (status) {
		case STATUS_START:
			switch (sign) {
			case ' ': case '\t':
				status = STATUS_READING_BEFORE_LINE;
				break;
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
				status = STATUS_READING_CODE1;
				if (!appendToString(&string, &aktPosInString, sign, stringLen)) { goto ERROR_END; }
				codeVal = sign - '0';
				break;
			case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
				status = STATUS_READING_CODE1;
				if (!appendToString(&string, &aktPosInString, sign + ('A' - 'a'), stringLen)) { goto ERROR_END; }
				codeVal = sign + (10 - 'a');
				break;
			case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
				status = STATUS_READING_CODE1;
				if (!appendToString(&string, &aktPosInString, sign, stringLen)) { goto ERROR_END; }
				codeVal = sign + (10 - 'A');
				break;
			default:
				__compiler__lastError = "Unexpected Sign! Expected Space, Tab, 0-9, a-f, A-F at Linebegin.";
				goto ERROR_END;
			}
			break;
		case STATUS_READING_CODE1:
			switch (sign) {
			case ' ': case '\t':
				status = STATUS_READING_BEFORE_CODE2;
				if (!appendToString(&string, &aktPosInString, ' ', stringLen)) { goto ERROR_END; }
				break;
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
				if (!appendToString(&string, &aktPosInString, sign, stringLen)) { goto ERROR_END; }
				codeVal = (codeVal << 4) + sign - '0';
				break;
			case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
				if (!appendToString(&string, &aktPosInString, sign + ('A' - 'a'), stringLen)) { goto ERROR_END; }
				codeVal = (codeVal << 4) + sign + (10 - 'a');
				break;
			case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
				if (!appendToString(&string, &aktPosInString, sign, stringLen)) { goto ERROR_END; }
				codeVal = (codeVal << 4) + sign + (10 - 'A');
				break;
			default:
				__compiler__lastError = "Unexpected Sign! Expected Space, Tab, 0-9, a-f, A-F while reading machine code.";
				goto ERROR_END;
			}
			break;
		case STATUS_READING_BEFORE_CODE2:
			switch (sign) {
			case ' ': case '\t':
				break;
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
				status = STATUS_READING_CODE2;
				if (!appendToString(&string, &aktPosInString, sign, stringLen)) { goto ERROR_END; }
				codeVal = (codeVal << 4) + sign - '0';
				break;
			case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
				status = STATUS_READING_CODE2;
				if (!appendToString(&string, &aktPosInString, sign + ('A' - 'a'), stringLen)) { goto ERROR_END; }
				codeVal = (codeVal << 4) + sign + (10 - 'a');
				break;
			case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
				status = STATUS_READING_CODE2;
				if (!appendToString(&string, &aktPosInString, sign, stringLen)) { goto ERROR_END; }
				codeVal = (codeVal << 4) + sign + (10 - 'A');
				break;
			default:
				__compiler__lastError = "Unexpected Sign! Expected Space, Tab, 0-9, a-f, A-F while reading machine code.";
				goto ERROR_END;
			}
			break;
		case STATUS_READING_CODE2:
			switch (sign) {
			case ' ': case '\t':
				status = STATUS_READING_BEFORE_LINE;
				newText = (ASM_TEXT*)malloc(sizeof(ASM_TEXT));
				if (newText == nullptr) {
					__compiler__lastError = MEMORY_MISSING;
					goto ERROR_END;
				}
				newText->lineOfCode = nullptr;
				newText->label = nullptr;
				newText->asmCode = nullptr;
				newText->comment = nullptr;
				newText->next = nullptr;
				newText->bytecode = scannerString2PChar(string, stringLen);
				string = nullptr;
				if (newText->bytecode == nullptr) {goto ERROR_END;}
				(*code)->guiText = newText;
				//set functionpointers and params
				if (!decodeInstruction(codeVal, (*code))) {goto ERROR_END;}
				break;
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
				if (!appendToString(&string, &aktPosInString, sign, stringLen)) { goto ERROR_END; }
				codeVal = (codeVal << 4) + sign - '0';
				break;
			case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
				if (!appendToString(&string, &aktPosInString, sign + ('A' - 'a'), stringLen)) { goto ERROR_END; }
				codeVal = (codeVal << 4) + sign + (10 - 'a');
				break;
			case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
				if (!appendToString(&string, &aktPosInString, sign, stringLen)) { goto ERROR_END; }
				codeVal = (codeVal << 4) + sign + (10 - 'A');
				break;
			default:
				__compiler__lastError = "Unexpected Sign! Expected Space, Tab, 0-9, a-f, A-F while reading machine code.";
				goto ERROR_END;
			}
			break;
		case STATUS_READING_BEFORE_LINE:
			if (sign == ' ' || sign == '\t');
			else if (sign >= '0' && sign <= '9') {
				if (!appendToString(&string, &aktPosInString, sign, stringLen)) { goto ERROR_END; }
				status = STATUS_READING_LINE;
			}
			else {
				__compiler__lastError = "Unexpected Sign! Expected Space, Tab, 0-9 for Linenumber!";
				goto ERROR_END;
			}
			break;
		case STATUS_READING_LINE:
			if (sign == ' ' || sign == '\t') {
				if (newText != nullptr) {
					newText->lineOfCode = scannerString2PChar(string, stringLen);
					string = nullptr;
					if (newText->lineOfCode == nullptr) {goto ERROR_END;}
				}
				else {
					newText = (ASM_TEXT*)malloc(sizeof(ASM_TEXT));
					if (newText == nullptr) {
						__compiler__lastError = MEMORY_MISSING;
						goto ERROR_END;
					}
					newText->bytecode = nullptr;
					newText->lineOfCode = scannerString2PChar(string, stringLen);
					string = nullptr;
					if (newText->lineOfCode == nullptr) {goto ERROR_END;}
					newText->label = nullptr;
					newText->asmCode = nullptr;
					newText->comment = nullptr;
					newText->next = nullptr;
				}
				status = STATUS_READING_BEFORE_LABEL1;
			}
			else if (sign >= '0' && sign <= '9') {
				if (!appendToString(&string, &aktPosInString, sign, stringLen)) { goto ERROR_END; }
			}
			else {
				__compiler__lastError = "Unexpected Sign! Expected Space, Tab, 0-9 for Linenumber!";
				goto ERROR_END;
			}
			break;
		case STATUS_READING_BEFORE_LABEL1:
			if (sign == ' ' || sign == '\t')status = STATUS_READING_BEFORE_LABEL2;
			else if ((sign >= 'a' && sign <= 'z') ||
				(sign >= 'A' && sign <= 'Z') ||
				(sign >= '0' && sign <= '9')) {
				status = STATUS_READING_LABEL;
				if (!appendToString(&string, &aktPosInString, sign, stringLen)){goto ERROR_END;}
			}
			else {
				__compiler__lastError = "Unexpected Sign! Expected Space, Tab, a-z, A-Z, 0-9 for Label!";
				goto ERROR_END;
			}
			break;
		case STATUS_READING_BEFORE_LABEL2:
			if (sign == ' ' || sign == '\t')status = STATUS_READING_BEFORE_ASM;
			else if ((sign >= 'a' && sign <= 'z') ||
				(sign >= 'A' && sign <= 'Z') ||
				(sign >= '0' && sign <= '9')) {
				status = STATUS_READING_LABEL;
				if (!appendToString(&string, &aktPosInString, sign, stringLen)) { goto ERROR_END; }
			}
			else {
				__compiler__lastError = "Unexpected Sign! Expected Space, Tab, a-z, A-Z, 0-9 for Label!";
				goto ERROR_END;
			}
			break;
		case STATUS_READING_LABEL:
			if (sign == ' ' || sign == '\t') {
				newText->label = scannerString2PChar(string, stringLen);
				string = nullptr;
				if (newText->label == nullptr) {goto ERROR_END;}
				status = STATUS_READING_BEFORE_ASM;
			}
			else  if ((sign >= 'a' && sign <= 'z') ||
				(sign >= 'A' && sign <= 'Z') ||
				(sign >= '0' && sign <= '9')) {
				if (!appendToString(&string, &aktPosInString, sign, stringLen)) { goto ERROR_END; }
			}
			else {
				__compiler__lastError = "Unexpected Sign! Expected Space, Tab, a-z, A-Z, 0-9 for Label!";
				goto ERROR_END;
			}
			break;
		case STATUS_READING_BEFORE_ASM:
			switch (sign) {
			case ' ': case '\t':
				break;
			case ';':
				if ((*code)->function != nullptr) {
					__compiler__lastError = "Expected assembly-code. Commend found!";
					goto ERROR_END;
				}
				status = STATUS_READING_KOMMENT;
				break;
			default:
				status = STATUS_READING_ASM;
				if (!appendToString(&string, &aktPosInString, sign, stringLen)) { goto ERROR_END; }
			}
			break;
		case STATUS_READING_ASM:
			if (sign == ';') {
				newText->asmCode = scannerString2PChar(string, stringLen);
				string = nullptr;
				if (newText->asmCode == nullptr) {goto ERROR_END;}
				status = STATUS_READING_KOMMENT;
			}
			else {
				if (!appendToString(&string, &aktPosInString, sign, stringLen)) { goto ERROR_END;}
			}
			break;
		case STATUS_READING_KOMMENT:
			if (!appendToString(&string, &aktPosInString, sign, stringLen)) {goto ERROR_END;}
			break;
		}
	}
ERRORLESS_END:
	DOIF(newText != nullptr && newText->next != nullptr)PRINTF2("newText->next != nullptr (@%d, status: %d)\n", __LINE__, status);
	DOIF(DEBUGLVL >= DEBUGLVL_MUCH)PRINTF("ERRORLESS_END\n");
	if ((*code)->function != nullptr) {
		DOIF(DEBUGLVL >= DEBUGLVL_MUCH)PRINTF("NEW-CODE is set!\n");
		(*code)++;
		(*len)++;
	}
	if (newText != nullptr) {
		DOIF(DEBUGLVL >= DEBUGLVL_MUCH)PRINTF("NEW-TEXT is set!\n");
		if (*text != nullptr) {
			(*text)->next = newText;
			*text = newText;
			(*pos)++;
		}
		else {
			*text = newText;
			*pos = 1;
		}
	}
	DOIF(string != nullptr)PRINTF2("newText->next != nullptr (@%d, status: %d)\n", __LINE__, status);
	return true;
ERROR_END:
	PRINTF1("ERROR_END ############: '%s'\n", __compiler__lastError);
	if (newText != nullptr) {
		if (newText->bytecode != nullptr)free(newText->bytecode);
		if (newText->lineOfCode != nullptr)free(newText->lineOfCode);
		if (newText->label != nullptr)free(newText->label);
		if (newText->asmCode != nullptr)free(newText->asmCode);
		if (newText->comment != nullptr)free(newText->comment);
		DOIF(newText->next != nullptr)PRINTF1("newText->next != nullptr (@%d)\n", __LINE__);
		free(newText);
	}
	if(string != nullptr)freeScannerString(string);
	return false;
}

bool decodeInstruction(int code, ASM_CODE* decoded) {
	if (code & 0xC000) {
		decoded->function = instructions::NOP;
		decoded->param1 = nullptr;
		decoded->param2 = nullptr;
		return false;
	}
	if (code & 0x2000) { //1X XXXX XXXX XXXX
		if (code & 0x1000) { //11 XXXX XXXX XXXX
			if (code & 0x0800) { //11 1XXX XXXX XXXX
				if (code & 0x0400) { //11 11XX XXXX XXXX
					if (code & 0x0200) { //11 111X XXXX XXXX
						decoded->function = instructions::ADDLW;
						decoded->param1 = (void*)(code & 0x00FF);
						decoded->param2 = nullptr;
						return true;
					}
					else { //11 110X XXXX XXXX
						decoded->function = instructions::SUBLW;
						decoded->param1 = (void*)(code & 0x00FF);
						decoded->param2 = nullptr;
						return true;
					}
				}
				else { //11 10XX XXXX XXXX
					if (code & 0x0200) { //11 101X XXXX XXXX
						if (code & 0x0100) { //11 1011 XXXX XXXX
							decoded->function = instructions::NOP;
							decoded->param1 = nullptr;
							decoded->param2 = nullptr;
							return false;
						}
						else { //11 1010 XXXX XXXX
							decoded->function = instructions::XORLW;
							decoded->param1 = (void*)(code & 0x00FF);
							decoded->param2 = nullptr;
							return true;
						}
					}
					else { //11 100X XXXX XXXX
						if (code & 0x0100) { //11 1001 XXXX XXXX
							decoded->function = instructions::ANDLW;
							decoded->param1 = (void*)(code & 0x00FF);
							decoded->param2 = nullptr;
							return true;
						}
						else { //11 1000 XXXX XXXX
							decoded->function = instructions::IORLW;
							decoded->param1 = (void*)(code & 0x00FF);
							decoded->param2 = nullptr;
							return true;
						}
					}
				}
			}
			else { //11 0XXX XXXX XXXX
				if (code & 0x0400) { //11 01XX XXXX XXXX
					decoded->function = instructions::RETLW;
					decoded->param1 = (void*)(code & 0x00FF);
					decoded->param2 = nullptr;
					return true;
				}
				else { //11 00XX XXXX XXXX
					decoded->function = instructions::MOVLW;
					decoded->param1 = (void*)(code & 0x00FF);
					decoded->param2 = nullptr;
					return true;
				}
			}
		}
		else { //10 XXXX XXXX XXXX
			if (code & 0x0800) { //10 1XXX XXXX XXXX
				decoded->function = instructions::GOTO;
				decoded->param1 = (void*)(code & 0x07FF);
				decoded->param2 = nullptr;
				return true;
			}
			else { //10 0XXX XXXX XXXX
				decoded->function = instructions::CALL;
				decoded->param1 = (void*)(code & 0x07FF);
				decoded->param2 = nullptr;
				return true;
			}
		}
	}
	else { //0X XXXX XXXX XXXX
		if (code & 0x1000) { //01 XXXX XXXX XXXX
			if (code & 0x0800) { //01 1XXX XXXX XXXX
				if (code & 0x0400) { //01 11XX XXXX XXXX
					decoded->function = instructions::BTFSS;
					decoded->param1 = (void*)(code & 0x007F);
					decoded->param2 = (void*)((code >> 7) & 0x0007);
					return true;
				}
				else { //01 10XX XXXX XXXX
					decoded->function = instructions::BTFSC;
					decoded->param1 = (void*)(code & 0x007F);
					decoded->param2 = (void*)((code >> 7) & 0x0007);
					return true;
				}
			}
			else { //01 0XXX XXXX XXXX
				if (code & 0x0400) { //01 01XX XXXX XXXX
					decoded->function = instructions::BSF;
					decoded->param1 = (void*)(code & 0x007F);
					decoded->param2 = (void*)((code >> 7) & 0x0007);
					return true;
				}
				else { //01 00XX XXXX XXXX
					decoded->function = instructions::BCF;
					decoded->param1 = (void*)(code & 0x007F);
					decoded->param2 = (void*)((code >> 7) & 0x0007);
					return true;
				}
			}
		}
		else { //00 XXXX XXXX XXXX
			if (code & 0x0800) { //00 1XXX XXXX XXXX
				if (code & 0x0400) { //00 11XX XXXX XXXX
					if (code & 0x0200) { //00 111X XXXX XXXX
						if (code & 0x0100) { //00 1111 XXXX XXXX
							decoded->function = instructions::INCFSZ;
							decoded->param1 = (void*)(code & 0x007F);
							decoded->param2 = (void*)((code >> 7) & 0x0001);
							return true;
						}
						else { //00 1110 XXXX XXXX
							decoded->function = instructions::SWAPF;
							decoded->param1 = (void*)(code & 0x007F);
							decoded->param2 = (void*)((code >> 7) & 0x0001);
							return true;
						}
					}
					else { //00 110X XXXX XXXX
						if (code & 0x0100) { //00 1101 XXXX XXXX
							decoded->function = instructions::RLF;
							decoded->param1 = (void*)(code & 0x007F);
							decoded->param2 = (void*)((code >> 7) & 0x0001);
							return true;
						}
						else { //00 1100 XXXX XXXX
							decoded->function = instructions::RRF;
							decoded->param1 = (void*)(code & 0x007F);
							decoded->param2 = (void*)((code >> 7) & 0x0001);
							return true;
						}
					}
				}
				else { //00 10XX XXXX XXXX
					if (code & 0x0200) { //00 101X XXXX XXXX
						if (code & 0x0100) { //00 1011 XXXX XXXX
							decoded->function = instructions::DECFSZ;
							decoded->param1 = (void*)(code & 0x007F);
							decoded->param2 = (void*)((code >> 7) & 0x0001);
							return true;
						}
						else { //00 1010 XXXX XXXX
							decoded->function = instructions::INCF;
							decoded->param1 = (void*)(code & 0x007F);
							decoded->param2 = (void*)((code >> 7) & 0x0001);
							return true;
						}
					}
					else { //00 110X XXXX XXXX
						if (code & 0x0100) { //00 1001 XXXX XXXX
							decoded->function = instructions::COMF;
							decoded->param1 = (void*)(code & 0x007F);
							decoded->param2 = (void*)((code >> 7) & 0x0001);
							return true;
						}
						else { //00 1000 XXXX XXXX
							decoded->function = instructions::MOVF;
							decoded->param1 = (void*)(code & 0x007F);
							decoded->param2 = (void*)((code >> 7) & 0x0001);
							return true;
						}
					}
				}
			}
			else { //00 0XXX XXXX XXXX
				if (code & 0x0400) { //00 01XX XXXX XXXX
					if (code & 0x0200) { //00 011X XXXX XXXX
						if (code & 0x0100) { //00 0111 XXXX XXXX
							decoded->function = instructions::ADDWF;
							decoded->param1 = (void*)(code & 0x007F);
							decoded->param2 = (void*)((code >> 7) & 0x0001);
							return true;
						}
						else { //00 0110 XXXX XXXX
							decoded->function = instructions::XORWF;
							decoded->param1 = (void*)(code & 0x007F);
							decoded->param2 = (void*)((code >> 7) & 0x0001);
							return true;
						}
					}
					else { //00 010X XXXX XXXX
						if (code & 0x0100) { //00 0101 XXXX XXXX
							decoded->function = instructions::ANDWF;
							decoded->param1 = (void*)(code & 0x007F);
							decoded->param2 = (void*)((code >> 7) & 0x0001);
							return true;
						}
						else { //00 0100 XXXX XXXX
							decoded->function = instructions::IORWF;
							decoded->param1 = (void*)(code & 0x007F);
							decoded->param2 = (void*)((code >> 7) & 0x0001);
							return true;
						}
					}
				}
				else { //00 00XX XXXX XXXX
					if (code & 0x0200) { //00 001X XXXX XXXX
						if (code & 0x0100) { //00 0011 XXXX XXXX
							decoded->function = instructions::DECF;
							decoded->param1 = (void*)(code & 0x007F);
							decoded->param2 = (void*)((code >> 7) & 0x0001);
							return true;
						}
						else { //00 0010 XXXX XXXX
							decoded->function = instructions::SUBWF;
							decoded->param1 = (void*)(code & 0x007F);
							decoded->param2 = (void*)((code >> 7) & 0x0001);
							return true;
						}
					}
					else { //00 010X XXXX XXXX
						if (code & 0x0100) { //00 0001 XXXX XXXX
							if (code & 0x0080) { //00 0001 1XXX XXXX
								decoded->function = instructions::CLRF;
								decoded->param1 = (void*)(code & 0x007F);
								decoded->param2 = nullptr;
								return true;
							}
							else { //00 0001 0XXX XXXX
								decoded->function = instructions::CLRW;
								decoded->param1 = (void*)(code & 0x007F);
								decoded->param2 = nullptr;
								return true;
							}
						}
						else { //00 0000 XXXX XXXX
							if (code & 0x0080) { //00 0001 1XXX XXXX
								decoded->function = instructions::MOVWF;
								decoded->param1 = (void*)(code & 0x007F);
								decoded->param2 = nullptr;
								return true;
							}
							else { //00 0000 0XXX XXXX
								if (code & 0x0040) { //00 0000 01XX XXXX
									if (code & 0x0020) { //00 0000 011X XXXX
										if (code & 0x0010) { //00 0000 0111 XXXX
											decoded->function = instructions::NOP;
											decoded->param1 = nullptr;
											decoded->param2 = nullptr;
											return false;
										}
										else { //00 0000 0110 XXXX
											if (code & 0x0008) { //00 0000 0110 1XXX
												decoded->function = instructions::NOP;
												decoded->param1 = nullptr;
												decoded->param2 = nullptr;
												return false;
											}
											else { //00 0000 0110 0XXX
												if (code & 0x0004) { //00 0000 0110 01XX
													if (code & 0x0002) { //00 0000 0110 011X
														decoded->function = instructions::NOP;
														decoded->param1 = nullptr;
														decoded->param2 = nullptr;
														return false;
													}
													else { //00 0000 0110 010X
														if (code & 0x0001) { //00 0000 0110 0101
															decoded->function = instructions::NOP;
															decoded->param1 = nullptr;
															decoded->param2 = nullptr;
															return false;
														}
														else { //00 0000 0110 0100
															decoded->function = instructions::CLRWDT;
															decoded->param1 = nullptr;
															decoded->param2 = nullptr;
															return true;
														}
													}
												}
												else { //00 0000 0110 00XX
													if (code & 0x0002) { //00 0000 0110 001X
														if (code & 0x0001) { //00 0000 0110 0011
															decoded->function = instructions::SLEEP;
															decoded->param1 = nullptr;
															decoded->param2 = nullptr;
															return true;
														}
														else { //00 0000 0110 0010
															decoded->function = instructions::NOP;
															decoded->param1 = nullptr;
															decoded->param2 = nullptr;
															return false;
														}
													}
													else { //00 0000 0110 010X
														if (code & 0x0002) { //00 0000 0110 0001
															decoded->function = instructions::NOP;
															decoded->param1 = nullptr;
															decoded->param2 = nullptr;
															return false;
														}
														else { //00 0000 0110 0000
															decoded->function = instructions::NOP;
															decoded->param1 = nullptr;
															decoded->param2 = nullptr;
															return true;
														}
													}
												}
											}
										}
									}
									else { //00 0000 010X XXXX
										if (code & 0x0010) { //00 0000 0101 XXXX
											decoded->function = instructions::NOP;
											decoded->param1 = nullptr;
											decoded->param2 = nullptr;
											return false;
										}
										else { //00 0000 0100 XXXX
											if (code & 0x000F) { //00 0000 0100 XXXX
												decoded->function = instructions::NOP;
												decoded->param1 = nullptr;
												decoded->param2 = nullptr;
												return false;
											}
											else { //00 0000 0100 0000
												decoded->function = instructions::NOP;
												decoded->param1 = nullptr;
												decoded->param2 = nullptr;
												return true;
											}
										}
									}
								}
								else { //00 0000 00XX XXXX
									if (code & 0x0020) { //00 0000 001X XXXX
										if (code & 0x0010) { //00 0000 0011 XXXX
											decoded->function = instructions::NOP;
											decoded->param1 = nullptr;
											decoded->param2 = nullptr;
											return false;
										}
										else { //00 0000 0010 XXXX
											if (code & 0x000F) { //00 0000 0010 XXXX
												decoded->function = instructions::NOP;
												decoded->param1 = nullptr;
												decoded->param2 = nullptr;
												return false;
											}
											else { //00 0000 0010 0000
												decoded->function = instructions::NOP;
												decoded->param1 = nullptr;
												decoded->param2 = nullptr;
												return true;
											}
										}
									}
									else { //00 0000 000X XXXX
										if (code & 0x0010) { //00 0000 0001 XXXX
											decoded->function = instructions::NOP;
											decoded->param1 = nullptr;
											decoded->param2 = nullptr;
											return false;
										}
										else { //00 0000 0000 XXXX
											if (code & 0x0008) { //00 0000 0000 1XXX
												if (code & 0x0006) {
													decoded->function = instructions::NOP;
													decoded->param1 = nullptr;
													decoded->param2 = nullptr;
													return false;
												}
												else { //00 0000 0000 100X
													if (code & 0x0001) { //00 0000 0000 1001
														decoded->function = instructions::RETFIE;
														decoded->param1 = nullptr;
														decoded->param2 = nullptr;
														return true;

													}
													else { //00 0000 0000 1000
														decoded->function = instructions::RETURN;
														decoded->param1 = nullptr;
														decoded->param2 = nullptr;
														return true;
													}
												}
											}
											else { //00 0000 0000 0XXX
												if (code & 0x0007) {
													decoded->function = instructions::NOP;
													decoded->param1 = nullptr;
													decoded->param2 = nullptr;
													return false;
												}
												else { // 00 0000 0000 0000
													decoded->function = instructions::NOP;
													decoded->param1 = nullptr;
													decoded->param2 = nullptr;
													return true;
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
}

//if false, donot forget to free the rest String!
bool appendToString(scannerstring** string, scannerstring** aktPosInString, char sign, int& len) {
	scannerstring* append = (scannerstring*)malloc(sizeof(scannerstring));
	if (append == nullptr) {
		__compiler__lastError = "Missing Memmory.";
		return false;
	}
	append->sign = sign;
	append->next = nullptr;
	if (*string == nullptr) { DOIF(DEBUGLVL >= DEBUGLVL_MUCH)PRINTF("NEW STRING\n"); *string = append; len = 1; }
	else if (aktPosInString == nullptr) {
		free(append);
		__compiler__lastError = "Error in Calling Function 'appendToString'.";
		return false;
	}
	else { DOIF(DEBUGLVL >= DEBUGLVL_MUCH)PRINTF("APPENDING TO STRING\n"); (*aktPosInString)->next = append; len++;}
	*aktPosInString = append;
	return true;

}

//after call always set String to nullptr!
char* scannerString2PChar(scannerstring* string, int len) {
	char *retString = (char*)malloc((len + 1) * sizeof(char));
	if (retString == nullptr) {
		__compiler__lastError = MEMORY_MISSING;
		freeScannerString(string);
		return nullptr;
	}
	char *tmpRet = retString;
	scannerstring *tmpString;
	while (string != nullptr) {
		*tmpRet = string->sign;
		tmpRet++;
		tmpString = string->next;
		free(string);
		string = tmpString;
	}
	*tmpRet = 0;
	return retString;
}
//after calling always set toFree to nullptr, if will be used later
void freeScannerString(scannerstring* toFree) {
	scannerstring* tmp;
	while (toFree != nullptr) {
		tmp = toFree->next;
		free(toFree);
		toFree = tmp;
	}
}
char getNextChar(FILE* file) {
	if (bytesReaded < 0 || bytesReaded <= aktPufferPosition) {
		bytesReaded = fread(puffer, sizeof(char), pufferSize, file);
		aktPufferPosition = 0;
		DOIF(DEBUGLVL >= DEBUGLVL_NORMAL)PRINTF2("\t\t\treaded %d bytes (puffer size: %d)....\n\n", bytesReaded, pufferSize);
	}
	if (bytesReaded == 0) {
		fileReadedTilEnd = true;
		return 0;
	}
	char ret = puffer[aktPufferPosition];
	aktPufferPosition++;
	return ret;
}

//after calling always set toFree to nullptr, if will be used later
void freeASM(ASM* toFree) {
	free(toFree->code);
	ASM_TEXT* txt = toFree->text, *tmp;
	while (txt != nullptr) {
		if (txt->bytecode != nullptr)free(txt->bytecode);
		if (txt->lineOfCode != nullptr)free(txt->lineOfCode);
		if (txt->label != nullptr)free(txt->label);
		if (txt->asmCode != nullptr)free(txt->asmCode);
		if (txt->comment != nullptr)free(txt->comment);
		tmp = txt->next;
		free(txt);
		txt = tmp;
	}
	free(toFree);
}

char * getCompilerError()
{
	return __compiler__lastError;
}

char* functionPointerToName(instruction_t f) {
	if(f == instructions::ADDWF)		return "ADDWF";
	else if(f == instructions::ANDWF)	return "ANDWF";
	else if(f == instructions::CLRF)	return "CLRF";
	else if(f == instructions::CLRW)	return "CLRW";
	else if(f == instructions::COMF)	return "COMF";
	else if(f == instructions::DECF)	return "DECF";
	else if(f == instructions::DECFSZ)	return "DECFSZ";
	else if(f == instructions::INCF)	return "INCF";
	else if(f == instructions::INCFSZ)	return "INCFSZ";
	else if(f == instructions::IORWF)	return "IORWF";
	else if(f == instructions::MOVF)	return "MOVF";
	else if(f == instructions::MOVWF)	return "MOVWF";
	else if(f == instructions::NOP)		return "NOP";
	else if(f == instructions::RLF)		return "RLF";
	else if(f == instructions::RRF)		return "RRF";
	else if(f == instructions::SUBWF)	return "SUBWF";
	else if(f == instructions::SUBWF)	return "SUBWF";
	else if(f == instructions::XORWF)	return "XORWF";
	else if(f == instructions::BCF)		return "BCF";
	else if(f == instructions::BSF)		return "BSF";
	else if(f == instructions::BTFSC)	return "BTFSC";
	else if(f == instructions::BTFSS)	return "BTFSS";
	else if(f == instructions::ADDLW)	return "ADDLW";
	else if(f == instructions::ANDLW)	return "ANDLW";
	else if(f == instructions::CALL)	return "CALL";
	else if(f == instructions::CLRWDT)	return "CLRWDT";
	else if(f == instructions::GOTO)	return "GOTO";
	else if(f == instructions::IORLW)	return "IORLW";
	else if(f == instructions::MOVLW)	return "MOVLW";
	else if(f == instructions::RETFIE)	return "RETFIE";
	else if(f == instructions::RETLW)	return "RETLW";
	else if(f == instructions::RETURN)	return "RETURN";
	else if(f == instructions::SLEEP)	return "SLEEP";
	else if(f == instructions::SUBLW)	return "SUBLW";
	else if(f == instructions::XORLW)	return "XORLW";
	else								return "Unknown";
}