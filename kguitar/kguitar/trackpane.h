#ifndef TRACKPANE_H
#define TRACKPANE_H

#include <qtable.h>

class TabSong;
class TabTrack;

class TrackPane: public QScrollView {
	Q_OBJECT

public:
	TrackPane(TabSong *s, int hh, int rh, QWidget *parent = 0, const char *name = 0);
	void updateList();

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
