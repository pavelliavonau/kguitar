#ifndef FINGERING_H
#define FINGERING_H

#include <QAbstractScrollArea>
#include <QMouseEvent>
#include "global.h"

class TabTrack;

class Fingering: public QAbstractScrollArea {
	Q_OBJECT
public:
	Fingering(TabTrack *p, QWidget *parent = 0);

	void setFinger(int string, int fret);

	int  app(int x) { return appl[x]; }
	void setApp(int x, int fret) { appl[x]=fret; }

public slots:
	void clear();
	void setFirstFret(int fret);
	void setFingering(const int *);

signals:
	void chordChange();

protected:
	// QWidget interface
	        void paintEvent(QPaintEvent *) override;
	virtual void mouseMoveEvent(QMouseEvent *) override;
	virtual void mousePressEvent(QMouseEvent *) override;
	void         mouseHandle(const QPoint &pos, bool domute);

private:
	enum { SCALE=20, CIRCLE=16, CIRCBORD=2, BORDER=5, SPACER=3,
	       FRETTEXT=10, SCROLLER=15, NOTES=20 };

	TabTrack *parm;

	int appl[MAX_STRINGS];
	int lastff;
};

#endif
