#include "MyTable.h"

#include <math.h> 
#include <FL/fl_draw.H>

MyTable::MyTable(int x, int y, int w, int h, int t, const char *l) : Fl_Table_Row(x, y, w, h, l) {
	cell_bgcolor = FL_WHITE;
	cell_fgcolor = FL_BLACK;
	callback(&event_callback, (void*)this);
	typ = t;
	end();
}
MyTable::~MyTable() {

}


void MyTable::draw_cell(TableContext context, int R, int C, int X, int Y, int W, int H) {
	switch (typ) {
	case 0:draw_cell_MEM(context, R, C, X, Y, W, H); return;
	case 1:draw_cell_IO(context, R, C, X, Y, W, H); return;
	}
}

void MyTable::draw_cell_MEM(TableContext context, int R, int C, int X, int Y, int W, int H) {
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
		PRINTF("TABLE CONTEXT CALLED\n");
		return;

	case CONTEXT_ENDPAGE:
	case CONTEXT_RC_RESIZE:
	case CONTEXT_NONE:
		return;
	}
}

void MyTable::draw_cell_IO(TableContext context, int R, int C, int X, int Y, int W, int H) {
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
	PRINTF1("'%s' callback: ", (label() ? label() : "?"));
	PRINTF5("Row=%d Col=%d Context=%d Event=%d InteractiveResize? %d\n",
		R, C, (int)context, (int)Fl::event(), (int)is_interactive_resize());
}