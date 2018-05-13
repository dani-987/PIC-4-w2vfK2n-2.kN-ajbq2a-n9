#pragma once

class GUI;

#include "DEBUG.H"
#include "Backend.h"
#include "MyTable.h"
#include "guistyle_config.h"

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Native_File_Chooser.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/fl_ask.H>
#include <FL/Fl_Box.H>
#include <FL/Enumerations.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Sys_Menu_Bar.H>
#include <FL/Fl_Toggle_Button.H>
#include <FL/Fl_Menu_Button.H>
#include <FL/Fl_Scroll.H>
#include <assert.h>

int& get_font_size();
void get_dimension(char* tomeasure, int& width, int& height);
void get_dimension_small(char* tomeasure, int& width, int& height);
int& get_col_width_CODE(int column);
int& get_label_width_button();
int& get_label_height_button();

//The below parameters are the basis for the position and size of everything inside the main Window. Everything else needs to be purely based on this
#define FONT_SIZE			get_font_size()			//Main font size 
#define FONT_SIZE_SMALL		(get_font_size() - 2)	//Font size for the small tables for SpRegs
#define TOPMARGINE			40						//Mimimum distance form the top for everything except the Menubar
#define SIDEMARGINE			20
#define BOTTOMMARGINE		20
#define WINDOW_WIDTH		w
#define WINDOW_HEIGTH		h
#define BASE_LABEL_MEM		"Tris"					//Longest table-label of the MEM-Block, basis for the dímensions of its two tables
#define BASE_LABEL_SPREGS	"INTEDG"				//Longest table-label of the SpRegs-Block, basis for the dímensions of its three tables
#define BASE_LABEL_BUTTON	"Reset"					//Longest label of any or the controll buttons
#define INTERSPACE			20						//Distance between two Areas of the Main window
#define INTERSPACE_SMALL	10						//Distance between two Areas in the scroll-subarea (SpRegs)
#define SCROLLBAR_WIDTH		30

//Sizes of the Cells in the static tables; IO-Table uses the same measures as the MEM-table and Code-table uses the same height
#define LABEL_WIDTH_MEM			get_cell_width_Mem()
#define CELL_WIDTH_MEM			((int)((float)LABEL_WIDTH_MEM * 1.25f))
#define ROW_HEADER_WIDTH_MEM	((int)((float)LABEL_WIDTH_MEM * 1.5f))
#define LABEL_HEIGHT_MEM		get_cell_height_Mem()
#define CELL_HEIGHT_MEM			((int)((float)LABEL_HEIGHT_MEM * 1.2f))
#define COL_HEADER_HEIGHT_MEM	((int)((float)LABEL_HEIGHT_MEM * 1.5f))

#define LABEL_WIDTH_SPREGS		get_cell_width_SpRegs()
#define CELL_WIDTH_SPREGS		((int)((float)LABEL_WIDTH_SPREGS * 1.2f))
#define LABEL_HEIGHT_SPREGS		get_cell_height_SpRegs()
#define CELL_HEIGHT_SPREGS		((int)((float)LABEL_HEIGHT_MEM * 1.5f))

#define LABEL_WIDTH_CODE(a)		get_col_width_CODE(a)
#define CELL_WIDTH_CODE(a)		((int)((float)LABEL_WIDTH_CODE(a) * 1.2f))
#define LABEL_HEIGHT_CODE		LABEL_HEIGHT_MEM
#define CELL_HEIGHT_CODE		CELL_HEIGHT_MEM
#define COL_HEADER_HEIGHT_CODE	COL_HEADER_HEIGHT_MEM


//Cell Counts of the static tables
#define RCMEM 16										//Number of Rows of the Memory Table
#define CCMEM 8											//Number of Colums of the Memory Table
#define CELL_COUNT_MEM 153								//Total number of Cells of the Memory Table (including headers)
#define RCIO 6											//Number of Rows of the IO-Table
#define CCIO 8											//Number of Colums of the IO-Table
#define CELL_COUNT_IO 54								//Total number of Cells of the IO-Table (including headers)
#define RCSPREGS 2										//Number of Colums of one of the Special Regsiter tables
#define CCSPREGS 8										//Number of Colums all special register tables

//Cell Counts in the changing Code table
#define CCCODE	6										//Number of Colums in the Code Table
#define RCCODE 30										//Default number of rows in the Code table, will be overwritten by load file
#define CellsCODE ((CODE_table->rows() + 1) * CODE_table->cols())	//Number of Cells in the current CODE-Table 

//Sizes of the controll buttons
#define LABEL_WIDTH_BUTTON		get_label_width_button()
#define BOX_WIDTH_BUTTON		(LABEL_WIDTH_BUTTON * 2)
#define LABEL_HEIGHT_BUTTON		get_label_height_button()
#define BOX_HEIGHT_BUTTON		(LABEL_HEIGHT_BUTTON * 2)

#define WINDOW_BASE_X_OFFSET	10
#define WINDOW_BASE_Y_OFFSET	5
#define GUI_STANDARD_W	Fl::w()-20
#define GUI_STANDARD_H	Fl::h()-35

//Numbers of Objects of each type in the SPREGS Scroll subwindow
#define BOXES 8
#define TABLES 3

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
	Fl_Button* SetW;
	Fl_Button* Timerreset;

	int finished_startup;

public:
	GUI(int x = WINDOW_BASE_X_OFFSET, int y = WINDOW_BASE_Y_OFFSET, int w = GUI_STANDARD_W, int h = GUI_STANDARD_H);
	~GUI();

	Backend*& getbackend();

	int run();

	//		All interfaces to backend
	void int_updateAll();
	void int_update();
	void handle_error();

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
	void callback_setW();
	void callback_changeOutput();
	void callback_setMem();
	void callback_resettimer();
	void callback_togglebreakpoint();
};


//		All interfaces to backend
void gui_int_update(void* gui);
void gui_int_updateAll(void* gui); 
void gui_handle_error(void* gui);

/*
void gui_callback_xy(void* gui) {
	((GUI*)gui)->callback_xy();
}
*/