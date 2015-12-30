#ifndef TRACKPANE_H
#define TRACKPANE_H

#include <QTableView>
#include <QMouseEvent>

class TabSong;
class TabTrack;

class TrackPane: public QTableView {
	Q_OBJECT

public:
	TrackPane(int rh, QWidget *parent = 0);
	void updateList();

protected:
	virtual void mousePressEvent(QMouseEvent *e) override;
};

#endif
