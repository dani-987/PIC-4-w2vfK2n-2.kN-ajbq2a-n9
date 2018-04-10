#include "Compiler.h"
#include <fstream>
#include <Windows.h>
#include <AtlConv.h>
#include <atlbase.h>

#define CANNOT_OPEN_FILE	"Kann Datei nicht öffnen."
#define MEMORY_MISSING		"Es fehlt an Arbeitsspeicher."

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


char* lastError = "Kein Fehler aufgetreten!";

char* puffer;
int bytesReaded, aktPufferPosition;
DWORD pufferSize = 0;
bool fileReadedTilEnd;

bool decodeLine(ASM_CODE** code, int* pos, ASM_TEXT** text, FILE* file);
bool decodeInstruction(int code, ASM_CODE* decoded);
bool appendToString(scannerstring** string, scannerstring** aktPosInString, char sign, int& len);
char* scannerString2PChar(scannerstring* string, int len);
void freeScannerString(scannerstring* toFree);
char getNextChar(FILE* file);

ASM * compileFile(char * file, int memsize)
{
	FILE* f = fopen(file, "r");
	if (!f) { 
		lastError = CANNOT_OPEN_FILE;
		return nullptr;
	}
	if (pufferSize < 0 && (!GetDiskFreeSpace(CA2A(file), nullptr, &pufferSize, nullptr, nullptr) || pufferSize < 8)) {
		pufferSize = 64 * 1024;
	}
	fileReadedTilEnd = false;
	ASM_CODE* asmcode = (ASM_CODE*)malloc(sizeof(ASM_CODE)*memsize);
	if (asmcode == nullptr) {
		lastError = MEMORY_MISSING;
		return nullptr;
	}
	ASM_TEXT* aktLine = nullptr, *startLine = nullptr;
	ASM* retASM = (ASM*)malloc(sizeof(ASM));
	int aktPosInCode = 0;
	if (retASM == nullptr) {
		free(asmcode);
		lastError = MEMORY_MISSING;
		return nullptr;
	}
	retASM->code = asmcode;
	bytesReaded = -1;
	while (!fileReadedTilEnd) {
		if (!decodeLine(&asmcode, &aktPosInCode, &aktLine, f)) {
			goto ERROR_END;
		}
		else if(startLine == nullptr){
			startLine = aktLine;
		}
		if ((int)(asmcode - retASM->code) > memsize) {
			lastError = "Programm to long.";
			goto ERROR_END;
		}
	}
	retASM->text = startLine;
	fclose(f);
	return retASM;
ERROR_END:
	retASM->text = startLine;
	freeASM(retASM);
	fclose(f);
	return nullptr;
}

bool decodeLine(ASM_CODE** code, int* pos, ASM_TEXT** text, FILE* file) {
	char sign, status = 0;
	ASM_CODE* newCode = nullptr;
	ASM_TEXT* newText = nullptr;
	scannerstring* string = nullptr, *aktPosInString;
	int codeVal = 0, stringLen;
	while (1) {
		sign = getNextChar(file);
		if (fileReadedTilEnd || sign == '\n') {
			if (status == STATUS_READING_CODE1 || 
				status == STATUS_READING_BEFORE_CODE2 ||
				status == STATUS_READING_CODE2) {
				if(fileReadedTilEnd)
					lastError = "Unerwartetes Dateiende während des Lesens des Maschinencodes.";
				else lastError = "Unerwartetes Zeilenende während des Lesens des Maschinencodes.";
				goto ERROR_END; 
			}
			else if(newCode != nullptr){
				if (status == STATUS_READING_BEFORE_LINE ||
					status == STATUS_READING_BEFORE_LABEL1 ||
					status == STATUS_READING_BEFORE_LABEL2 ||
					status == STATUS_READING_LABEL ||
					status == STATUS_READING_BEFORE_ASM) {
					if (fileReadedTilEnd)
						lastError = "Unerwartetes Dateiende, der Assemblercode fehlt.";
					else lastError = "Unerwartetes Zeilenende, der Assemblercode fehlt.";
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
			case '0': case '1': case '3': case '4': case '5':
			case '6': case '7': case '8': case '9':
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
				lastError = "Unexpected Sign! Expected Space, Tab, 0-9, a-f, A-F at Linebegin.";
				goto ERROR_END;
			}
			break;
		case STATUS_READING_CODE1:
			switch (sign) {
			case ' ': case '\t':
				status = STATUS_READING_BEFORE_CODE2;
				if (!appendToString(&string, &aktPosInString, ' ', stringLen)) { goto ERROR_END; }
				break;
			case '0': case '1': case '3': case '4': case '5':
			case '6': case '7': case '8': case '9':
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
				lastError = "Unexpected Sign! Expected Space, Tab, 0-9, a-f, A-F while reading machine code.";
				goto ERROR_END;
			}
			break;
		case STATUS_READING_BEFORE_CODE2:
			switch (sign) {
			case ' ': case '\t':
				break;
			case '0': case '1': case '3': case '4': case '5':
			case '6': case '7': case '8': case '9':
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
				lastError = "Unexpected Sign! Expected Space, Tab, 0-9, a-f, A-F while reading machine code.";
				goto ERROR_END;
			}
			break;
		case STATUS_READING_CODE2:
			switch (sign) {
			case ' ': case '\t':
				status = STATUS_READING_BEFORE_LINE;
				newText = (ASM_TEXT*)malloc(sizeof(ASM_TEXT));
				if (newText == nullptr) {
					lastError = MEMORY_MISSING;
					freeScannerString(string);
					goto ERROR_END;
				}
				newText->lineOfCode = nullptr;
				newText->label = nullptr;
				newText->asmCode = nullptr;
				newText->comment = nullptr;
				newCode = (ASM_CODE*)malloc(sizeof(ASM_CODE));
				if (newText == nullptr) {
					lastError = MEMORY_MISSING;
					free(newText);
					freeScannerString(string);
					goto ERROR_END;
				}
				newText->bytecode = scannerString2PChar(string, stringLen);
				string = nullptr;
				if (newText->bytecode == nullptr) {
					free(newText);
					free(newCode);
					freeScannerString(string);
					goto ERROR_END;
				}
				newCode->guiText = newText;
				//set functionpointers and params
				if (!decodeInstruction(codeVal, newCode)) {
					free(newText->bytecode);
					free(newText);
					free(newCode);
					freeScannerString(string);
					goto ERROR_END;
				}
				break;
			case '0': case '1': case '3': case '4': case '5':
			case '6': case '7': case '8': case '9':
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
				lastError = "Unexpected Sign! Expected Space, Tab, 0-9, a-f, A-F while reading machine code.";
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
				lastError = "Unexpected Sign! Expected Space, Tab, 0-9 for Linenumber!";
				goto ERROR_END;
			}
			break;
		case STATUS_READING_LINE:
			if (sign == ' ' || sign == '\t') {
				if (newText != nullptr) {
					newText->lineOfCode = scannerString2PChar(string, stringLen);
					if (newText->lineOfCode == nullptr) {
						free(newText->bytecode);
						free(newText);
						free(newCode);
						freeScannerString(string);
						goto ERROR_END;
					}
				}
				else {
					newText = (ASM_TEXT*)malloc(sizeof(ASM_TEXT));
					if (newText == nullptr) {
						lastError = MEMORY_MISSING;
						goto ERROR_END;
					}
					newText->bytecode = nullptr;
					newText->lineOfCode = scannerString2PChar(string, stringLen);
					newText->label = nullptr;
					newText->asmCode = nullptr;
					newText->comment = nullptr;
				}
				status = STATUS_READING_BEFORE_LABEL1;
			}
			else if (sign >= '0' && sign <= '9') {
				if (!appendToString(&string, &aktPosInString, sign, stringLen)) { goto ERROR_END; }
			}
			else {
				lastError = "Unexpected Sign! Expected Space, Tab, 0-9 for Linenumber!";
				goto ERROR_END;
			}
			break;
		case STATUS_READING_BEFORE_LABEL1:
			if (sign == ' ' || sign == '\t');
			else if ((sign >= 'a' && sign <= 'z') ||
				(sign >= 'A' && sign <= 'Z') ||
				(sign >= '0' && sign <= '9')) {
				status = STATUS_READING_LABEL;
				if (!appendToString(&string, &aktPosInString, sign, stringLen)){goto ERROR_END;}
			}
			else {
				lastError = "Unexpected Sign! Expected Space, Tab, a-z, A-Z, 0-9 for Label!";
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
				lastError = "Unexpected Sign! Expected Space, Tab, a-z, A-Z, 0-9 for Label!";
				goto ERROR_END;
			}
			break;
		case STATUS_READING_LABEL:
			break;
		case STATUS_READING_BEFORE_ASM:
			switch (sign) {
			case ' ': case '\t':
				break;
			case ';':
				if (newCode != nullptr) {
					lastError = "Expected assembly-code. Commend found!";
					goto ERROR_END;
				}
				status = STATUS_READING_KOMMENT;
			}
			break;
		default:
			status = STATUS_READING_ASM;
			if (!appendToString(&string, &aktPosInString, sign, stringLen)) { goto ERROR_END; }
			break;
		case STATUS_READING_ASM:
			if (sign == ';') {
				newText->asmCode = scannerString2PChar(string, stringLen);
				if (newText->asmCode == nullptr) {
					if (newCode != nullptr)free(newCode);
					if (newText->bytecode != nullptr)free(newText->bytecode);
					if (newText->lineOfCode != nullptr)free(newText->lineOfCode);
					if (newText->label != nullptr)free(newText->label);
					free(newText);
					freeScannerString(string);
					goto ERROR_END;
				}
			}
			else {
				if (!appendToString(&string, &aktPosInString, sign, stringLen)) { 
					freeScannerString(string);
					goto ERROR_END;
				}
			}
			break;
		case STATUS_READING_KOMMENT:
			break;
		}
	}
ERRORLESS_END:
	if (newCode != nullptr) {
		*code = newCode;
		code++;
	}
	if (newText != nullptr) {
		if (*text != nullptr) {
			(*text)->next = newText;
			*text = newText;
			*pos++;
		}
		else {
			*text = newText;
			*pos = 1;
		}
	}
	return true;
ERROR_END:
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

bool appendToString(scannerstring** string, scannerstring** aktPosInString, char sign, int& len) {
	scannerstring* append = (scannerstring*)malloc(sizeof(scannerstring));
	if (append == nullptr) {
		lastError = "Missing Memmory.";
		return false;
	}
	if (*string = nullptr) { *string = append; len = 1; }
	else if (aktPosInString == nullptr) {
		free(append);
		lastError = "Error in Calling Function 'appendToString'.";
		return false;
	}
	else {(*aktPosInString)->next = append; len++;}
	*aktPosInString = append;
	return true;

}
char* scannerString2PChar(scannerstring* string, int len) {
	char *retString = (char*)malloc((len + 1) * sizeof(char));
	if (retString == nullptr) {
		lastError = MEMORY_MISSING;
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
	}
	if (bytesReaded == 0) {
		fileReadedTilEnd = true;
		return 0;
	}
	char ret = puffer[aktPufferPosition];
	aktPufferPosition++;
	return ret;
}

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
	return lastError;
}

char* functionPointerToName(void(*f)(void*, void*)) {
	switch ((int)f) {
	case (int)instructions::ADDWF: return "ADDWF";
	case (int)instructions::ANDWF: return "ANDWF";
	case (int)instructions::CLRF: return "CLRF";
	case (int)instructions::CLRW: return "CLRW";
	case (int)instructions::COMF: return "COMF";
	case (int)instructions::DECF: return "DECF";
	case (int)instructions::DECFSZ: return "DECFSZ";
	case (int)instructions::INCF: return "INCF";
	case (int)instructions::INCFSZ: return "INCFSZ";
	case (int)instructions::IORWF: return "IORWF";
	case (int)instructions::MOVF: return "MOVF";
	case (int)instructions::MOVWF: return "MOVWF";
	case (int)instructions::NOP: return "NOP";
	case (int)instructions::RLF: return "RLF";
	case (int)instructions::RRF: return "RRF";
	case (int)instructions::SUBWF: return "SUBWF";
	case (int)instructions::SUBWF: return "SUBWF";
	case (int)instructions::XORWF: return "XORWF";
	case (int)instructions::BCF: return "BCF";
	case (int)instructions::BSF: return "BSF";
	case (int)instructions::BTFSC: return "BTFSC";
	case (int)instructions::BTFSS: return "BTFSS";
	case (int)instructions::ADDLW: return "ADDLW";
	case (int)instructions::ANDLW: return "ANDLW";
	case (int)instructions::CALL: return "CALL";
	case (int)instructions::CLRWDT: return "CLRWDT";
	case (int)instructions::GOTO: return "GOTO";
	case (int)instructions::IORLW: return "IORLW";
	case (int)instructions::MOVLW: return "MOVLW";
	case (int)instructions::RETFIE: return "RETFIE";
	case (int)instructions::RETLW: return "RETLW";
	case (int)instructions::RETURN: return "RETURN";
	case (int)instructions::SLEEP: return "SLEEP";
	case (int)instructions::SUBLW: return "SUBLW";
	case (int)instructions::XORLW: return "XORLW";
		return "Unknown";
	}
}