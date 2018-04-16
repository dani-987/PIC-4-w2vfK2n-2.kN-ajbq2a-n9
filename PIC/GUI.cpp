#include "GUI.h"
#include <FL\fl_ask.H>

namespace gui_callbacks {
	void loadFile(Fl_Widget *, void *);
}


GUI::GUI(int x, int y, int w, int h) : Fl_Double_Window(x,y,w,h, "PIC-Simulator")
{
	backend = new Backend(this);
	menubar = new Fl_Menu_Bar(0, 0, w, 20);
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
//######################	I	N	T	E	R	F	A	C	E	#########################
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
	menubar->resize(0, 0, w, 20);
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