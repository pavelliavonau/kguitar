#include "trackpane.h"

#include "tabsong.h"

#include <qpainter.h>
#include <qdrawutil.h>

TrackPane::TrackPane(TabSong *s, int hh, int rh, QWidget *parent = 0, const char *name = 0):
	QTableView(parent, name)
{
	song = s;

	setTableFlags(Tbl_autoHScrollBar | Tbl_smoothScrolling);
	setFrameStyle(Panel | Sunken);
	setBackgroundMode(PaletteBase);

	//	setFocusPolicy(QWidget::StrongFocus);

	rowh = rh;
	headh = hh;

	setCellWidth(rh);

	updateList();

	show();
}

void TrackPane::updateList()
{
	setNumRows(song->t.count() + 1); // plus 1 header row
	setNumCols(20);
}

TrackPane::~TrackPane()
{
}

int TrackPane::cellHeight(int n)
{
	if (n == 0)
		return headh;
	else
		return rowh;
}

// Draws that pretty squares for track pane. Row 0 corresponds to
// header, all other rows are for track squares.
void TrackPane::paintCell(QPainter *p, int row, int col)
{
	if (row != 0) 
		qDrawWinButton(p, 0, 0, cellWidth(), cellWidth(), colorGroup(), FALSE);
}
