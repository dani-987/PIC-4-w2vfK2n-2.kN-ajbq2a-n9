#include "MyTable.h"
#define COLHEADERPOS(C) (C+1)
#define ROWHEADERPOS(R) (R*(rows()+1))
#define CELLPOS(R,C)	((R+1)*(rows()+1)+C+1)


MyTable::MyTable(int x, int y, int w, int h,  const char *l) : Fl_Table_Row(x, y, w, h, l) {
	callback(&event_callback, (void*)this);
	mystyle = setstyle_MEM();
	end();
}
MyTable::~MyTable() {

}


void MyTable::draw_cell(TableContext context, int R, int C, int X, int Y, int W, int H) {

	int position = 0;
	switch (context)
	{
	case CONTEXT_STARTPAGE:
		fl_font(mystyle[position].font, mystyle[position].fontsize);
		return;

	case CONTEXT_COL_HEADER:
		fl_push_clip(X, Y, W, H);
		{
			position = COLHEADERPOS(C);
			fl_draw_box(mystyle[position].boxtyp, X, Y, W, H, mystyle[position].bordercolor);
			//fl_color(FL_BLACK);
			fl_draw(mystyle[position].label, X, Y, W, H, mystyle[position].align);
		}
		fl_pop_clip();
		return;

	case CONTEXT_ROW_HEADER:
		//sprintf_s(s, "%d%d", (int)floor(R / 2), (R % 2) ? 8 : 0);
		fl_push_clip(X, Y, W, H);
		{
			position = ROWHEADERPOS(R);
			fl_draw_box(mystyle[position].boxtyp, X, Y, W, H, mystyle[position].bordercolor);
			//fl_color(FL_BLACK);
			fl_draw(mystyle[position].label, X, Y, W, H, mystyle[position].align);
		}
		fl_pop_clip();
		return;

	case CONTEXT_CELL:
	{
		//sprintf_s(s, "%d%d", 0, 0);
		fl_push_clip(X, Y, W, H);
		{
			position = CELLPOS(R, C);
			// BG COLOR
			fl_color(is_selected(R, C) ? selection_color() : mystyle[position].backgroundcolor);
			fl_rectf(X, Y, W, H);

			// TEXT
			//fl_color(cell_fgcolor);
			fl_draw(mystyle[position].label, X, Y, W, H, mystyle[position].align);

			// BORDER
			//fl_color(color());
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