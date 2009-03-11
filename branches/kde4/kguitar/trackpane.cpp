#include "trackpane.h"

#include "tabsong.h"

#include <qpainter.h>
#include <qdrawutil.h>
#include <qstyle.h>
//Added by qt3to4:
#include <QMouseEvent>
#include <QStyleOption>

TrackPane::TrackPane(TabSong *s, int hh, int cs, QWidget *parent, const char *name)
	: Q3ScrollView(parent, name)
{
	song = s;

//	setTableFlags(Tbl_autoHScrollBar | Tbl_smoothScrolling);
	setFrameStyle(Panel | Sunken);
	setBackgroundMode(Qt::PaletteBase);

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
void TrackPane::drawContents(QPainter *p, int clipx, int clipy, int clipw, int /*cliph*/)
{
	int x1 = clipx / cellSide - 1;
	int x2 = (clipx + clipw) / cellSide + 2;

	int py = headerHeight;

	foreach (TabTrack *trk, song->t) {
		int px = x1 * cellSide;
		for (int i = x1; i <= x2; i++) {
			if (trk->barStatus(i)) {
				QStyleOption option;
				option.initFrom(this);
				style()->drawPrimitive(QStyle::PE_FrameButtonBevel, &option, p, this);
				// GREYTODO: port this rect
//				                      QRect(px, py, cellSide, cellSide), colorGroup());
			}
			if (trk->xb == i) {
				QStyleOptionFocusRect option;
				option.initFrom(this);
				style()->drawPrimitive(QStyle::PE_FrameFocusRect, &option, p, this);
				// GREYTODO: port this rect
//				                      QRect(px, py, cellSide, cellSide), colorGroup());
			}
			px += cellSide;
		}
		py += cellSide;
	}

	// Draw header, covering some tracks if necessary
	if (clipy < contentsY() + headerHeight) {
		// GREYTODO: port
//		style()->drawPrimitive(QStyle::PE_HeaderSection, p,
//		                      QRect(x1 * cellSide, contentsY(),
//		                            x2 * cellSide, contentsY() + headerHeight),
//		                      colorGroup());
	}
}

void TrackPane::mousePressEvent(QMouseEvent *e)
{
	if (e->button() == Qt::LeftButton) {
		int barnum = (e->pos().x() + contentsX()) / cellSide;
		uint tracknum = (e->pos().y() + contentsY() - headerHeight) / cellSide;

		if (tracknum >= song->t.count())
			return;

		emit trackSelected(song->t.at(tracknum));
		emit barSelected(barnum);

		update();
	}
}

void TrackPane::repaintTrack(TabTrack *trk)
{
	repaintContents();
}

void TrackPane::repaintCurrentTrack()
{
	repaintContents();
}

void TrackPane::syncVerticalScroll(int /* x */, int y)
{
	scrollBy(0, y - contentsY());
}
