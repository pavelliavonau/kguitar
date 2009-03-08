#ifndef FINGERLIST_H
#define FINGERLIST_H

#include <qgridview.h>
#include "global.h"

#define ICONCHORD	   50

class TabTrack;

typedef struct {
	int f[MAX_STRINGS];
} fingering;

class FingerList: public QGridView {
	Q_OBJECT
public:
	FingerList(TabTrack *p, QWidget *parent = 0, const char *name = 0);
	~FingerList();

	void addFingering(const int a[MAX_STRINGS]);
	void clear();
	void beginSession();
	void endSession();
	int count() { return appl.count(); };
	void selectFirst();

signals:
	void chordSelected(const int *);

protected:
	virtual void paintCell(QPainter *, int row, int col);
	virtual void resizeEvent(QResizeEvent *);
	virtual void mousePressEvent(QMouseEvent *);

private:
	enum { SCALE=6, CIRCLE=5, CIRCBORD=1, BORDER=1, SPACER=1, FRETTEXT=9 };

	int num, perRow;
	QMemArray<fingering> appl;

	int curSel, oldCol, oldRow;
	TabTrack *parm;

	QFont *fretNumberFont;
};

#endif
