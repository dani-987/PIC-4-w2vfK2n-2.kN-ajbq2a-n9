#pragma once
//This File contains the hardcoded configuration for what every table in our GUI looks like
//This is realized through the struct tablestyle. It contains all the necessary properties to draw cells with the correct colors, labels, ...
//Each Table gets an Array of tablestyles with one Object per Cell, including headers

struct tablestyle;
typedef struct tablestyle tablestyle;
// ROW ->    Collum |
//					v

#include "GUI.h"
#include <FL/Enumerations.H>
#include <FL/Fl_Box.H>


//Contains all necessary style information to draw a cell
struct tablestyle {
	char* label;
	Fl_Font font;
	Fl_Fontsize fontsize;
	Fl_Boxtype boxtyp;
	Fl_Color textcolor;
	Fl_Color bordercolor;
	Fl_Color backgroundcolor;
	Fl_Align align;
};

//Methodes that generte the style-Arrays for each table
tablestyle* setstyle_MEM();
tablestyle* setstyle_IO();
tablestyle* setstyle_Code(int lines, ASM_TEXT* code);
tablestyle* setstyle_SpRegs(int type);	//type: 0=Status; 1=Option, 2=Intcon


void freetablestyle(tablestyle*& tofree, int cells);

//Updates the labels of the tables
void setregbox(Fl_Box*& regs, int line, int value);
void setregtable(tablestyle*& mystyle, int value);
void setMEMcell(tablestyle*& mystyle, int pos, int bank, int value);
void setIOcell(tablestyle*& mystyle, int line, int value);