#ifndef FRETBOARD_H
#define FRETBOARD_H

#include <qwidget.h>
#include "global.h"

class TabTrack;
class QSizePolicy;

class Fretboard: public QWidget {
    Q_OBJECT
public:
    Fretboard(TabTrack *, QWidget *parent = 0, const char *name = 0);

public slots:
	void setTrack(TabTrack *);

signals:
	void buttonClicked(int, int, ButtonState);

protected:
	virtual void paintEvent(QPaintEvent *);
	virtual void mousePressEvent(QMouseEvent *);
	virtual void resizeEvent(QResizeEvent *);
	virtual QSizePolicy sizePolicy();

private:
	void recalculateSizes();

	TabTrack *trk;
	double fr[MAX_FRETS + 1]; // Proper physical fret positions
};

#endif
