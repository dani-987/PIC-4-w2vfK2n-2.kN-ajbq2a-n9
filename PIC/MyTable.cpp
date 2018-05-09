#include "MyTable.h"

//define debug-level
#define DEBUG_LVL_NONE	0
#define DEBUG_LVL_NORMAL	1
#define DEBUG_LVL_MUCH	2
#define DEBUG_LVL_ALL	3
VARDEF(int, DEBUG___LVL, DEBUG_LVL_NORMAL);


#define COLHEADERPOS(C) (C+((row_header())?(1):(0)))
#define ROWHEADERPOS(R) ((R+((col_header())?(1):(0)))*(cols()+1))
#define CELLPOS(R,C)	((R+((col_header())?(1):(0)))*(cols()+((row_header())?(1):(0)))+C+((row_header())?(1):(0)))


MyTable::MyTable(int x, int y, int w, int h,  const char *l) : Fl_Table_Row(x, y, w, h, l) {
	codeline = -1;
	end();
}
MyTable::~MyTable() {

}

tablestyle *& MyTable::getstyle()
{
	return mystyle;
}


void MyTable::draw_cell(TableContext context, int R, int C, int X, int Y, int W, int H) {

	int position = 0;
	switch (context)
	{
	case CONTEXT_STARTPAGE:
		DOIF(DEBUG___LVL >= DEBUG_LVL_MUCH)PRINTF6("Called Startpage Context (R: %d C: %d X: %d Y: %d W: %d H: %d)\n",R,C,X,Y,W,H);
		fl_font(mystyle[position].font, mystyle[position].fontsize);
		return;

	case CONTEXT_COL_HEADER:
		position = COLHEADERPOS(C);
		DOIF(DEBUG___LVL >= DEBUG_LVL_MUCH)PRINTF4("Called Col_Header Context with: R=%d, C=%d, position=%d, label=%s\n",R,C, position, mystyle[position].label);

		fl_push_clip(X, Y, W, H);
		{
			// BG COLOR
			fl_color(mystyle[position].backgroundcolor);
			fl_rectf(X, Y, W, H);

			fl_draw_box(mystyle[position].boxtyp, X, Y, W, H, mystyle[position].bordercolor);
			fl_color(mystyle[position].textcolor);
			fl_draw(mystyle[position].label, X, Y, W, H, mystyle[position].align);

			fl_color(mystyle[position].bordercolor);
			fl_rect(X, Y, W, H);
		}
		fl_pop_clip();
		return;

	case CONTEXT_ROW_HEADER:
		//sprintf_s(s, "%d%d", (int)floor(R / 2), (R % 2) ? 8 : 0);
		position = ROWHEADERPOS(R);
		DOIF(DEBUG___LVL >= DEBUG_LVL_MUCH)PRINTF4("Called ROW_Header Context with: R=%d, C=%d, position=%d label=%s\n", R, C, position, mystyle[position].label);

		fl_push_clip(X, Y, W, H);
		{
			// BG COLOR
			fl_color(mystyle[position].backgroundcolor);
			fl_rectf(X, Y, W, H);

			fl_draw_box(mystyle[position].boxtyp, X, Y, W, H, mystyle[position].bordercolor);
			fl_color(mystyle[position].textcolor);
			fl_draw(mystyle[position].label, X, Y, W, H, mystyle[position].align);

			fl_color(mystyle[position].bordercolor);
			fl_rect(X, Y, W, H);
		}
		fl_pop_clip();
		return;

	case CONTEXT_CELL:
	{
		fl_push_clip(X, Y, W, H);
		{
			position = CELLPOS(R, C);
			DOIF(DEBUG___LVL >= DEBUG_LVL_MUCH)PRINTF4("Called Cell Context with: R=%d, C=%d, position=%d, label=%s\n", R, C, position, mystyle[position].label);
			DOIF(DEBUG___LVL >= DEBUG_LVL_ALL)PRINTF2("Headers enabled: rows: %d, columns: %d\n", row_header(), col_header());

			// BG COLOR
			fl_color((getcodeline()==R) ? selection_color() : mystyle[position].backgroundcolor);
			fl_rectf(X, Y, W, H);

			// TEXT
			fl_color(mystyle[position].textcolor);
			fl_draw(mystyle[position].label, X, Y, W, H, mystyle[position].align);

			// BORDER
			fl_color(mystyle[position].bordercolor);
			fl_rect(X, Y, W, H);
		}
		fl_pop_clip();
		return;
	}

	case CONTEXT_TABLE:
		PRINTF("TABLE CONTEXT CALLED\n");
		return;

	case CONTEXT_ENDPAGE:
	case CONTEXT_RC_RESIZE:
	case CONTEXT_NONE:
		return;
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
	DOIF(DEBUG___LVL >= DEBUG_LVL_MUCH)PRINTF1("'%s' callback: ", (label() ? label() : "?"));
	DOIF(DEBUG___LVL >= DEBUG_LVL_ALL)PRINTF5("Row=%d Col=%d Context=%d Event=%d InteractiveResize? %d\n",
		R, C, (int)context, (int)Fl::event(), (int)is_interactive_resize());
}

int MyTable::getcodeline() {
	return codeline;
}
void MyTable::setcodeline(int newline) {
	codeline = newline;
}