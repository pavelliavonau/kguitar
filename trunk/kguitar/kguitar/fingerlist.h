#ifndef FINGERLIST_H
#define FINGERLIST_H

#include <qtableview.h>
#include "global.h"

#define ICONCHORD      50

class FingerList: public QTableView
{
    Q_OBJECT
public:
    FingerList(QWidget *parent=0, const char *name=0);

    void addFingering(const int a[MAX_STRINGS], bool update=TRUE);
    void clear();

public slots:
/*     void setFirstFret(int fret); */

signals:
    void chordSelected(const int *);

protected:
    virtual void paintCell(QPainter *, int row, int col);
    virtual void resizeEvent(QResizeEvent *); 
/*     virtual void drawContents(QPainter *); */
/*     virtual void mouseMoveEvent(QMouseEvent *); */
    virtual void mousePressEvent(QMouseEvent *);
/*     void         mouseHandle(const QPoint &pos, bool domute); */

private:
    enum { SCALE=6, CIRCLE=4, CIRCBORD=1, BORDER=1, SPACER=1, FRETTEXT=5 };
    
    int num,perRow;
    int appl[100][MAX_STRINGS];

    int curSel;
    int numstr;
};

#endif
