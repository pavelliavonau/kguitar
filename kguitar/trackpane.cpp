#include "trackpane.h"

#include "data/tabsong.h"

#include <QPainter>
//#include <qdrawutil.h>
#include <QStyle>
#include <QMouseEvent>
#include <QStyleOption>

TrackPane::TrackPane(TabSong *s, int hh, int cs, QWidget *parent)
	: QWidget(parent)
{
	song = s;

//	setTableFlags(Tbl_autoHScrollBar | Tbl_smoothScrolling);
	//setFrameStyle(Panel | Sunken);
	setBackgroundRole(QPalette::Light);

	//	setFocusPolicy(QWidget::StrongFocus);

	cellSide = cs;
	headerHeight = hh;

	updateList();

	show();

	updateGeometry();
}

void TrackPane::updateList()
{
  resize(song->maxLen() * cellSide, song->t.count() * cellSide + headerHeight);
 // update();
  updateGeometry();
}

//QSize TrackPane::sizeHint() const
//{
//  int w = song->maxLen() * cellSide;
//  int h = song->t.count() * cellSide + headerHeight;

//  QWidget* p = static_cast<QWidget*>(parent());
//  int pw = p->width();
//  int ph = p->height();

//  if(pw > w)
//    w = pw;

//  if(ph > h)
//    h = ph;

//  qDebug() << "----------- w " << w;

//  return QSize(w, h);
//}

// Draws that pretty squares for track pane.
void TrackPane::paintEvent(QPaintEvent * e)
{
  QWidget::paintEvent(e);

  QPainter rp(this);

//  rp.setBrush(Qt::SolidPattern);
//  rp.setPen(Qt::black);
//  rp.drawRect(0,0,100,100);
  QPainter* p = &rp;
  QRect clip = e->rect();
  int clipx = clip.x();
  int clipy = clip.y();
  int clipw = clip.width();

  int x1 = clipx / cellSide - 1;
  int x2 = (clipx + clipw) / cellSide + 2;

  int py = headerHeight;

  foreach (TabTrack *trk, song->t) {
          int px = x1 * cellSide;
          for (int i = x1; i <= x2; i++) {
                  if (trk->barStatus(i)) {
                          QStyleOption option;
                          option.initFrom(this);
                          option.rect = QRect(px, py, cellSide, cellSide);
                          style()->drawPrimitive(QStyle::PE_FrameButtonBevel, &option, p, this);
                          p->drawRect(option.rect);
                  }
                  if (trk->xb == i) {
                          QStyleOptionFocusRect option;
                          option.initFrom(this);
                          option.rect = QRect(px, py, cellSide, cellSide);
                          style()->drawPrimitive(QStyle::PE_FrameFocusRect, &option, p, this);
                          p->drawRect(option.rect);
                  }
                  px += cellSide;
          }
          py += cellSide;
  }

  // Draw header, covering some tracks if necessary
  if (clipy < contentsY() + headerHeight) {
          QStyleOptionHeader option;
          option.initFrom(this);
          option.rect = QRect(x1 * cellSide, contentsY(),
                              x2 * cellSide, contentsY() + headerHeight);
          style()->drawControl(QStyle::CE_HeaderSection, &option, p, this);
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

//void TrackPane::resizeEvent(QResizeEvent *)
//{
//  updateGeometry();
//}

//void TrackPane::repaintTrack(TabTrack *trk)
//{
//	Q_UNUSED(trk);
//	//repaintContents();
//	repaint();
//}

void TrackPane::repaintCurrentTrack()
{
	repaint();
	//repaintContents();
}
