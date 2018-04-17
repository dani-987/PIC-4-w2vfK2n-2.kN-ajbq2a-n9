#include "GUI.h"


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

	//Table for the Memory
	Mem_table = new MyTable(20, 40, CW*(CCMEM + 1.2), CH*(RCMEM + 1.2), 0, "Memory");
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
	IO_table = new MyTable(20, 60 + CH * (RCMEM + 1.2), CW*(CCMEM + 1.2), CW*(CCIO + 1.2), 1, "IO-Registers");
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


MyTable::MyTable(int x, int y, int w, int h, int t, const char *l) : Fl_Table_Row(x, y, w, h, l) {
	cell_bgcolor = FL_WHITE;
	cell_fgcolor = FL_BLACK;
	callback(&event_callback, (void*)this);
	typ = t;
	end();
}
MyTable::~MyTable(){

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
	menubar->resize(0, 0, w, 20);
	flush();
}


void MyTable::draw_cell(TableContext context, int R, int C, int X, int Y, int W, int H) {
	switch (typ) {
	case 0:draw_cell_MEM(context, R, C, X, Y, W, H); return;
	case 1:draw_cell_IO(context, R, C, X, Y, W, H); return;
	}
}

void MyTable::draw_cell_MEM(TableContext context, int R, int C, int X, int Y, int W, int H){
	static char s[10];

	switch (context)
	{
	case CONTEXT_STARTPAGE:
		fl_font(FL_HELVETICA, 16);
		return;

	case CONTEXT_COL_HEADER:
		sprintf_s(s, "%d%d", 0, C);
		fl_push_clip(X, Y, W, H);
		{
			fl_draw_box(FL_THIN_UP_BOX, X, Y, W, H, col_header_color());
			fl_color(FL_BLACK);
			fl_draw(s, X, Y, W, H, FL_ALIGN_CENTER);
		}
		fl_pop_clip();
		return;

	case CONTEXT_ROW_HEADER:
		sprintf_s(s, "%d%d", (int)floor(R / 2), (R % 2) ? 8 : 0);
		fl_push_clip(X, Y, W, H);
		{
			fl_draw_box(FL_THIN_UP_BOX, X, Y, W, H, row_header_color());
			fl_color(FL_BLACK);
			fl_draw(s, X, Y, W, H, FL_ALIGN_CENTER);
		}
		fl_pop_clip();
		return;

	case CONTEXT_CELL:
	{
		sprintf_s(s, "%d%d", 0, 0);
		fl_push_clip(X, Y, W, H);
		{
			// BG COLOR
			fl_color(is_selected(R, C) ? selection_color() : cell_bgcolor);
			fl_rectf(X, Y, W, H);

			// TEXT
			fl_color(cell_fgcolor);
			fl_draw(s, X, Y, W, H, FL_ALIGN_CENTER);

			// BORDER
			fl_color(color());
			fl_rect(X, Y, W, H);
		}
		fl_pop_clip();
		return;
	}

	case CONTEXT_TABLE:
		fprintf(stderr, "TABLE CONTEXT CALLED\n");
		return;

	case CONTEXT_ENDPAGE:
	case CONTEXT_RC_RESIZE:
	case CONTEXT_NONE:
		return;
	}
}

void MyTable::draw_cell_IO(TableContext context, int R, int C, int X, int Y, int W, int H){
	static char s[10];

	switch (context)
	{
	case CONTEXT_STARTPAGE:
		fl_font(FL_HELVETICA, 16);
		return;

	case CONTEXT_ROW_HEADER:
		switch (R) {
		case 0: sprintf_s(s, "RA"); break;
		case 1:case 4: sprintf_s(s, "Tris"); break;
		case 2:case 5: sprintf_s(s, "Pin"); break;
		case 3:sprintf_s(s, "RB"); break;
		}
		fl_push_clip(X, Y, W, H);
		{
			fl_draw_box(FL_THIN_UP_BOX, X, Y, W, H, row_header_color());
			fl_color(FL_BLACK);
			fl_draw(s, X, Y, W, H, FL_ALIGN_CENTER);
		}
		fl_pop_clip();
		return;

	case CONTEXT_CELL:
	{
		switch (R % 3) {
		case 0: sprintf_s(s, "%d", 7 - C); break;
		case 1: sprintf_s(s, "in"); break;
		case 2: sprintf_s(s, "0"); break;
		}
		fl_push_clip(X, Y, W, H);
		{
			// BG COLOR
			fl_color(is_selected(R, C) ? selection_color() : cell_bgcolor);
			fl_rectf(X, Y, W, H);

			// TEXT
			fl_color(cell_fgcolor);
			fl_draw(s, X, Y, W, H, FL_ALIGN_CENTER);

			// BORDER
			fl_color(color());
			fl_rect(X, Y, W, H);
		}
		fl_pop_clip();
		return;
	}

	case CONTEXT_TABLE:
		fprintf(stderr, "TABLE CONTEXT CALLED\n");
		return;

	case CONTEXT_ENDPAGE:
	case CONTEXT_RC_RESIZE:
	case CONTEXT_NONE:
		return;
	}
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

void MyTable::event_callback(Fl_Widget*, void *data)
{
	MyTable *o = (MyTable*)data;
	o->event_callback2();
}

void MyTable::event_callback2()
{
	int R = callback_row(),
		C = callback_col();
	TableContext context = callback_context();
	printf("'%s' callback: ", (label() ? label() : "?"));
	printf("Row=%d Col=%d Context=%d Event=%d InteractiveResize? %d\n",
		R, C, (int)context, (int)Fl::event(), (int)is_interactive_resize());
}