#include "trackpane.h"

#include "tabsong.h"

#include <qpainter.h>
#include <qdrawutil.h>
#include <qstyle.h>

TrackPane::TrackPane(TabSong *s, int hh, int cs, QWidget *parent, const char *name)
	: QScrollView(parent, name)
{
	song = s;

//	setTableFlags(Tbl_autoHScrollBar | Tbl_smoothScrolling);
	setFrameStyle(Panel | Sunken);
	setBackgroundMode(PaletteBase);

	//	setFocusPolicy(QWidget::StrongFocus);

	cellSide = cs;
	headerHeight = hh;

	updateList();

	show();
}

void TrackPane::updateList()
{
	resizeContents(song->maxLen() * cellSide, song->t.count() * cellSide + headerHeight);
	update();
}

// Draws that pretty squares for track pane.
void TrackPane::drawContents(QPainter *p, int clipx, int /*clipy*/, int clipw, int /*cliph*/)
{
	int x1 = clipx / cellSide - 1;
	int x2 = (clipx + clipw) / cellSide + 1;

	int py = headerHeight;

	for (TabTrack *trk = song->t.first(); trk; trk = song->t.next()) {
		int px = x1 * cellSide;
		for (int i = x1; i <= x2; i++) {
			if (trk->barStatus(i))
				style().drawPrimitive(QStyle::PE_ButtonBevel, p,
				                      QRect(px, py, cellSide, cellSide), colorGroup());
			px += cellSide;
		}
		py += cellSide;
	}
}

void TrackPane::mousePressEvent(QMouseEvent *e)
{
	if (e->button() == LeftButton) {
		int barnum = e->pos().x() / cellSide;
		int tracknum = (e->pos().y() - headerHeight) / cellSide;

		if (tracknum >= song->t.count())
			return;
		emit trackChanged(song->t.at(tracknum));
		emit newBarSelected(barnum);
	}
}
