#ifndef FRETBOARD_H
#define FRETBOARD_H

#include <qwidget.h>
#include "global.h"

class TabTrack;
class QSizePolicy;
class QPixmap;
class QImage;

class Fretboard: public QWidget {
    Q_OBJECT
public:
    Fretboard(TabTrack *, QWidget *parent = 0, const char *name = 0);
	~Fretboard();
	void drawBackground();

public slots:
	void setTrack(TabTrack *);

signals:
	void buttonPress(int, int, ButtonState);
	void buttonRelease(ButtonState);

protected:
	virtual void paintEvent(QPaintEvent *);
	virtual void mousePressEvent(QMouseEvent *);
	virtual void mouseMoveEvent(QMouseEvent *);
	virtual void mouseReleaseEvent(QMouseEvent *);
	virtual void resizeEvent(QResizeEvent *);
	virtual QSizePolicy sizePolicy();

private:
	void handleMouse(QMouseEvent *);
	void recalculateSizes();

	TabTrack *trk;
	double fr[MAX_FRETS + 1]; // Proper physical fret positions
	QPixmap *back, *wood;
	QImage *fret, *zeroFret;
};

#endif
