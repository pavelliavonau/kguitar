#ifndef TRACKVIEW_H
#define TRACKVIEW_H

#include <qtableview.h>

#define VERTSPACE 20
#define VERTLINE 10
#define HORDUR 4

#define BOTTOMDUR   VERTSPACE+VERTLINE*(s+1)

class TabSong;
class TabTrack;

class TrackView: public QTableView
{
    Q_OBJECT
public:
    TrackView(QWidget *parent=0, const char *name=0);
    ~TrackView();

protected:
    virtual void paintCell(QPainter *, int row, int col);    
    virtual void resizeEvent(QResizeEvent *e);
    virtual void keyPressEvent(QKeyEvent *e);
private:
    bool moveFinger(int from, int to);

    TabSong *song;
    TabTrack *curt;
};

#endif
