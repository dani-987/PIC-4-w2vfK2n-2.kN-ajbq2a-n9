#include "GUI.h"

#define X_MENUBAR		0
#define Y_MENUBAR		0
#define W_MENUBAR		w
#define H_MENUBAR		20

#define W_IO_TAB		((CW*(CCIO + 1.2))+20)
#define H_IO_TAB		(CH*(RCIO)+2)
#define X_IO_TAB		10
#define Y_IO_TAB		(h-(10+H_IO_TAB))

#define X_MEM_TAB		X_IO_TAB
#define Y_MEM_TAB		40
#define W_MEM_TAB		W_IO_TAB
#define H_MEM_TAB		(Y_IO_TAB-(Y_MEM_TAB+20))

namespace gui_callbacks {
	void loadFile(Fl_Widget *, void *);
}


GUI::GUI(int x, int y, int w, int h) : Fl_Double_Window(x,y,w,h, "PIC-Simulator")
{
	backend = new Backend(this);
	menubar = new Fl_Menu_Bar(X_MENUBAR, Y_MENUBAR, W_MENUBAR, H_MENUBAR);
	menubar->add("&Datei/&Lade Datei", nullptr, gui_callbacks::loadFile, this);
	Fl::scheme(SCHEME);
	size_range(620, 440);
	color(FL_WHITE);

	char* myPath = (char*)malloc(256);
	if (myPath != nullptr) {
		memset(myPath, 0, 256);
		PWSTR path = NULL;
		if (SHGetKnownFolderPath(FOLDERID_Documents, 0, NULL, &path)) {
			wcstombs(myPath, path, 255);
			CoTaskMemFree(path);
		}
	}

	chooser = new Fl_Native_File_Chooser();
	chooser->title("Programmdatei aussuchen...");
	chooser->type(Fl_Native_File_Chooser::BROWSE_FILE);
	chooser->filter("Programmdatei\t*.LST\n");
	chooser->directory(myPath);	//myPath == nullptr macht keine Probleme...

	if (myPath != nullptr)free(myPath);

	//Table for the Memory
	Mem_table = new MyTable(X_MEM_TAB, Y_MEM_TAB, W_MEM_TAB, H_MEM_TAB, 0, "Memory");
	Mem_table->selection_color(FL_YELLOW);
	Mem_table->when(FL_WHEN_RELEASE | FL_WHEN_CHANGED);
	Mem_table->table_box(FL_NO_BOX);


	// ROWS
	Mem_table->row_header(1);
	Mem_table->row_header_width(CW*1.2);
	Mem_table->row_resize(0);
	Mem_table->rows(RCMEM);
	Mem_table->row_height_all(CH);

	// COLS
	Mem_table->cols(CCMEM);
	Mem_table->col_header(1);
	Mem_table->col_header_height(CH*1.2);
	Mem_table->col_resize(0);
	Mem_table->col_width_all(CW);

	//Table for the IO-Registers
	IO_table = new MyTable(X_IO_TAB, Y_IO_TAB, W_IO_TAB, H_IO_TAB, 1, "IO-Registers");
	IO_table->when(FL_WHEN_RELEASE | FL_WHEN_CHANGED);
	IO_table->table_box(FL_NO_BOX);

	//ROWS
	IO_table->row_resize(0);
	IO_table->row_header(1);
	IO_table->row_header_width(CW*1.2);
	IO_table->rows(RCIO);
	IO_table->row_height_all(CH);

	//COLS
	IO_table->cols(CCIO);
	IO_table->col_resize(0);
	IO_table->col_width_all(CW);
}


GUI::~GUI()
{
}

int GUI::run()
{
	this->show();
	return Fl::run();
}


//#######################################################################################
//#######################################################################################
//######################  I	  N	  T	  E	  R	  F	  A	  C	  E   S	#########################
//#######################################################################################
//#######################################################################################

void GUI::int_updateAll()
{
}

//#######################################################################################
//#######################################################################################
//######################	O	V	E	R	R	I	D	E	S	#########################
//#######################################################################################
//#######################################################################################

void GUI::resize(int x, int y, int w, int h){
	Fl_Double_Window::resize(x, y, w, h);
	menubar->resize(X_MENUBAR, Y_MENUBAR, W_MENUBAR, H_MENUBAR);
	Mem_table->resize(X_MEM_TAB, Y_MEM_TAB, W_MEM_TAB, H_MEM_TAB);
	IO_table->resize(X_IO_TAB, Y_IO_TAB, W_IO_TAB, H_IO_TAB);
	flush();
}

//#######################################################################################
//#######################################################################################
//######################	C	A	L	L	B	A	C	K	S	#########################
//#######################################################################################
//#######################################################################################

void gui_callbacks::loadFile(Fl_Widget *w, void *gui){
	((GUI*)gui)->callback_load_file();
}

void GUI::callback_load_file(){
	if (chooser->show() != 0)return;
	PRINTF1("Choosed File: '%s'", chooser->filename());
	if(!backend->LoadProgramm((char*)chooser->filename()))
		fl_alert(backend->getErrorMSG());
	else {
		//updateAll();
	}
}