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
#define MENU_ITEMCOUNT	13

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
#define H_CODE_TAB		(h - 150)
#define X_CODE_TAB		(w - (W_CODE_TAB + 10))
#define Y_CODE_TAB		40

//Position and size of the Spezial Registers Box
#define X_SPEC_REGS		(X_IO_TAB+W_IO_TAB+40)
#define Y_SPEC_REGS		40
#define W_SPEC_REGS		(w/5)
#define H_SPEC_REGS		60

//Position and size of the controll buttons

#define H_CONT_BUTT		40
#define W_CONT_BUTT		(w/15)
#define Y_CONT_BUTT		(h-(10+H_CONT_BUTT))
#define X_CONT_BUTT		(X_CODE_TAB + 20)
#define BUTT_OFFSET		(W_CONT_BUTT + 20)


namespace gui_callbacks {
	void loadFile(Fl_Widget *, void *);
	void Start(Fl_Widget *, void *);
	void Stop(Fl_Widget *, void *);
	void Step(Fl_Widget *, void *);
	void Reset(Fl_Widget *, void *);
	void setRate_s1(Fl_Widget *, void *);
	void setRate_s2(Fl_Widget *, void *);
	void setRate_s3(Fl_Widget *, void *);
	void setRate_s4(Fl_Widget *, void *);
	void setWatchdog(Fl_Widget *, void *);
	void donothing(Fl_Widget *, void *);
}
Fl_Menu_Item menutable_vorlage[] = {
	{ "&Datei", 0, nullptr, 0, FL_SUBMENU },
		{ "&Lade Datei", 0, gui_callbacks::loadFile, 0 },
		{0},
	{ "Optionen", 0, nullptr, 0, FL_SUBMENU },
		{ "Taktrate", 0, nullptr, 0, FL_SUBMENU },
			{ "10 MHz", 0, gui_callbacks::setRate_s1, 0, FL_MENU_RADIO } , //Default Speed
			{ "4 MHz", 0, gui_callbacks::setRate_s2, 0, FL_MENU_RADIO | FL_MENU_VALUE },
			{ "400 kHz", 0, gui_callbacks::setRate_s3, 0, FL_MENU_RADIO },
			{ "40 kHz", 0, gui_callbacks::setRate_s4, 0, FL_MENU_RADIO },
			{0},
		{ "Watchdog", 0, gui_callbacks::setWatchdog, 0, FL_MENU_TOGGLE },
		{0},
	{0}
};

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
	//Structure of the Menu. Components of each item: label, shortcut, callback, value, Flag (Optinal)
	menutable = (Fl_Menu_Item*)malloc(sizeof(Fl_Menu_Item) * MENU_ITEMCOUNT);
	memcpy(menutable, menutable_vorlage, sizeof(menutable_vorlage));
	for (int i = 0; i < MENU_ITEMCOUNT; i++) {
		if (menutable[i].text != nullptr) {
			menutable[i].user_data_ = this;
		}
	}

	menubar = new Fl_Menu_Bar(X_MENUBAR, Y_MENUBAR, W_MENUBAR, H_MENUBAR);
	menubar->menu(menutable);
	//menubar->add("&Datei/&Lade Datei", nullptr, gui_callbacks::loadFile, this);

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
	Fl_Box* tempreg;
	for (int i = 0; i < BOXES; i++) {
		tempreg = new Fl_Box(FL_FLAT_BOX, w, h, 0, 0, "");
		registers[i] = tempreg;
		setregbox(registers[i], i, 0);
	}

	Start = new Fl_Button(X_CONT_BUTT, Y_CONT_BUTT, W_CONT_BUTT, H_CONT_BUTT, "Start");
	Start->callback(gui_callbacks::Start, this);

	Stop = new Fl_Button(X_CONT_BUTT + BUTT_OFFSET, Y_CONT_BUTT, W_CONT_BUTT, H_CONT_BUTT, "Stop");
	Stop->callback(gui_callbacks::Stop, this);

	Step = new Fl_Button(X_CONT_BUTT + (BUTT_OFFSET * 2), Y_CONT_BUTT + (BUTT_OFFSET * 2), W_CONT_BUTT, H_CONT_BUTT, "Step");
	Step->callback(gui_callbacks::Step, this);

	Reset = new Fl_Button(X_CONT_BUTT + (BUTT_OFFSET * 3), Y_CONT_BUTT + (BUTT_OFFSET * 3), W_CONT_BUTT, H_CONT_BUTT, "Reset");
	Reset->callback(gui_callbacks::Reset, this);

	int_updateAll();
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


//Todo: Update PC properly
void GUI::int_updateAll()
{
	backend->StartedUpdating();
	PRINTF("int_updateAll() called!\n");
	int value;
	//First update everything on bank on
	//beginning with the spezial registers that, so of them having additional representations that need to be updated
	for (int i = 1; i < 0x0C; i++) {
		value = getbackend()->GetByte(i, 0); 
		switch (i) {
			//update the Special begister boxes and the mirrored entries in bank 1 in teh MEM-Table
		case 0x02:
			setregbox(registers[3], 3, value);
			setregbox(registers[5], 5, value + ((getbackend()->GetByte(i, 0) << 8)));
			setMEMcell(Mem_table->getstyle(), i, 1, value);
			break;
		case 0x03:
			setregbox(registers[1], 1, value);
			setregbox(registers[2], 2, value);
			setMEMcell(Mem_table->getstyle(), i, 1, value);
			break;
		case 0x04:
			setMEMcell(Mem_table->getstyle(), i, 1, value);
			break;
		case 0x05:
			setIOcell(IO_table->getstyle(), 2, value);
			break;
		case 0x06:
			setIOcell(IO_table->getstyle(), 5, value);
			break;
		case 0x0A:
			setregbox(registers[4], 4, value);
			setMEMcell(Mem_table->getstyle(), i, 1, value);
			break;
		case 0x0B:
			setregbox(registers[8], 8, value);
			setregbox(registers[9], 9, value);
			setMEMcell(Mem_table->getstyle(), i, 1, value);
			break;
		}
		setMEMcell(Mem_table->getstyle(), i, 0, value);
	}

	//Update the rest of the normal Ram Cells
	for (int i = 0x0C; i < 0x50; i++) {
		setMEMcell(Mem_table->getstyle(), i, 0, getbackend()->GetByte(i, 0));
	}
	
	//Update bank 1:
	for (int i = 1; i < 0x0A; i++) {
		value = getbackend()->GetByte(i, 1);
		switch (i) {
		case 0x01:
			setregbox(registers[6], 6, value);
			setregbox(registers[7], 7, value);
			break;
		case 0x05:
			setIOcell(IO_table->getstyle(), 1, value);
			break;
		case 0x06:
			setIOcell(IO_table->getstyle(), 4, value);
			break;
		case 0x08: case 0x09: break;	//No other areas need to be updated, this is only here to jump over the continue below
		
		default: continue;	//as this loop only updates non mapped registers, all other registers can be ignored
		}
		setMEMcell(Mem_table->getstyle(), i, 1, value);
	}

	//Finally, redraw everything, including Reg W
	IO_table->redraw();
	for (int i = 0; i < BOXES; i++) {	
		registers[i]->redraw();
	}
	Mem_table->redraw();
	setregbox(registers[0], 0, getbackend()->GetRegW());
	registers[0]->redraw();
	CODE_table->redraw();
}

void GUI::int_update(){
	backend->StartedUpdating();	//call at begining is necessary!
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
			setMEMcell(Mem_table->getstyle(), pos, 1, value);
			queueSPregs |= 16 + 4;
			break;
		case 0x03:
			setregbox(registers[1], 1, value);
			setregbox(registers[2], 2, value);
			setMEMcell(Mem_table->getstyle(), pos, 1, value);
			queueSPregs |= 2 + 1;
			break;
		case 0x04:
			setMEMcell(Mem_table->getstyle(), pos, 1, value);
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
			setMEMcell(Mem_table->getstyle(), pos, 1, value);
			queueSPregs |= 16 + 8;
			break;
		case 0x0B:
			setregbox(registers[8], 8, value);
			setregbox(registers[9], 9, value);
			setMEMcell(Mem_table->getstyle(), pos, 1, value);
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
	Mem_table->redraw();
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
	int newy = 0;
	for (int i = 0; i < BOXES; i++) {
		registers[i]->resize(X_SPEC_REGS, Y_SPEC_REGS + newy, W_SPEC_REGS, H_SPEC_REGS);
		newy += H_SPEC_REGS + 10;
		if (i == 2 || i == 7) newy += 30;
	}
	Start->resize(X_CONT_BUTT, Y_CONT_BUTT, W_CONT_BUTT, H_CONT_BUTT);
	Stop->resize(X_CONT_BUTT + BUTT_OFFSET, Y_CONT_BUTT, W_CONT_BUTT, H_CONT_BUTT);
	Step->resize(X_CONT_BUTT + (BUTT_OFFSET * 2), Y_CONT_BUTT, W_CONT_BUTT, H_CONT_BUTT);
	Reset->resize(X_CONT_BUTT + (BUTT_OFFSET * 3), Y_CONT_BUTT, W_CONT_BUTT, H_CONT_BUTT);
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
void gui_callbacks::Start(Fl_Widget *w, void *gui) {
	((GUI*)gui)->callback_start();
}
void gui_callbacks::Stop(Fl_Widget *w, void *gui) {
	((GUI*)gui)->callback_stop();
}
void gui_callbacks::Step(Fl_Widget *w, void *gui) {
	((GUI*)gui)->callback_step();
}
void gui_callbacks::Reset(Fl_Widget *w, void *gui) {
	((GUI*)gui)->callback_reset();
}

void gui_callbacks::setRate_s1(Fl_Widget *w, void *gui) {
	((GUI*)gui)->callback_settact(4);
}

void gui_callbacks::setRate_s2(Fl_Widget *w, void *gui) {
	((GUI*)gui)->callback_settact(UC_STANDARD_SPEED);
}

void gui_callbacks::setRate_s3(Fl_Widget *w, void *gui) {
	((GUI*)gui)->callback_settact(100);
}

void gui_callbacks::setRate_s4(Fl_Widget *w, void *gui) {
	((GUI*)gui)->callback_settact(1000);
}

void gui_callbacks::setWatchdog(Fl_Widget *w, void *gui) {
	((GUI*)gui)->callback_watchdog();
}

void gui_callbacks::donothing(Fl_Widget *, void *){}

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
	int_updateAll();
}

//callback that is called when the window is closed
//used to perform actions before programm is shut down  
void GUI::window_cb(Fl_Widget*, void*) {
	exit(0);
}

void GUI::callback_start(){
	getbackend()->Start();
}

void GUI::callback_stop() {
	getbackend()->Stop();
}

void GUI::callback_step() {
	getbackend()->Step();
}

void GUI::callback_reset() {
	getbackend()->Reset();
}

void GUI::callback_settact(int freq) {
	printf("Set speed to Speed%d\n", freq);
	getbackend()->SetCommandSpeed(freq);
}
void GUI::callback_watchdog() {
	printf("%s watchdog\n", (getbackend()->IsWatchdogEnabled())?("Disabled"):("Enabled"));
	if (getbackend()->IsWatchdogEnabled()) {
		getbackend()->DisableWatchdog();
	}
	else {
		getbackend()->EnableWatchdog();
	}
}