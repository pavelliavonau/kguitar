#ifndef TRACKVIEW_H
#define TRACKVIEW_H

#include <qtableview.h>

#define VERTSPACE 20
#define VERTLINE 10

class TabSong;

class TrackView: public QTableView
{
    Q_OBJECT
public:
    TrackView(QWidget *parent=0, const char *name=0);
    ~TrackView();
protected:
    virtual void paintCell(QPainter *, int row, int col);    
    virtual void resizeEvent(QResizeEvent *e);
private:
    TabSong *song;
};

#endif