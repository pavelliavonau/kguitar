#ifndef TRACKPANE_H
#define TRACKPANE_H

#include <qtableview.h>

class TabSong;
class TabTrack;

class TrackPane: public QTableView {
	Q_OBJECT

public:
	TrackPane(TabSong *s, int hh, int rh, QWidget *parent = 0, const char *name = 0);
	~TrackPane();
	void updateList();

signals:
	void newTrackSelected(TabTrack *);
	void newBarSelected(uint);

protected:
	virtual int cellHeight(int n);
	virtual void paintCell(QPainter *, int row, int col);
	virtual void mousePressEvent(QMouseEvent *e);

private:
	TabSong *song;
	int rowh, headh;
};

#endif
