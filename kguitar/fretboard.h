#ifndef FRETBOARD_H
#define FRETBOARD_H

#include "global.h"

#include <QWidget>

class TabTrack;
class QSizePolicy;
class QPixmap;
class QImage;
class QAbstractItemView;
class QModelIndex;

class Fretboard: public QWidget {
	Q_OBJECT
public:
	Fretboard(QAbstractItemView* tv, QWidget *parent = 0);
	~Fretboard();

public slots:
	void currentBarChangedSlot(QModelIndex, QModelIndex);
	void setTonic(int);
	void setMode(int);
	void drawBackground();

signals:
	void buttonPress(int, int, Qt::ButtonState);
	void buttonRelease(Qt::ButtonState);

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
	void drawScaleBack();

	TabTrack *trk();
	QAbstractItemView* tv;
	double fr[MAX_FRETS + 1]; // Proper physical fret positions
	QPixmap *scaleback, *back, *wood;
	QImage *fret, *zeroFret;

	int tonic;
	int mode;

	enum {
		STRING_HEIGHT = 24,
		ZERO_FRET_WIDTH = 24,
		INLAY_RADIUS = 7,
		FINGER_RADIUS = 8,
		SIDE_BORDER = 2,
		SCALE_BORDER = 5,
		SCALE_ROUND = 70
	};
};

#endif
