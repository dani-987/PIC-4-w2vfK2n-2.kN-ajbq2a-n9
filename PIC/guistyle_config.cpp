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

void filltxt_code(char*& txt, char* tofill) {
	if (tofill != nullptr) {
		txt = (char*)malloc(strlen(tofill) + 1);
		sprintf(txt, " %s", tofill);
	}
	else {
		txt = (char*)malloc(1);
		sprintf(txt, "");
	}
}

//Tablestyles for the Memory Table
tablestyle MEM_Startpage = {nullptr, FL_HELVETICA, FONT_SIZE, FL_NO_BOX, FL_BLACK, FL_BLACK, FL_LIGHT2, FL_ALIGN_CENTER},
	MEM_Rowheaders = { nullptr, FL_HELVETICA, FONT_SIZE, FL_NO_BOX, FL_BLACK, FL_BLACK, FL_LIGHT2, FL_ALIGN_CENTER },
	MEM_Colheaders = { nullptr, FL_HELVETICA, FONT_SIZE, FL_NO_BOX, FL_BLACK, FL_BLACK, FL_LIGHT2, FL_ALIGN_CENTER },
	MEM_Reserved_B0 = { nullptr, FL_HELVETICA, FONT_SIZE, FL_NO_BOX, FL_BLACK, FL_BLACK, FL_BLUE, FL_ALIGN_CENTER },
	MEM_Reserved_B1 = { nullptr, FL_HELVETICA, FONT_SIZE, FL_NO_BOX, FL_BLACK, FL_BLACK, FL_GREEN, FL_ALIGN_CENTER },
	MEM_Available = { nullptr, FL_HELVETICA, FONT_SIZE, FL_NO_BOX, FL_BLACK, FL_BLACK, FL_WHITE, FL_ALIGN_CENTER },
	MEM_Uninstalled = { nullptr, FL_HELVETICA, FONT_SIZE, FL_NO_BOX, FL_BLACK, FL_BLACK, FL_LIGHT1, FL_ALIGN_CENTER };

tablestyle * setstyle_MEM()
{
	char* txt;
	tablestyle* s = (tablestyle*)malloc(sizeof(tablestyle) * CELL_COUNT_MEM);
	for (int i = 0; i < CELL_COUNT_MEM; i++) {
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
		else if (i == 17 || (i > 22 && i < 27) || i == 35 || (i > 40 && i < 50) || i > 126) {
			s[i] = MEM_Uninstalled;
			filltxt(txt, "");
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


tablestyle IO_Rowheaders = { nullptr, FL_HELVETICA, FONT_SIZE, FL_NO_BOX, FL_BLACK, FL_BLACK, FL_LIGHT2, FL_ALIGN_CENTER },
	IO_DigitEnum = { nullptr, FL_HELVETICA, FONT_SIZE, FL_NO_BOX, FL_BLACK, FL_BLACK, FL_WHITE, FL_ALIGN_CENTER },
	IO_TrisValues = { nullptr, FL_HELVETICA, FONT_SIZE, FL_NO_BOX, FL_BLACK, FL_BLACK, FL_WHITE, FL_ALIGN_CENTER },
	IO_Values = { nullptr, FL_HELVETICA, FONT_SIZE, FL_NO_BOX, FL_BLACK, FL_BLACK, FL_WHITE, FL_ALIGN_CENTER };

//TODO, aber nicht notwendig: alles auf filltxt umstellen
tablestyle * setstyle_IO()
{
	tablestyle* s = (tablestyle*)malloc(sizeof(tablestyle) * CELL_COUNT_IO);
	for (int i = 0; i < CELL_COUNT_IO; i++) {
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

tablestyle CODE_Colheaders = { nullptr, FL_HELVETICA, FONT_SIZE, FL_NO_BOX, FL_BLACK, FL_BLACK, FL_DARK_YELLOW, FL_ALIGN_CENTER },
CODE_TEXT = { nullptr, FL_HELVETICA, FONT_SIZE, FL_NO_BOX, FL_BLACK, FL_BLACK, FL_LIGHT1, FL_ALIGN_LEFT };

tablestyle * setstyle_Code(int lines, ASM_TEXT* code) {
	int CellsCode = (lines + 1) * CCCODE;
	tablestyle* s = (tablestyle*)malloc(sizeof(tablestyle) * CellsCode);
	for (int i = 0; i < CCCODE; i++) {
		s[i] = CODE_Colheaders;
		char *txt;
		switch (i) {
		case 0: filltxt(txt, "BP"); break;
		case 1: filltxt(txt, "Bytecode"); break;
		case 2: filltxt(txt, "Zeilen"); break;
		case 3: filltxt(txt, "Labels"); break;
		case 4: filltxt(txt, "Befehle"); break;
		case 5: filltxt(txt, "Kommentare"); break;
		default: txt = (char*)malloc(1); sprintf(txt, ""); break; //default case again only here so the compiler doesn't complain. It will never be used
		}
		int neww, newh;
		get_dimension(txt, neww, newh);
		get_col_width_CODE(i) = neww;
		s[i].label = txt;
	}
	if (code != nullptr) {
		//This loop imports all the strings from the loaded file into the tablestyle-array
		for (int i = CCCODE; i < CellsCode; i++) {
			char* txt;
			s[i] = CODE_TEXT;
			switch (i % CCCODE) {
			case 0: filltxt_code(txt, ""); break;
			case 1: filltxt_code(txt, code->bytecode); break;
			case 2: filltxt_code(txt, code->lineOfCode); break;
			case 3: filltxt_code(txt, code->label); break;
			case 4: filltxt_code(txt, code->asmCode); break;
			case 5: filltxt_code(txt, code->comment); code = code->next; break;
			default: txt = (char*)malloc(1); sprintf(txt, ""); break;//default path will never be used, but compiler wants it anyway because of the initialization of txt
			}
			int neww, newh;
			get_dimension(txt, neww, newh);
			get_col_width_CODE(i%6) = max(neww, get_col_width_CODE(i%6));
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
			//no dimension calcuation here because "" is always smaller than any of the header labels
		}
	}
	return s;
}

tablestyle SpRegs_Headers = { nullptr, FL_HELVETICA, FONT_SIZE - 2, FL_NO_BOX, FL_BLACK, FL_BLACK, FL_LIGHT2, FL_ALIGN_CENTER },
		SpRegs_Values = { nullptr, FL_HELVETICA, FONT_SIZE - 2, FL_NO_BOX, FL_BLACK, FL_BLACK, FL_WHITE, FL_ALIGN_CENTER };


tablestyle * setstyle_SpRegs(int type) {
	assert(type >= 0 && type <= 2);
	tablestyle* s = (tablestyle*)malloc(sizeof(tablestyle) * 16);
	char* txt;
	char** Bitnames = &txt;	//Initialization
	char* Bitnames_Status[] = { { "IRP" },{ "RP1" },{ "RP0" },{ "!TO" },{ "!PD" },{ "Z" },{ "DC" },{ "C" } };
	char* Bitnames_Option[] = { { "!RPBU" },{ "INTEDG" },{ "T0CS" },{ "T0SE" },{ "PSA" },{ "PS2" },{ "PS1" },{ "PS0" } };
	char* Bitnames_Intcon[] = { { "GIE" },{ "EEIE" },{ "T0IE" },{ "INTE" },{ "RBIE" },{ "T0IF" },{ "INTF" },{ "RBIF" } };
	switch (type) {
		case 0: Bitnames = Bitnames_Status; break;
		case 1: Bitnames = Bitnames_Option; break;
		case 2: Bitnames = Bitnames_Intcon; break;
	}
	for (int i = 0; i < 16; i++) {
		if (i < 8) {
			s[i] = SpRegs_Headers;
			filltxt(txt, Bitnames[i]);
			s[i].label = txt;
		}
		else {
			s[i] = SpRegs_Values;
			filltxt(txt, "0");
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
			sprintf(txt, "PCL:\t\t%02X", value);
			break;
		}
		case 3: {
			sprintf(txt, "PCLATH:\t%02X", value);
			break;
		}
		case 4: {
			sprintf(txt, "PC:\t\t%04X", value);
			break;
		}
		case 5: {
			sprintf(txt, "Option:\t%02X", value);
			break;
		}
		case 6: {
			sprintf(txt, "INTCON:\t%02X", value);
			break;
		}
	}
	regs->label(txt);
}

void setregtable(tablestyle *& mystyle, int value){
	for (int i = 0; i < 8; i++) {
		sprintf(mystyle[i + 8].label,"%s", (value & (1 << (8 - i))) ? "1" : "0");
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
		sprintf(mystyle[pos + i].label, "%c", value&(1<<(7-i))?high:low);
	}
}