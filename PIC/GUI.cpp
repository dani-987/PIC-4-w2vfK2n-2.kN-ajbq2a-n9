#include "GUI.h"

//Variables for size-parameters that might be allow to change (TODO) + Getters
int my_font_size = 14;
int cell_width_Mem;
int cell_height_Mem;
int cell_width_SpRegs;
int cell_height_SpRegs;
int* col_width_CODE;
int label_width_button;
int label_height_button;

int& get_font_size() { return my_font_size; }
int& get_cell_width_Mem() { return cell_width_Mem; }
int& get_cell_height_Mem() { return cell_height_Mem; }
int& get_cell_width_SpRegs() { return cell_width_SpRegs; }
int& get_cell_height_SpRegs() { return cell_height_SpRegs; }
int& get_col_width_CODE(int column) { return col_width_CODE[column]; }
int& get_label_width_button() { return label_width_button; }
int& get_label_height_button() { return label_height_button; }

//Set Cell dimensions based on the currently selcted font size
void setsizes() {
	get_dimension(BASE_LABEL_MEM, get_cell_width_Mem(), get_cell_height_Mem());
	get_dimension_small(BASE_LABEL_SPREGS, get_cell_width_SpRegs(), get_cell_height_SpRegs());
	get_dimension(BASE_LABEL_BUTTON, get_label_width_button(), get_label_height_button());
}

//Custome overload for fl_measure: gives the size (in pixels) of an object as printed on screen
void get_dimension(char* tomeasure, int& width, int& height) {
	fl_font(FL_HELVETICA, FONT_SIZE);
	width = 0; height = 0;
	fl_measure(tomeasure, width, height);
}

void get_dimension_small(char* tomeasure, int& width, int& height) {
	fl_font(FL_HELVETICA, FONT_SIZE_SMALL);
	width = 0; height = 0;
	fl_measure(tomeasure, width, height);
}

void gui_int_update(void * gui){((GUI*)gui)->int_update();}

void gui_int_updateAll(void * gui){((GUI*)gui)->int_updateAll();}

//Position and size of the menubar
#define X_MENUBAR		0
#define Y_MENUBAR		0
#define W_MENUBAR		WINDOW_WIDTH
#define H_MENUBAR		CELL_HEIGHT_MEM
#define MENU_ITEMCOUNT	13

//Position and size of the IO-Table
#define W_IO_TAB		(CELL_WIDTH_MEM*CCMEM+ROW_HEADER_WIDTH_MEM + 5)
#define H_IO_TAB		(CELL_HEIGHT_MEM*RCIO + 5)
#define X_IO_TAB		SIDEMARGINE
#define Y_IO_TAB		(h - (BOTTOMMARGINE+H_IO_TAB))

//Position and size of the MEM-Table
#define X_MEM_TAB		SIDEMARGINE
#define Y_MEM_TAB		TOPMARGINE
#define W_MEM_TAB		W_IO_TAB
#define H_MEM_TAB		(Y_IO_TAB-(Y_MEM_TAB+INTERSPACE))

//Position and size of the Spezial Registers Scroll subwindow
#define X_SPREGS		(X_IO_TAB+W_IO_TAB+INTERSPACE)
#define Y_SPREGS		TOPMARGINE
#define W_SPREGS		(CCSPREGS*CELL_WIDTH_SPREGS+SCROLLBAR_WIDTH)
#define H_SPREGS		(h - (TOPMARGINE+BOTTOMMARGINE))
#define W_SPREGS_TABLE	(CELL_WIDTH_SPREGS*CCSPREGS + 5)
#define H_SPREGS_TABLE	(CELL_HEIGHT_SPREGS*RCSPREGS + 5)
#define W_SPREGS_BOX	W_SPREGS_TABLE
#define H_SPREGS_BOX	CELL_HEIGHT_SPREGS

//Position and size of the CODE-Table
#define X_CODE_TAB		(X_SPREGS+W_SPREGS+INTERSPACE)
#define Y_CODE_TAB		TOPMARGINE
#define W_CODE_TAB		(w-(X_SPREGS+W_SPREGS+INTERSPACE))
#define H_CODE_TAB		(h - (TOPMARGINE+BOTTOMMARGINE+BOX_HEIGHT_BUTTON+INTERSPACE))

//Position and size of the controll buttons
#define H_BUTTON		BOX_HEIGHT_BUTTON
#define W_BUTTON		BOX_WIDTH_BUTTON
#define Y_BUTTON		(Y_CODE_TAB+H_CODE_TAB+INTERSPACE)
#define X_BUTTON(a)		(X_CODE_TAB + INTERSPACE + a * (W_BUTTON + INTERSPACE))
#define BUTTON_OFFSET	20


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

//Structure of the Menubar-Items; When changing also change MENU_ITEMCOUNT!
Fl_Menu_Item menutable_vorlage[] = {
	{ "&Datei", 0, nullptr, 0, FL_SUBMENU },
		{ "&Lade Datei", 0, gui_callbacks::loadFile, 0 },
		{0},
	{ "Optionen", 0, nullptr, 0, FL_SUBMENU },
		{ "Taktrate", 0, nullptr, 0, FL_SUBMENU },
			{ "10 MHz", 0, gui_callbacks::setRate_s1, 0, FL_MENU_RADIO } , 
			{ "4 MHz", 0, gui_callbacks::setRate_s2, 0, FL_MENU_RADIO | FL_MENU_VALUE }, //Default Speed
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
	//int wi = 0, hi = 0;
	//fl_font(FL_HELVETICA, FONT_SIZE);
	//fl_measure("Tris", w, h);
	//Create Backend
	backend = new Backend(this);

	/*//TODO: fontsize ändern <- sollte in resize gemacht werden?
	if (w < 750)my_font_size = 12;
	else if (w < 1000)my_font_size = 14;
	else my_font_size = 16;
	*/

	setsizes();

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
	//Mem_table->selection_color(FL_YELLOW);
	Mem_table->when(FL_WHEN_RELEASE | FL_WHEN_CHANGED);
	Mem_table->table_box(FL_NO_BOX);

	// Configure table rows
	Mem_table->row_header(1);
	Mem_table->row_header_width(ROW_HEADER_WIDTH_MEM);
	Mem_table->row_resize(0);
	Mem_table->rows(RCMEM);
	Mem_table->row_height_all(CELL_HEIGHT_MEM);

	// Configure table columns
	Mem_table->cols(CCMEM);
	Mem_table->col_header(1);
	Mem_table->col_header_height(COL_HEADER_HEIGHT_MEM);
	Mem_table->col_resize(0);
	Mem_table->col_width_all(CELL_WIDTH_MEM);

	//Table for the IO-Registers
	IO_table = new MyTable(X_IO_TAB, Y_IO_TAB, W_IO_TAB, H_IO_TAB, "IO-Registers");
	IO_table->getstyle() = setstyle_IO();
	IO_table->when(FL_WHEN_RELEASE | FL_WHEN_CHANGED);
	IO_table->table_box(FL_NO_BOX);

	//Configure table rows
	IO_table->row_resize(0);
	IO_table->row_header(1);
	IO_table->row_header_width(ROW_HEADER_WIDTH_MEM);
	IO_table->rows(RCIO);
	IO_table->row_height_all(CELL_HEIGHT_MEM);

	//Configure table columns
	IO_table->cols(CCIO);
	IO_table->col_resize(0);
	IO_table->col_width_all(CELL_WIDTH_MEM);

	//Table for the Code
	CODE_table = new MyTable(X_CODE_TAB, Y_CODE_TAB, W_CODE_TAB, H_CODE_TAB, "Code");
	col_width_CODE = (int*)malloc(sizeof(int)*RCCODE);
	ZeroMemory(col_width_CODE, sizeof(int)*RCCODE);
	CODE_table->getstyle() = setstyle_Code(RCCODE, nullptr);
	//CODE_table->selection_color(FL_YELLOW);
	CODE_table->when(FL_WHEN_RELEASE | FL_WHEN_CHANGED);
	CODE_table->table_box(FL_NO_BOX);

	// Configure table rows
	CODE_table->rows(RCCODE);
	CODE_table->row_resize(0);
	CODE_table->row_height_all(CELL_HEIGHT_CODE);

	// Configure table columns
	CODE_table->cols(CCCODE);
	CODE_table->col_header(1);
	CODE_table->col_header_height(COL_HEADER_HEIGHT_CODE);
	CODE_table->col_resize(1);
	//CODE_table->col_width_all(w/15);
	for (int i = 0; i < CCCODE; i++) {
		CODE_table->col_width(i, CELL_WIDTH_CODE(i));
	}

	registers = (Fl_Box**)malloc(sizeof(Fl_Box*) * BOXES);
	regtables = (MyTable**)malloc(sizeof(MyTable*) * TABLES);

	subwin = new Fl_Scroll(X_SPREGS, Y_SPREGS, W_SPREGS , H_SPREGS);
	//Sets up the boxes for the special regsiters
	int newy = 0;
	for (int i = 0; i < BOXES; i++) {
		registers[i] = new Fl_Box(FL_UP_BOX, 0, newy, W_SPREGS_BOX, H_SPREGS_BOX,"");
		newy += H_SPREGS_BOX + (i == 1 || i == 5 || i == 7) ? (H_SPREGS_TABLE) : 0 + INTERSPACE_SMALL;
		setregbox(registers[i], i, 0);
	}

	//sets up the small tables that show bitwise representations of three special registers Their posiitoned like the boxes above
	newy = 0;
	for (int i = 0; i < TABLES; i++) {
		MyTable* temptable = new MyTable(0, newy, W_SPREGS_TABLE, H_SPREGS_TABLE);
		newy += H_SPREGS_TABLE + (i == 0) ? 4 : 1 * (H_SPREGS_BOX + INTERSPACE_SMALL);

		temptable->getstyle() = setstyle_SpRegs(i);
		temptable->when();
		temptable->table_box(FL_NO_BOX);

		//Configure table rows
		temptable->rows(RCSPREGS);
		temptable->row_resize(0);
		temptable->row_height_all(CELL_HEIGHT_SPREGS);

		//Configure table columns
		temptable->cols(CCSPREGS);
		temptable->col_resize(0);
		temptable->col_width_all(CELL_WIDTH_SPREGS);
		regtables[i] = temptable;
	}
	subwin->end();
	subwin->scroll_to(0, 0);

	Start = new Fl_Button(X_BUTTON(0), Y_BUTTON, W_BUTTON, H_BUTTON, "Start");
	Start->callback(gui_callbacks::Start, this);

	Stop = new Fl_Button(X_BUTTON(1), Y_BUTTON, W_BUTTON, H_BUTTON, "Stop");
	Stop->callback(gui_callbacks::Stop, this);

	Step = new Fl_Button(X_BUTTON(2), Y_BUTTON, W_BUTTON, H_BUTTON, "Step");
	Step->callback(gui_callbacks::Step, this);

	Reset = new Fl_Button(X_BUTTON(3), Y_BUTTON, W_BUTTON, H_BUTTON, "Reset");
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


void GUI::int_updateAll()
{
	backend->StartedUpdating();
	//PRINTF("int_updateAll() called!\n");
	int value;
	//First update everything on bank on
	//beginning with the spezial registers that, so of them having additional representations that need to be updated
	for (int i = 1; i < 0x0C; i++) {
		value = getbackend()->GetByte(i, 0); 
		switch (i) {
			//update the Special begister boxes and the mirrored entries in bank 1 in teh MEM-Table
		case 0x02:	//PCL
			setregbox(registers[2], 2, value);
			setregbox(registers[4], 4, getbackend()->GetPC());
			setMEMcell(Mem_table->getstyle(), i, 1, value);
			break;
		case 0x03:	//Status
			setregbox(registers[1], 1, value);
			setregtable(regtables[0]->getstyle(), value);
			setMEMcell(Mem_table->getstyle(), i, 1, value);
			break;
		case 0x04:	//FSR
			setMEMcell(Mem_table->getstyle(), i, 1, value);
			break;
		case 0x05:	//RA
			setIOcell(IO_table->getstyle(), 2, value);
			break;
		case 0x06:	//RB
			setIOcell(IO_table->getstyle(), 5, value);
			break;
		case 0x0A:	//PCLATH
			setregbox(registers[3], 3, value);
			setMEMcell(Mem_table->getstyle(), i, 1, value);
			break;
		case 0x0B:	//INTCON
			setregbox(registers[6], 6, value);
			setregtable(regtables[2]->getstyle(), value);
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
		case 0x01:	//Option
			setregbox(registers[5], 5, value);
			setregtable(regtables[1]->getstyle(), value);
			break;
		case 0x05:	//TRISA
			setIOcell(IO_table->getstyle(), 1, value);
			break;
		case 0x06:	//TRISB
			setIOcell(IO_table->getstyle(), 4, value);
			break;
		case 0x08: case 0x07: case 0x09: break;	//No other areas need to be updated, this is only here to jump over the continue below
		
		default: continue;	//as this loop only updates non mapped registers, all other registers can be ignored
		}
		setMEMcell(Mem_table->getstyle(), i, 1, value);
	}
	setregbox(registers[0], 0, getbackend()->GetRegW());

	//Finally, redraw everything, including Reg W
	IO_table->redraw();
	for (int i = 0; i < BOXES; i++) {	
		registers[i]->redraw();
	}
	for (int i = 0; i < 3; i++) {
		regtables[i]->redraw();
	}
	Mem_table->redraw();
	CODE_table->redraw();
	CODE_table->setcodeline(getbackend()->GetAktualCodePosition());
}

void GUI::int_update(){

	backend->StartedUpdating();	//call at begining is necessary!
	//PRINTF("int_update() called!\n");
	int pos;
	byte bank;
	char queueIO = 0;		//Remember when a value in the IO-table is changed and a redraw is necessary 
	int queueSpRegs = 0;	//Remember whether one of the Spezial register boxes was changed and a redraw is necessary
	while (backend->GetNextChangedCell(pos, bank)) {
		//printf("Byte %02x in Bank %d changed!", pos, bank);	
		int value = getbackend()->GetByte(pos, bank);
		switch (pos) {
		case 0x01: if (bank) {	//Option
			setregbox(registers[5], 5, value);
			setregtable(regtables[1]->getstyle(), value);
				queueSpRegs |= 128 + 16;
			}
			break;
		case 0x02:	//PCL
			setregbox(registers[2], 2, value);
			setregbox(registers[4], 4, getbackend()->GetPC());
			setMEMcell(Mem_table->getstyle(), pos, 1, value);
			queueSpRegs |= 8 + 2;
			break;
		case 0x03:	//Status
			setregbox(registers[1], 1, value);
			setregtable(regtables[0]->getstyle(), value);
			setMEMcell(Mem_table->getstyle(), pos, 1, value);
			queueSpRegs |= 64 + 1;
			break;
		case 0x04:	//FSR
			setMEMcell(Mem_table->getstyle(), pos, 1, value);
			break;
		case 0x05:if (bank) {	//RA
				setIOcell(IO_table->getstyle(), 1, value);
			}
			else {	//TRISA
				setIOcell(IO_table->getstyle(), 2, value);
			}
			queueIO = 1;
			break;
		case 0x06:if (bank) {	//RB
				setIOcell(IO_table->getstyle(), 4, value);
			}
			else {	//TRISB
				setIOcell(IO_table->getstyle(), 5, value);
			}
			queueIO = 1;
			break;
		case 0x0A:	//PCLATH
			setregbox(registers[3], 3, value);
			setMEMcell(Mem_table->getstyle(), pos, 1, value);
			queueSpRegs |= 4;
			break;
		case 0x0B:	//INTCON
			setregbox(registers[6], 6, value);
			setregtable(regtables[2]->getstyle(), value);
			setMEMcell(Mem_table->getstyle(), pos, 1, value);
			queueSpRegs |= 32 + 256;
			break;
		}
		setMEMcell(Mem_table->getstyle(), pos, bank, value);
	}
	setregbox(registers[0], 0, getbackend()->GetRegW());

	if (queueIO) IO_table->redraw();
	for (int i = 0; i < BOXES; i++) {
		if (queueSpRegs&(1 << (i - 1) || i == 0)) {
			registers[i]->redraw();
		}
	}
	for (int i = BOXES; i < BOXES + 3; i++) {
		if (queueSpRegs&(1 << (i - 1))) {
			regtables[i - BOXES]->redraw();
		}
	}
	Mem_table->redraw();
	CODE_table->setcodeline(getbackend()->GetAktualCodePosition());
	CODE_table->redraw();
}

//#######################################################################################
//#######################################################################################
//######################	O	V	E	R	R	I	D	E	S	#########################
//#######################################################################################
//#######################################################################################

void GUI::resize(int x, int y, int w, int h){
	/*//TODO: fonsize ändern
	if (w < 750)my_font_size = 12;
	else if (w < 1000)my_font_size = 14;
	else my_font_size = 16;
	*/
	Fl_Double_Window::resize(x, y, w, h);
	menubar->resize(X_MENUBAR, Y_MENUBAR, W_MENUBAR, H_MENUBAR);
	Mem_table->resize(X_MEM_TAB, Y_MEM_TAB, W_MEM_TAB, H_MEM_TAB);
	IO_table->resize(X_IO_TAB, Y_IO_TAB, W_IO_TAB, H_IO_TAB);
	CODE_table->resize(X_CODE_TAB, Y_CODE_TAB, W_CODE_TAB, H_CODE_TAB);
	int newy = 0, tcount = 0, bcount = 0;
	for (int i = 0; i < BOXES + 3; i++) {
		if (i == 2 || i == 7 || i == 9) {
			regtables[tcount]->resize(0, newy, W_SPREGS_TABLE, H_SPREGS_TABLE);
			newy += H_SPREGS_TABLE + (tcount == 0) ? 4 : 1 * (H_SPREGS_BOX + INTERSPACE_SMALL);
			tcount++;
		}
		else {
			registers[bcount]->resize(0, newy, W_SPREGS_BOX, H_SPREGS_BOX);
			newy += H_SPREGS_BOX;
			newy += H_SPREGS_BOX + (bcount == 1 || bcount == 5 || bcount == 7) ? (H_SPREGS_TABLE) : 0 + INTERSPACE_SMALL;
			bcount++;
		}
	}
	subwin->scroll_to(0, 0);
	//int subx = subwin->xposition(), suby = subwin->yposition();
	subwin->resize(X_SPREGS, Y_SPREGS, W_SPREGS, H_SPREGS);
	Start->resize(X_BUTTON(0), Y_BUTTON, W_BUTTON, H_BUTTON);
	Stop->resize(X_BUTTON(1), Y_BUTTON, W_BUTTON, H_BUTTON);
	Step->resize(X_BUTTON(2), Y_BUTTON, W_BUTTON, H_BUTTON);
	Reset->resize(X_BUTTON(3), Y_BUTTON, W_BUTTON, H_BUTTON);
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
		CODE_table->getstyle() = setstyle_Code(lines, code);
		backend->FreeProgrammText(code);
		CODE_table->rows(lines);
		for (int i = 0; i < CCCODE; i++) {
			CODE_table->col_width(i, CELL_WIDTH_CODE(i));
		}
		int_updateAll();
	}
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
	//printf("Set speed to Speed%d\n", freq);
	getbackend()->SetCommandSpeed(freq);
}
void GUI::callback_watchdog() {
	//printf("%s watchdog\n", (getbackend()->IsWatchdogEnabled())?("Disabled"):("Enabled"));
	if (getbackend()->IsWatchdogEnabled()) {
		getbackend()->DisableWatchdog();
	}
	else {
		getbackend()->EnableWatchdog();
	}
}