#include "trackpane.h"

#include "tabsong.h"

#include <qpainter.h>
#include <qdrawutil.h>
#include <qstyle.h>

TrackPane::TrackPane(TabSong *s, int hh, int rh, QWidget *parent, const char *name):
	QGridView(parent, name)
{
	song = s;

//	setTableFlags(Tbl_autoHScrollBar | Tbl_smoothScrolling);
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
	setNumCols(song->maxLen());
	update();
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
	if ((row != 0) && (song->t.at(row - 1)->barStatus(col)))
		style().drawPrimitive(QStyle::PE_ButtonBevel, p, QRect(0, 0, cellWidth(), cellWidth()), colorGroup());
}

void TrackPane::mousePressEvent(QMouseEvent *e)
{
	if (e->button() == LeftButton) {
		int barnum = columnAt(e->pos().x());
		int tracknum = rowAt(e->pos().y()) - 1;

		if (tracknum >= song->t.count())
			return;
		emit newTrackSelected(song->t.at(tracknum));
		emit newBarSelected(barnum);
	}
}
