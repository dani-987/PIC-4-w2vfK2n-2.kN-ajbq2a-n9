#pragma once

class MyTable;

#include "DEBUG.H"
#include <FL/Fl_Table_Row.H>

#define CW 40		//Width of a Cell
#define CH 30		//Heigth of a Cell
#define RCMEM 16	//Number of Rows of the Memory Table
#define CCMEM 8		//Number of Colums of the Memory Table
#define RCIO 6		//Number of Rows of the IO-Table
#define CCIO 8		//Number of Colums of the IO-Table

class MyTable : public Fl_Table_Row
{
private:
	Fl_Color cell_bgcolor;				// color of cell's bg color
	Fl_Color cell_fgcolor;				// color of cell's fg color
	int typ;						// The typ of table: MEM = 0; IO = 1; ...

protected:
	void draw_cell(TableContext context,  		// table cell drawing
		int R = 0, int C = 0, int X = 0, int Y = 0, int W = 0, int H = 0);
	void draw_cell_MEM(TableContext context,  		// table cell drawing for Memory Table
		int R = 0, int C = 0, int X = 0, int Y = 0, int W = 0, int H = 0);
	void draw_cell_IO(TableContext context,  		// table cell drawing for IO-Table
		int R = 0, int C = 0, int X = 0, int Y = 0, int W = 0, int H = 0);
	static void event_callback(Fl_Widget*, void*);
	void event_callback2();				// callback for table events
public:
	MyTable(int x, int y, int w, int h, int t, const char *l = 0);
	~MyTable();

};