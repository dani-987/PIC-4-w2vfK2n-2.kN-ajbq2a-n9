#include "tablestyle_config.h"

//Tablestyles for the Memory Table
tablestyle MEM_Startpage,
	MEM_Rowheaders,
	MEM_Colheaders,
	MEM_Reserved_B0,
	MEM_Reserved_B1,
	MEM_Available,
	MEM_Uninstalled;

tablestyle * setstyle_MEM()
{
	tablestyle* s = (tablestyle*)malloc(sizeof(tablestyle) * CellsMEM);
	for (int i = 0; i < CellsMEM; i++) {
		if (!i) { s[i] = MEM_Startpage;}
		//the first few types have unchanging labels that are set here and never change
		else if (i < 9) { 
			s[i] = MEM_Colheaders; 
			char txt[2];
			sprintf(txt, "%02d", 8 - i);
			s[i].label = txt;
		}
		else if (!(i % 9)) {
			s[i] = MEM_Rowheaders;
			char txt[2];
			sprintf(txt, "%d%d", (i<36)?0:(((i-36)%9)/2), (i%2)?0:8);
			s[i].label = txt;
		}
		else if (i == 17 || (i > 21 && i < 27) || (i > 39 && i < 49) || i > 99) {
			s[i] = MEM_Uninstalled;
			s[i].label = "00";
		}
		//Anything below here is initilaized with "00" and has it's label updated on change of the respective Memory Adress
		else if (i > 22) {
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


tablestyle IO_Rowheaders,
	IO_DigitEnum,
	IO_TrisValues,
	IO_Values;

tablestyle * setstyle_IO()
{
	tablestyle* s = (tablestyle*)malloc(sizeof(tablestyle) * CellsIO);
	for (int i = 0; i < CellsIO; i++) {
		if (!(i % 9)) {
			s[i] = IO_Rowheaders;
			char txt[10];
			switch (i % 9) {
			case 0: sprintf(txt, "RA"); break;
			case 1: case 4: sprintf(txt, "Tris"); break;
			case 2: case 5: sprintf(txt, "Pin"); break;
			case 3: sprintf(txt, "RB"); break;
			}
			s[i].label = txt;
		}
		else if ((i % 27) > 0 && (i % 27) < 9) {
			s[i] = IO_DigitEnum;
			char txt[2];
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
