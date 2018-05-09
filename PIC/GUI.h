#pragma once

class GUI;

#include "DEBUG.H"
#include "Backend.h"
#include "MyTable.h"
#include "guistyle_config.h"

#include <FL\Fl.H>
#include <FL\Fl_Double_Window.H>
#include <FL\Fl_Menu_Bar.H>
#include <FL\Fl_Native_File_Chooser.H>
#include <FL\Fl_Input.H>
#include <FL\Fl_Check_Button.H>
#include <FL\Fl_Choice.H>
#include <FL\fl_ask.H>
#include <FL\Fl_Box.H>
#include <FL\Enumerations.H>
#include <FL\Fl_Button.H>
#include <FL/Fl_Sys_Menu_Bar.H>
#include <FL/Fl_Toggle_Button.H>
#include <FL/Fl_Menu_Button.H>
#include <FL\Fl_Scroll.H>
#include <assert.h>

int __get_font_size_table_();

#define FONT_SIZE_TABLE __get_font_size_table_()

#define CW ((int)(2.5f * (float)FONT_SIZE_TABLE))		//Width of a Cell
#define CH ((int)(1.875f * (float)FONT_SIZE_TABLE))		//Heigth of a Cell

#define CW_S ((int)(2.5f * (float)(FONT_SIZE_TABLE - 2)))		//Width of asmall  Cell
#define CH_S ((int)(1.875f * (float)(FONT_SIZE_TABLE - 2)))		//Heigth of a small Cell

#define RCMEM 16										//Number of Rows of the Memory Table
#define CCMEM 8											//Number of Colums of the Memory Table
#define CellsMEM 153									//Total number of Cells of the Memory Table (including headers)

#define RCIO 6											//Number of Rows of the IO-Table
#define CCIO 8											//Number of Colums of the IO-Table
#define CellsIO 54										//Total number of Cells of the IO-Table (including headers)

#define CCCODE	5										//Number of Colums in the Code Table
#define RCCODE 30										//Default number of rows in the Code table, will be overwritten by load file
#define CellsCODE ((CODE_table->rows() + 1) * CODE_table->cols())	//Number of Cells in the current CODE-Table 

#define WINDOW_BASE_X_OFFSET	10
#define WINDOW_BASE_Y_OFFSET	5
#define GUI_STANDARD_W	Fl::w()-20
#define GUI_STANDARD_H	Fl::h()-35

#define BOXES 7

#define SCHEME			"none"

class GUI : public Fl_Double_Window
{
private:
	Backend* backend;
	Fl_Menu_Item* menutable;

	Fl_Menu_Bar* menubar;
	Fl_Native_File_Chooser* chooser;

	MyTable *Mem_table;
	MyTable *IO_table;
	MyTable *CODE_table;

	//Some FL_Boxes and MyTables that are used to depict the special registers in detail
	//they are seperated, because their frequency of updating is extremly different (W and PC vs. Option)
	Fl_Scroll* subwin;
	Fl_Box** registers;
	MyTable** regtables;

	//The buttons used to controll the program execution
	Fl_Button* Start;
	Fl_Button* Stop;
	Fl_Button* Step;
	Fl_Button* Reset;

public:
	GUI(int x = WINDOW_BASE_X_OFFSET, int y = WINDOW_BASE_Y_OFFSET, int w = GUI_STANDARD_W, int h = GUI_STANDARD_H);
	~GUI();

	Backend*& getbackend();

	int run();

	//		All interfaces to backend
	void int_updateAll();
	void int_update();

	//		All Overrides of the GUI
	void resize(int x, int y, int w, int h);

	//		All Callbacks 
	//Callback for opening file
	void callback_load_file();

	static void window_cb(Fl_Widget*, void*);

	//Callbacks for controll buttons
	void callback_start();
	void callback_stop();
	void callback_step();
	void callback_reset();
	void callback_settact(int freq);
	void callback_watchdog();
};

//TODO: def in .cpp
//		All interfaces to backend
void gui_int_update(void* gui);
void gui_int_updateAll(void* gui); 

/*
void gui_callback_xy(void* gui) {
	((GUI*)gui)->callback_xy();
}
*/