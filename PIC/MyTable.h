#pragma once

class MyTable;

#include "DEBUG.H"
#include "GUI.h"
#include "tablestyle_config.h"

#include <math.h> 
#include <FL/fl_draw.H>
#include <FL/Fl_Table_Row.H>


class MyTable : public Fl_Table_Row
{
private:
	Fl_Color cell_bgcolor;				// color of cell's bg color
	Fl_Color cell_fgcolor;				// color of cell's fg color
	tablestyle* mystyle;				// the style-Array of the table

protected:
	void draw_cell(TableContext context,  		// table cell drawing
		int R = 0, int C = 0, int X = 0, int Y = 0, int W = 0, int H = 0);
	static void event_callback(Fl_Widget*, void*);
	void event_callback2();				// callback for table events
public:
	MyTable(int x, int y, int w, int h, const char *l = 0);
	~MyTable();

};