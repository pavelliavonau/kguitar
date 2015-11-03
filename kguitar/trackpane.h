#ifndef TRACKPANE_H
#define TRACKPANE_H

#include <QScrollArea>
#include <QMouseEvent>

class TabSong;
class TabTrack;

class TrackPane: public QWidget {
	Q_OBJECT

public:
	TrackPane(TabSong *s, int hh, int rh, QWidget *parent = 0);
	void updateList();
//	virtual QSize sizeHint() const override;

public slots:
	//void repaintTrack(TabTrack *);
	void repaintCurrentTrack();

signals:
	void trackSelected(TabTrack *);
	void barSelected(uint);

protected:
	virtual void paintEvent(QPaintEvent *) override;
	virtual void mousePressEvent(QMouseEvent *e) override;
	//virtual void resizeEvent(QResizeEvent *) override;

	int contentsX(){ return /*viewport()->x()*/0; }
	int contentsY(){ return /*viewport()->y()*/0; }

private:
	TabSong *song;
	int headerHeight, cellSide;
};

#endif
