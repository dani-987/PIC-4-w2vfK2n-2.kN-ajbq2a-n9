#pragma once

class GUI;

#include "DEBUG.H"
#include "Backend.h"
#include "MyTable.h"
#include "tablestyle_config.h"

#include <FL\Fl.H>
#include <FL\Fl_Double_Window.H>
#include <FL\Fl_Menu_Bar.H>
#include <FL\Fl_Native_File_Chooser.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/fl_ask.H>
#include <FL/Fl_Box.H>
#include <FL/Enumerations.H>

#define CW 40		//Width of a Cell
#define CH 30		//Heigth of a Cell
#define RCMEM 16	//Number of Rows of the Memory Table
#define CCMEM 8		//Number of Colums of the Memory Table
#define CellsMEM 153//Total number of Cells of the Memory Table (including headers)
#define RCIO 6		//Number of Rows of the IO-Table
#define CCIO 8		//Number of Colums of the IO-Table
#define CellsIO 54	//Total number of Cells of the IO-Table (including headers)


#define WINDOW_BASE_X_OFFSET	10
#define WINDOW_BASE_Y_OFFSET	5
#define GUI_STANDARD_W	Fl::w()-20
#define GUI_STANDARD_H	Fl::h()-35

#define SCHEME			"none"

class GUI : public Fl_Double_Window
{
private:
	Backend* backend;

	Fl_Menu_Bar* menubar;
	Fl_Native_File_Chooser* chooser;

	MyTable *Mem_table;
	MyTable *IO_table;
public:
	GUI(int x = WINDOW_BASE_X_OFFSET, int y = WINDOW_BASE_Y_OFFSET, int w = GUI_STANDARD_W, int h = GUI_STANDARD_H);
	~GUI();

	int run();

	//		All interfaces to backend
	void int_updateAll();

	//		All Overrides of the GUI
	void resize(int x, int y, int w, int h);

	//		All Callbacks 
	//Callback for opneing file
	void callback_load_file();

	//void callback_xy();
};


/*
void gui_callback_xy(void* gui) {
	((GUI*)gui)->callback_xy();
}
*/