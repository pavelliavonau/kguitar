#include "fingerlist.h"

#include <qpainter.h>

FingerList::FingerList(QWidget *parent,const char *name): QTableView(parent,name)
{
  setTableFlags(Tbl_autoVScrollBar | Tbl_smoothScrolling);
  setFrameStyle(Panel | Sunken);
  setNumRows(3);
  setNumCols(3);
  setCellWidth(20);
  setCellHeight(20);
  repaint();
}

void FingerList::paintCell(QPainter *p, int row, int col)
{
  p->drawEllipse(0,0,10,10);
  return;
}
