#ifndef FINGERLIST_H
#define FINGERLIST_H

#include <qtableview.h>
#include "global.h"

#define ICONCHORD      50

class FingerList: public QTableView
{
    Q_OBJECT
public:
    FingerList(QWidget *parent = 0, const char *name = 0);

    void addFingering(const int a[MAX_STRINGS]);
    void clear();

public slots:
/*     void setFirstFret(int fret); */

protected:
    virtual void paintCell(QPainter *, int row, int col);
    virtual void resizeEvent(QResizeEvent *); 
/*     virtual void drawContents(QPainter *); */
/*     virtual void mouseMoveEvent(QMouseEvent *); */
/*     virtual void mousePressEvent(QMouseEvent *); */
/*     void         mouseHandle(const QPoint &pos, bool domute); */

private:
    enum { SCALE=20, CIRCLE=16, CIRCBORD=2, BORDER=5, SPACER=3, FRETTEXT=10 };
    
    int num,perRow;
    int appl[100][MAX_STRINGS];

    int current;
    int numstr;
    int firstFret;
};

#endif
