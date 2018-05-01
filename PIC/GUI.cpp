#include "GUI.h"

int __font_size_table_ = 14;
int __get_font_size_table_() {return __font_size_table_;}

void gui_int_update(void * gui){((GUI*)gui)->int_update();}

void gui_int_updateAll(void * gui){((GUI*)gui)->int_updateAll();}

//Position and size of the menu-bar
#define X_MENUBAR		0
#define Y_MENUBAR		0
#define W_MENUBAR		w
#define H_MENUBAR		20

//Position and size of the IO-Table
#define W_IO_TAB		((CW*(CCIO + 1.2))+20)
#define H_IO_TAB		(CH*(RCIO)+2)
#define X_IO_TAB		10
#define Y_IO_TAB		(h-(10+H_IO_TAB))

//Position and size of the MEM-Table
#define X_MEM_TAB		X_IO_TAB
#define Y_MEM_TAB		40
#define W_MEM_TAB		W_IO_TAB
#define H_MEM_TAB		(Y_IO_TAB-(Y_MEM_TAB+20))

//Position and size of the CODE-Table
#define W_CODE_TAB		(w / 3)
#define H_CODE_TAB		(h - 50)
#define X_CODE_TAB		(w - (W_CODE_TAB + 10))
#define Y_CODE_TAB		40

//Position and size of the Spezial Registers Box



namespace gui_callbacks {
	void loadFile(Fl_Widget *, void *);
}

//Generates the GUI initial and creates the Backend
GUI::GUI(int x, int y, int w, int h) : Fl_Double_Window(x,y,w,h, "PIC-Simulator")
{
	//Create Backend
	backend = new Backend(this);

	/*//TODO: fontsize ändern <- sollte in resize gemacht werden?
	if (w < 750)__font_size_table_ = 12;
	else if (w < 1000)__font_size_table_ = 14;
	else __font_size_table_ = 16;
	*/

	//create Menubar
	menubar = new Fl_Menu_Bar(X_MENUBAR, Y_MENUBAR, W_MENUBAR, H_MENUBAR);
	menubar->add("&Datei/&Lade Datei", nullptr, gui_callbacks::loadFile, this);

	//Fl::scheme(SCHEME);

	//inital size
	size_range(620, 440);
	color(FL_WHITE);
	callback(window_cb);

	//Generate initial Path for open file dialog
	char* myPath = (char*)malloc(256);
	if (myPath != nullptr) {
		memset(myPath, 0, 256);
		PWSTR path = NULL;
		if (SHGetKnownFolderPath(FOLDERID_Documents, 0, NULL, &path)) {
			wcstombs(myPath, path, 255);
			CoTaskMemFree(path);
		}
	}

	//Genetrate native file chosser for open file dialog
	chooser = new Fl_Native_File_Chooser();
	chooser->title("Programmdatei aussuchen...");
	chooser->type(Fl_Native_File_Chooser::BROWSE_FILE);
	chooser->filter("Programmdatei\t*.LST\n");
	chooser->directory(myPath);	//myPath == nullptr macht keine Probleme...

	if (myPath != nullptr)free(myPath);

	//Table for the Memory
	Mem_table = new MyTable(X_MEM_TAB, Y_MEM_TAB, W_MEM_TAB, H_MEM_TAB, "Memory");
	Mem_table->getstyle() = setstyle_MEM();
	Mem_table->selection_color(FL_YELLOW);
	Mem_table->when(FL_WHEN_RELEASE | FL_WHEN_CHANGED);
	Mem_table->table_box(FL_NO_BOX);


	// Configure table rows
	Mem_table->row_header(1);
	Mem_table->row_header_width(CW*1.2);
	Mem_table->row_resize(0);
	Mem_table->rows(RCMEM);
	Mem_table->row_height_all(CH);

	// Configure table collums
	Mem_table->cols(CCMEM);
	Mem_table->col_header(1);
	Mem_table->col_header_height(CH*1.2);
	Mem_table->col_resize(0);
	Mem_table->col_width_all(CW);

	//Table for the IO-Registers
	IO_table = new MyTable(X_IO_TAB, Y_IO_TAB, W_IO_TAB, H_IO_TAB, "IO-Registers");
	IO_table->getstyle() = setstyle_IO();
	IO_table->when(FL_WHEN_RELEASE | FL_WHEN_CHANGED);
	IO_table->table_box(FL_NO_BOX);

	//Configure table rows
	IO_table->row_resize(0);
	IO_table->row_header(1);
	IO_table->row_header_width(CW*1.2);
	IO_table->rows(RCIO);
	IO_table->row_height_all(CH);

	//Configure table collums
	IO_table->cols(CCIO);
	IO_table->col_resize(0);
	IO_table->col_width_all(CW);

	//Table for the Code
	CODE_table = new MyTable(X_CODE_TAB, Y_CODE_TAB, W_CODE_TAB, H_CODE_TAB, "CODE");
	CODE_table->getstyle() = setstyle_Code(RCCODE, nullptr);
	//CODE_table->selection_color(FL_YELLOW);
	CODE_table->when(FL_WHEN_RELEASE | FL_WHEN_CHANGED);
	CODE_table->table_box(FL_NO_BOX);


	// Configure table rows
	CODE_table->rows(RCCODE);
	CODE_table->row_resize(0);
	CODE_table->row_height_all(CH);

	// Configure table collums
	CODE_table->cols(CCCODE);
	CODE_table->col_header(1);
	CODE_table->col_header_height(CH*1.2);
	CODE_table->col_resize(0);
	CODE_table->col_width_all(w/15);

	registers = (Fl_Box**)malloc(sizeof(Fl_Box*) * BOXES);

	//The boxes are initialy setup with boxes of size 0. Their proper size is set in GUI::resize
	for (int i = 0; i < BOXES; i++) {
		registers[i] = new Fl_Box(FL_NO_BOX, w, h, 0, 0, "");
		setregbox(registers[i], i, 0);
	}
}


GUI::~GUI()
{
}

Backend*& GUI::getbackend()
{
	return backend;
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

void GUI::int_update(){
	backend->StartedUpdating();	//call at begining is necessary!
	//TODO...
	PRINTF("int_update() called!\n");
	int pos;
	byte bank;
	char queueIO = 0;		//Remember when a value in the IO-table is changed and a redraw is necessary 
	int queueSPregs = 0;	//Remember whether one of the Spezial register boxes was changed and a redraw is necessary
	while (backend->GetNextChangedCell(pos, bank)) {
		printf("Byte %02x in Bank %d changed!", pos, bank);	
		int value = getbackend()->GetByte(pos, bank);
		switch (pos) {
		case 0x01: if (bank) {
				setregbox(registers[6], 6, value);
				setregbox(registers[7], 7, value);
				queueSPregs |= 64 + 32;
			}
			break;
		case 0x02:
			setregbox(registers[3], 3, value);
			setregbox(registers[5], 5, value + ((getbackend()->GetByte(pos, bank) << 8)));
			queueSPregs |= 16 + 4;
			break;
		case 0x03:
			setregbox(registers[1], 1, value);
			setregbox(registers[2], 2, value);
			queueSPregs |= 2 + 1;
			break;
		case 0x05:if (bank) {
				setIOcell(IO_table->getstyle(), 1, value);
			}
			else {
				setIOcell(IO_table->getstyle(), 2, value);
			}
			queueIO = 1;
			break;
		case 0x06:if (bank) {
				setIOcell(IO_table->getstyle(), 4, value);
			}
			else {
				setIOcell(IO_table->getstyle(), 5, value);
			}
			queueIO = 1;
			break;
		case 0x0A:
			setregbox(registers[4], 4, value);
			setregbox(registers[5], 5, (getbackend()->GetByte(pos, bank) + (value << 8)));
			queueSPregs |= 16 + 8;
			break;
		case 0x0B:
			setregbox(registers[8], 8, value);
			setregbox(registers[9], 9, value);
			queueSPregs |= 128 + 256;
			break;
		}
		setMEMcell(Mem_table->getstyle(), pos, bank, value);
	}
	if (queueIO) IO_table->redraw();
	for (int i = 0; i < BOXES; i++) {
		if (queueSPregs&(1 << i)) {
			registers[i + 1]->redraw();
		}
	}
	//redraw Register W
	setregbox(registers[0], 0, getbackend()->GetRegW());
	registers[0]->redraw();
}

//#######################################################################################
//#######################################################################################
//######################	O	V	E	R	R	I	D	E	S	#########################
//#######################################################################################
//#######################################################################################

void GUI::resize(int x, int y, int w, int h){
	/*//TODO: fonsize ändern
	if (w < 750)__font_size_table_ = 12;
	else if (w < 1000)__font_size_table_ = 14;
	else __font_size_table_ = 16;
	*/
	Fl_Double_Window::resize(x, y, w, h);
	menubar->resize(X_MENUBAR, Y_MENUBAR, W_MENUBAR, H_MENUBAR);
	Mem_table->resize(X_MEM_TAB, Y_MEM_TAB, W_MEM_TAB, H_MEM_TAB);
	IO_table->resize(X_IO_TAB, Y_IO_TAB, W_IO_TAB, H_IO_TAB);
	CODE_table->resize(X_CODE_TAB, Y_CODE_TAB, W_CODE_TAB, H_CODE_TAB);
	int newy = 40;
	for (int i = 0; i < BOXES; i++) {
		registers[i]->resize(X_MEM_TAB + W_MEM_TAB + 40, newy, w / 5, 60);
		newy += 30;
		if (i == 2 || i == 7) newy += 30;
	}
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
	PRINTF1("Chosen File: '%s'", chooser->filename());
	if(!backend->LoadProgramm((char*)chooser->filename()))
		fl_alert(backend->GetErrorMSG());
	else {
		freetablestyle(CODE_table->getstyle(), CellsCODE);
		size_t lines = 1;
		ASM_TEXT* code = backend->GetProgrammText(lines);
		CODE_table->getstyle() = setstyle_Code(lines, code);	//TODO: entsprechende Funktion:freesytle_Code(tablestyle*& toFree); wird benötigt und muss durch free(CODE_table->getstyle()); ersetzt werden...
		backend->FreeProgrammText(code);
		CODE_table->rows(lines);
	}
}

//callback that is called when the window is closed
//used to perform actions before programm is shut down  
void GUI::window_cb(Fl_Widget*, void*) {
	exit(0);
}