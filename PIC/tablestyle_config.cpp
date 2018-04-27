#include "tablestyle_config.h"

//Tablestyles for the Memory Table
tablestyle MEM_Startpage = {nullptr, FL_HELVETICA, FONT_SIZE_TABLE, FL_NO_BOX, FL_BLACK, FL_BLACK, FL_LIGHT2, FL_ALIGN_CENTER},
	MEM_Rowheaders = { nullptr, FL_HELVETICA, FONT_SIZE_TABLE, FL_NO_BOX, FL_BLACK, FL_BLACK, FL_LIGHT2, FL_ALIGN_CENTER },
	MEM_Colheaders = { nullptr, FL_HELVETICA, FONT_SIZE_TABLE, FL_NO_BOX, FL_BLACK, FL_BLACK, FL_LIGHT2, FL_ALIGN_CENTER },
	MEM_Reserved_B0 = { nullptr, FL_HELVETICA, FONT_SIZE_TABLE, FL_NO_BOX, FL_BLACK, FL_BLACK, FL_BLUE, FL_ALIGN_CENTER },
	MEM_Reserved_B1 = { nullptr, FL_HELVETICA, FONT_SIZE_TABLE, FL_NO_BOX, FL_BLACK, FL_BLACK, FL_GREEN, FL_ALIGN_CENTER },
	MEM_Available = { nullptr, FL_HELVETICA, FONT_SIZE_TABLE, FL_NO_BOX, FL_BLACK, FL_BLACK, FL_WHITE, FL_ALIGN_CENTER },
	MEM_Uninstalled = { nullptr, FL_HELVETICA, FONT_SIZE_TABLE, FL_NO_BOX, FL_BLACK, FL_BLACK, FL_LIGHT1, FL_ALIGN_CENTER };

tablestyle * setstyle_MEM()
{
	tablestyle* s = (tablestyle*)malloc(sizeof(tablestyle) * CellsMEM);
	for (int i = 0; i < CellsMEM; i++) {
		if (!i) { s[i] = MEM_Startpage;}
		//the first few types have unchanging labels that are set here and never change
		else if (i < 9) { 
			s[i] = MEM_Colheaders; 
			char *txt = (char*)malloc(3);
			sprintf(txt, "%02d", i - 1);
			s[i].label = txt;
		}
		else if (!(i % 9)) {
			s[i] = MEM_Rowheaders;
			char *txt = (char*)malloc(3);

			if (i < 45) sprintf(txt, "%d%d", (i<27)?0:8, (i % 2) ? 0 : 8);
			else sprintf(txt, "%d%d", ((i-36)/9)/2, (i%2)?8:0);
			s[i].label = txt;
		}
		else if (i == 17 || (i > 21 && i < 27) || i == 35 || (i > 39 && i < 49) || i > 99) {
			s[i] = MEM_Uninstalled;
			s[i].label = "00";
		}
		//Anything below here is initilaized with "00" and has it's label updated on change of the respective Memory Adress
		else if (i < 22) {
			s[i] = MEM_Reserved_B0;
			s[i].label = "00";
		}
		else if (i < 40) {
			s[i] = MEM_Reserved_B1;
			s[i].label = "00";
		}
		else {
			s[i] = MEM_Available;
			s[i].label = "00";
		}
	}
	return s;
}


tablestyle IO_Rowheaders = { nullptr, FL_HELVETICA, FONT_SIZE_TABLE, FL_NO_BOX, FL_BLACK, FL_BLACK, FL_LIGHT2, FL_ALIGN_CENTER },
	IO_DigitEnum = { nullptr, FL_HELVETICA, FONT_SIZE_TABLE, FL_NO_BOX, FL_BLACK, FL_BLACK, FL_WHITE, FL_ALIGN_CENTER },
	IO_TrisValues = { nullptr, FL_HELVETICA, FONT_SIZE_TABLE, FL_NO_BOX, FL_BLACK, FL_BLACK, FL_WHITE, FL_ALIGN_CENTER },
	IO_Values = { nullptr, FL_HELVETICA, FONT_SIZE_TABLE, FL_NO_BOX, FL_BLACK, FL_BLACK, FL_WHITE, FL_ALIGN_CENTER };

tablestyle * setstyle_IO()
{
	tablestyle* s = (tablestyle*)malloc(sizeof(tablestyle) * CellsIO);
	for (int i = 0; i < CellsIO; i++) {
		if (!(i % 9)) {
			s[i] = IO_Rowheaders;
			char *txt = (char*)malloc(5);
			switch (i / 9) {
			case 0: sprintf(txt, "RA"); break;
			case 1: case 4: sprintf(txt, "Tris"); break;
			case 2: case 5: sprintf(txt, "Pin"); break;
			case 3: sprintf(txt, "RB"); break;
			}
			s[i].label = txt;
		}
		else if ((i % 27) > 0 && (i % 27) < 9) {
			s[i] = IO_DigitEnum;
			char *txt = (char*)malloc(10);
			sprintf(txt, "%d", 8 - (i % 9));
		}
		else if ((i % 27) > 9 && (i % 27) < 18) {
			s[i] = IO_TrisValues;
			s[i].label = "i";
		}
		else {
			s[i] = IO_Values;
			s[i].label = "0";
		}
	}
	return s;
}

tablestyle CODE_Colheaders = { nullptr, FL_HELVETICA, FONT_SIZE_TABLE, FL_NO_BOX, FL_BLACK, FL_BLACK, FL_DARK_YELLOW, FL_ALIGN_CENTER },
CODE_TEXT = { nullptr, FL_HELVETICA, FONT_SIZE_TABLE, FL_NO_BOX, FL_BLACK, FL_BLACK, FL_BLUE, FL_ALIGN_LEFT };

void filltxt(char*& txt, char* tofill) {
	txt = (char*)malloc(strlen(tofill) + 1);
	sprintf(txt, tofill);
}

tablestyle * setstyle_Code(int lines, ASM_TEXT* code){
	int CellsCode = (lines + 1) * CCCODE;
	tablestyle* s = (tablestyle*)malloc(sizeof(tablestyle) * CellsCode);
	if (code != nullptr) {
		for (int i = 0; i < CellsCode; i++) {
			if (i < CCCODE) {
				s[i] = CODE_Colheaders;
				char *txt = (char*)malloc(15);
				switch (i) {
				case 0: sprintf(txt, "Bytecode"); break;
				case 1: sprintf(txt, "Zeilen"); break;
				case 2: sprintf(txt, "Labels"); break;
				case 3: sprintf(txt, "Befehle"); break;
				case 4: sprintf(txt, "Kommentare"); break;
				}
				s[i].label = txt;
			}
			else {
				char* txt;
				s[i] = CODE_TEXT;
				switch (i % CCCODE) {
				case 0: if (code->bytecode != nullptr) { filltxt(txt, code->bytecode); } else { filltxt(txt, ""); } break;
				case 1: if (code->lineOfCode != nullptr) { filltxt(txt, code->lineOfCode); } else { filltxt(txt, ""); }  break;
				case 2: if (code->label != nullptr) { filltxt(txt, code->label); } else { filltxt(txt, ""); }  break;
				case 3: if (code->asmCode != nullptr) { filltxt(txt, code->asmCode); } else { filltxt(txt, ""); }  break;
				case 4: if (code->comment != nullptr) { filltxt(txt, code->comment); } else { filltxt(txt, ""); } code = code->next; break;
				//case 0: txt = (char*)malloc(strlen(code->bytecode) + 1); sprintf(txt, code->bytecode); break;
				//case 1: txt = (char*)malloc(strlen(code->lineOfCode) + 1); sprintf(txt, code->lineOfCode); break;
				//case 2: txt = (char*)malloc(strlen(code->label) + 1); sprintf(txt, code->label); break;
				//case 3: txt = (char*)malloc(strlen(code->asmCode) + 1); sprintf(txt, code->asmCode); break;
				//case 4: txt = (char*)malloc(strlen(code->comment) + 1); sprintf(txt, code->comment); code = code->next; break;
				default: txt = (char*)malloc(1); sprintf(txt, ""); break;//default path will never be used, but compiler wants it anyway because of the initialization of txt
				}
				s[i].label = txt;
			}
		}
	}
	else {
		for (int i = 0; i < CellsCode; i++) {
			if (i < CCCODE) {
				s[i] = CODE_Colheaders;
				char *txt = (char*)malloc(15);
				switch (i) {
				case 0: sprintf(txt, "Bytecode"); break;
				case 1: sprintf(txt, "Zeilen"); break;
				case 2: sprintf(txt, "Labels"); break;
				case 3: sprintf(txt, "Befehle"); break;
				case 4: sprintf(txt, "Kommentare"); break;
				}
				s[i].label = txt;
			}
			else {
				char* txt=(char*)malloc(1);
				s[i] = CODE_TEXT;
				sprintf(txt, "");
				s[i].label = txt;
			}
		}
	}
	return s;
}