#ifndef TRACKPANE_H
#define TRACKPANE_H

#include <q3table.h>
//Added by qt3to4:
#include <QMouseEvent>

class TabSong;
class TabTrack;

class TrackPane: public Q3ScrollView {
	Q_OBJECT

public:
	TrackPane(TabSong *s, int hh, int rh, QWidget *parent = 0, const char *name = 0);
	void updateList();

public slots:
	void repaintTrack(TabTrack *);
	void repaintCurrentTrack();
	void syncVerticalScroll(int, int);

signals:
	void trackSelected(TabTrack *);
	void barSelected(uint);

protected:
	virtual void drawContents(QPainter *, int clipx, int clipy, int clipw, int cliph);
	virtual void mousePressEvent(QMouseEvent *e);

private:
	TabSong *song;
	int headerHeight, cellSide;
};

#endif
