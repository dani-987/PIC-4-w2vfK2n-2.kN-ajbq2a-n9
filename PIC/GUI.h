#pragma once

class GUI;

#include "DEBUG.H"
#include "Backend.h"
#include <FL\Fl.H>
#include <FL\Fl_Double_Window.H>
#include <FL\Fl_Menu_Bar.H>
#include <FL\Fl_Native_File_Chooser.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/fl_draw.H>
#include <FL/fl_ask.H>
#include <FL/Fl_Table_Row.H>


#define WINDOW_BASE_X_OFFSET	10
#define WINDOW_BASE_Y_OFFSET	5
#define GUI_STANDARD_W	Fl::w()-20
#define GUI_STANDARD_H	Fl::h()-35

#define CW 40		//Width of a Cell
#define CH 30		//Heigth of a Cell
#define RCMEM 16	//Number of Rows of the Memory Table
#define CCMEM 8		//Number of Colums of the Memory Table
#define RCIO 6		//Number of Rows of the IO-Table
#define CCIO 8		//Number of Colums of the IO-Table


#define SCHEME			"none"

class GUI : public Fl_Double_Window
{
private:
	Backend* backend;

	Fl_Menu_Bar* menubar;
	Fl_Native_File_Chooser* chooser;
public:
	GUI(int x = WINDOW_BASE_X_OFFSET, int y = WINDOW_BASE_Y_OFFSET, int w = GUI_STANDARD_W, int h = GUI_STANDARD_H);
	~GUI();

	int run();

	//interface to backend
	void int_updateAll();

	//overrides
	void resize(int x, int y, int w, int h);

	//callbacks
	void callback_load_file();

	//void callback_xy();
};


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

static MyTable *Mem_table = 0;
static MyTable *IO_table = 0;


/*
void gui_callback_xy(void* gui) {
	((GUI*)gui)->callback_xy();
}
*/