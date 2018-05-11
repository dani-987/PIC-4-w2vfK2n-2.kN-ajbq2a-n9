#pragma once

class MyTable;

#include "DEBUG.H"
#include "GUI.h"
#include "guistyle_config.h"

#include <math.h> 
#include <FL/fl_draw.H>
#include <FL/Fl_Table_Row.H>

#define COLHEADERPOS(C) (C+((row_header())?(1):(0)))
#define ROWHEADERPOS(R) ((R+((col_header())?(1):(0)))*(cols()+1))
#define CELLPOS(R,C)	((R+((col_header())?(1):(0)))*(cols()+((row_header())?(1):(0)))+C+((row_header())?(1):(0)))

class MyTable : public Fl_Table_Row
{
private:
	Fl_Color cell_bgcolor;				// color of cell's bg color
	Fl_Color cell_fgcolor;				// color of cell's fg color
	tablestyle* mystyle;				// the style-Array of the table
	int codeline;

protected:
	void draw_cell(TableContext context,  		// table cell drawing
		int R = 0, int C = 0, int X = 0, int Y = 0, int W = 0, int H = 0);
	static void event_callback(Fl_Widget*, void*);
	void event_callback2();				// callback for table events
public:
	MyTable(int x, int y, int w, int h, const char *l = 0);
	~MyTable();
	int getcodeline();
	void setcodeline(int newline);
	tablestyle*& getstyle();
};