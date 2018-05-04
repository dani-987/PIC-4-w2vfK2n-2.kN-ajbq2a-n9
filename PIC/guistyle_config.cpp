#include "guistyle_config.h"


void filltxt(char*& txt, char* tofill) {
	if (tofill != nullptr) {
		txt = (char*)malloc(strlen(tofill) + 1);
		sprintf(txt, tofill);
	}
	else {
		txt = (char*)malloc(1);
		sprintf(txt, "");
	}
}

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
	char* txt;
	tablestyle* s = (tablestyle*)malloc(sizeof(tablestyle) * CellsMEM);
	for (int i = 0; i < CellsMEM; i++) {
		if (!i) { s[i] = MEM_Startpage; txt = nullptr; }
		//the first few cases have unchanging labels that are set here and never change (headers and unimplemented cells)
		else if (i < 9) { 
			s[i] = MEM_Colheaders; 
			txt = (char*)malloc(3);
			sprintf(txt, "%02d", i - 1);			
		}
		else if (!(i % 9)) {
			s[i] = MEM_Rowheaders;
			txt = (char*)malloc(3);
			if (i < 45) sprintf(txt, "%d%d", (i<27)?0:8, (i % 2) ? 0 : 8);
			else sprintf(txt, "%d%d", ((i-36)/9)/2, (i%2)?8:0);
		}
		else if (i == 17 || (i > 22 && i < 27) || i == 35 || (i > 40 && i < 50) || i > 99) {
			s[i] = MEM_Uninstalled;
			filltxt(txt, "00");
		}
		//Anything below here is initilaized with "00" and has it's label updated on change of the respective Memory Adress
		else if (i < 23) {
			s[i] = MEM_Reserved_B0;
			filltxt(txt, "00");
		}
		else if (i < 41) {
			s[i] = MEM_Reserved_B1;
			filltxt(txt, "00");
		}
		else {
			s[i] = MEM_Available;
			filltxt(txt, "00");
		}
		s[i].label = txt;
	}
	return s;
}


tablestyle IO_Rowheaders = { nullptr, FL_HELVETICA, FONT_SIZE_TABLE, FL_NO_BOX, FL_BLACK, FL_BLACK, FL_LIGHT2, FL_ALIGN_CENTER },
	IO_DigitEnum = { nullptr, FL_HELVETICA, FONT_SIZE_TABLE, FL_NO_BOX, FL_BLACK, FL_BLACK, FL_WHITE, FL_ALIGN_CENTER },
	IO_TrisValues = { nullptr, FL_HELVETICA, FONT_SIZE_TABLE, FL_NO_BOX, FL_BLACK, FL_BLACK, FL_WHITE, FL_ALIGN_CENTER },
	IO_Values = { nullptr, FL_HELVETICA, FONT_SIZE_TABLE, FL_NO_BOX, FL_BLACK, FL_BLACK, FL_WHITE, FL_ALIGN_CENTER };

//TODO, aber nicht notwendig: alles auf filltxt umstellen
tablestyle * setstyle_IO()
{
	tablestyle* s = (tablestyle*)malloc(sizeof(tablestyle) * CellsIO);
	for (int i = 0; i < CellsIO; i++) {
		char* txt = "";			//Initializing here because compiler will otherwise cry about use of uninitilized variable 
		if (!(i % 9)) {
			s[i] = IO_Rowheaders;
			switch (i / 9) {
			case 0: filltxt(txt, "RA"); break;
			case 1: case 4: filltxt(txt, "Tris"); break;
			case 2: case 5: filltxt(txt, "Pin"); break;
			case 3: filltxt(txt, "RB"); break;
			}
		}
		else if ((i % 27) > 0 && (i % 27) < 9) {
			s[i] = IO_DigitEnum;
			txt = (char*)malloc(2);
			sprintf(txt, "%d", 8 - (i % 9));
		}
		else if ((i % 27) > 9 && (i % 27) < 18) {
			s[i] = IO_TrisValues;
			filltxt(txt, "i");
		}
		else {
			s[i] = IO_Values;
			filltxt(txt, "i");
		}
		s[i].label = txt;
	}
	return s;
}

tablestyle CODE_Colheaders = { nullptr, FL_HELVETICA, FONT_SIZE_TABLE, FL_NO_BOX, FL_BLACK, FL_BLACK, FL_DARK_YELLOW, FL_ALIGN_CENTER },
CODE_TEXT = { nullptr, FL_HELVETICA, FONT_SIZE_TABLE, FL_NO_BOX, FL_BLACK, FL_BLACK, FL_BLUE, FL_ALIGN_LEFT };

tablestyle * setstyle_Code(int lines, ASM_TEXT* code) {
	int CellsCode = (lines + 1) * CCCODE;
	tablestyle* s = (tablestyle*)malloc(sizeof(tablestyle) * CellsCode);
	for (int i = 0; i < CCCODE; i++) {
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
	if (code != nullptr) {
		//This loop imports all the strings from the loaded file into the tablestyle-array
		for (int i = CCCODE; i < CellsCode; i++) {
			char* txt;
			s[i] = CODE_TEXT;
			switch (i % CCCODE) {
			case 0: filltxt(txt, code->bytecode); break;
			case 1: filltxt(txt, code->lineOfCode); break;
			case 2: filltxt(txt, code->label); break;
			case 3: filltxt(txt, code->asmCode); break;
			case 4: filltxt(txt, code->comment); code = code->next; break;
			default: txt = (char*)malloc(1); sprintf(txt, ""); break;//default path will never be used, but compiler wants it anyway because of the initialization of txt
			}
			s[i].label = txt;
		}
	}
	else {
		//This loop is for the intial table when no file is loaded, so all labels are empty
		for (int i = CCCODE; i < CellsCode; i++) {
			char* txt = (char*)malloc(1);
			s[i] = CODE_TEXT;
			sprintf(txt, "");
			s[i].label = txt;
		}
	}
	return s;
}
	

void freetablestyle(tablestyle*& tofree, int cells){
	for (int i = 0; i < cells; i++) {
		free(tofree[i].label);
	}
	free(tofree);
}


void setregbox(Fl_Box*& regs, int line, int value) {
	char* txt=(char*)malloc(50);
	
	switch (line) {
	case 0: {
		sprintf(txt, "W-Reg:\t\t%02X", value);
		break;
	}
	case 1: {
		sprintf(txt, "Status:\t\t%02X", value);
		break;
	}
	case 2: {
		int i = value;
		sprintf(txt, "IRP\tRP1\tRP0\tTO\tPD\tZ\tDC\tC\n%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t", i&0x128 ? 1 : 0, i & 0x64 ? 1 : 0, i & 0x32 ? 1 : 0, i & 0x16 ? 1 : 0, i & 0x8 ? 1 : 0, i & 0x4 ? 1 : 0, i & 0x2 ? 1 : 0, i & 0x1 ? 1 : 0);
		break;
	}
	case 3: {
		sprintf(txt, "PCL:\t\t%02X", value);
		break;
	}
	case 4: {
		sprintf(txt, "PCLATH:\t%02X", value);
		break;
	}
	case 5: {
		sprintf(txt, "PC:\t\t%04X", value);
		break;
	}
	case 6: {
		sprintf(txt, "Option:\t%02X", value);
		break;
	}
	case 7: {
		int i = value;
		sprintf(txt, "RBPU\tINTEDG\tT0CS\tT0SE\tPSA\tPS2\tPS1\tPS0\n%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t", i & 0x128 ? 1 : 0, i & 0x64 ? 1 : 0, i & 0x32 ? 1 : 0, i & 0x16 ? 1 : 0, i & 0x8 ? 1 : 0, i & 0x4 ? 1 : 0, i & 0x2 ? 1 : 0, i & 0x1 ? 1 : 0);
		break;
	}
	case 8: {
		sprintf(txt, "INTCON:\t%02X", value);
		break;
	}
	case 9: {
		int i = value;
		sprintf(txt, "GIE\tEEIE\tT0IE\tINTE\tRBIE\tT0IF\tINTF\tRBIF\n%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t", i & 0x128 ? 1 : 0, i & 0x64 ? 1 : 0, i & 0x32 ? 1 : 0, i & 0x16 ? 1 : 0, i & 0x8 ? 1 : 0, i & 0x4 ? 1 : 0, i & 0x2 ? 1 : 0, i & 0x1 ? 1 : 0);
		break;
	}
			regs->label(txt);
	}
}

//Update a label of a single cell in the MEM-table
void setMEMcell(tablestyle*& mystyle, int pos, int bank, int value) {
	//Translate pos and bank into position in the tablestyle-array
	if (bank) {			//For the non-mirrored special registers on bank 1:
		pos = 28 + pos + (int)(pos / 8);
	}
	else if (pos < 12) {//For the special registers on bank 0
		pos = 10 + pos + (int)(pos / 8);
	}
	else {				//For the normal registers
		pos -= 8;
		pos = 46 + pos + (int)(pos / 8);
	}
	sprintf(mystyle[pos].label, "%02X", value);
}

//Update one line in the IO-table, happens whenever RA, RB, TrisA or TrisB are changed
void setIOcell(tablestyle*& mystyle, int line, int value) {
	char high = (line % 3 == 1) ? 'i' : '1', low = (line % 3 == 1) ? 'o' : '0';
	int pos = (CCIO + 1) * line + 1;
	for (int i = 0; i < CCIO; i++) {

		sprintf(mystyle[pos + i].label, "%c", value&(1<<i)?high:low);
	}
}