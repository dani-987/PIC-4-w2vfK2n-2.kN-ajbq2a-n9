#include "GUI.h"


namespace gui_callbacks {
	void loadFile(Fl_Widget *, void *);
}


GUI::GUI(int x, int y, int w, int h) : Fl_Double_Window(x,y,w,h, "PIC-Simulator")
{
	backend = new Backend();
	menubar = new Fl_Menu_Bar(0, 0, w, 20);
	menubar->add("&Datei/&Lade Datei", nullptr, gui_callbacks::loadFile, this);
	Fl::scheme(SCHEME);
	size_range(600, 400);
	color(FL_WHITE);

	chooser = new Fl_File_Chooser(".",                      // directory
		"*.LST",											// filter
		Fl_File_Chooser::SINGLE,							// chooser type
		"Wähle .LST-Datei aus");							// title
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
	chooser->show();
	while (chooser->shown()){Fl::wait();}
	if (chooser->value() == NULL)return;
	PRINTF1("Choosed File: '%s'", chooser->value());
	if(!backend->LoadProgramm((char*)chooser->value()))
		fl_alert(backend->getErrorMSG());
}