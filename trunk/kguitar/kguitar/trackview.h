#ifndef TRACKVIEW_H
#define TRACKVIEW_H

#include <qtableview.h>

#define VERTSPACE 25
#define VERTLINE 10
#define HORDUR 4
#define HORCELL 8

#define BOTTOMDUR   VERTSPACE+VERTLINE*(s+1)

class TabSong;
class TabTrack;

class TrackView: public QTableView
{
    Q_OBJECT
public:
    TrackView(QWidget *parent=0, const char *name=0);
    ~TrackView();

    TabSong* sng() { return song; }

    TabTrack* trk() { return curt; }
    void setCurt(TabTrack *trk) { curt = trk; }

    void setFinger(int num, int fret);

protected:
    virtual void paintCell(QPainter *, int row, int col);    
    virtual void resizeEvent(QResizeEvent *e);
    virtual void keyPressEvent(QKeyEvent *e);
private:
    bool moveFinger(int from, int to);

    TabSong *song;
    TabTrack *curt;

    uchar lastnumber;
};

#endif
