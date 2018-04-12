#pragma once

class GUI;

#include "DEBUG.H"
#include "Backend.h"
#include <FL\Fl.H>
#include <FL\Fl_Double_Window.H>
#include <FL\Fl_Menu_Bar.H>
#include <FL\Fl_File_Chooser.H>

#define GUI_STANDARD_X	10
#define GUI_STANDARD_Y	5
#define GUI_STANDARD_W	Fl::w()-20
#define GUI_STANDARD_H	Fl::h()-35

#define SCHEME			"gtk+"

class GUI : public Fl_Double_Window
{
private:
	Backend* backend;

	Fl_Menu_Bar* menubar;
	Fl_File_Chooser* chooser;
public:
	GUI(int x = GUI_STANDARD_X, int y = GUI_STANDARD_Y, int w = GUI_STANDARD_W, int h = GUI_STANDARD_H);
	~GUI();

	int run();

	//interface to backend
	void int_updateAll();

	//overrides
	void resize(int x, int y, int w, int h);

	//callbacks
	void callback_load_file();
};