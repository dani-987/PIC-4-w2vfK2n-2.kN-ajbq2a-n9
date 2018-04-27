#pragma once
//This File contains the hardcoded configuration for what every table in our GUI looks like
//This is realized through the struct tablestyle. It contains all the necessary properties to draw cells with the correct colors, labels, ...
//Each Table gets an Array of tablestyles with one Object per Cell, including headers

struct tablestyle;
typedef struct tablestyle tablestyle;
// ROW ->    Collum |
//					v

#include "GUI.h"
#include <FL\Enumerations.H>


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
tablestyle* setstyle_Code(int lines);
