#ifndef FINGERLIST_H
#define FINGERLIST_H

#include <qtableview.h>
#include "global.h"

#define ICONCHORD      50

class TabTrack;

typedef struct {
    int f[MAX_STRINGS];
} fingering;

class FingerList: public QTableView
{
    Q_OBJECT
public:
    FingerList(TabTrack *p, QWidget *parent=0, const char *name=0);

    void addFingering(const int a[MAX_STRINGS], bool update);
    void clear();
    void switchAuto(bool update);

signals:
    void chordSelected(const int *);

protected:
    virtual void paintCell(QPainter *, int row, int col);
    virtual void resizeEvent(QResizeEvent *); 
    virtual void mousePressEvent(QMouseEvent *);

private:
    enum { SCALE=6, CIRCLE=4, CIRCBORD=1, BORDER=1, SPACER=1, FRETTEXT=5 };
    
    int num,perRow;
    QArray<fingering> appl;

    int curSel,oldCol,oldRow;
    TabTrack *parm;
};

#endif
