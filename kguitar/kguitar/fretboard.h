#ifndef FRETBOARD_H
#define FRETBOARD_H

#include <qwidget.h>
#include "tabtrack.h"
#include "global.h"

class Fretboard: public QWidget {
    Q_OBJECT
public:
    Fretboard(TabTrack *, QWidget *parent = 0, const char *name = 0);
	void setTrack(TabTrack *);

protected:
	virtual void paintEvent(QPaintEvent *);
	virtual void mousePressEvent(QMouseEvent *);
	virtual void resizeEvent(QResizeEvent *);

private:
	void recalculateSizes();

	TabTrack *trk;
	int fr[MAX_FRETS + 1];
};

#endif
