#ifndef TRACKVIEW_H
#define TRACKVIEW_H

#include <qtableview.h>

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
    int finger(int num);

    void updateRows();

public slots:
    void setLength1() { setLength(480); };
    void setLength2() { setLength(240); };
    void setLength4() { setLength(120); };
    void setLength8() { setLength(60); };
    void setLength16() { setLength(30); };
    void setLength32() { setLength(15); };
    void timeSig();
    void linkPrev();
    void addHarmonic();
    void addArtHarm();
    void addLegato();

signals:
    void statusBarChanged();

protected:
    virtual void paintCell(QPainter *, int row, int col);    
    virtual void resizeEvent(QResizeEvent *e);
    virtual void keyPressEvent(QKeyEvent *e);
	virtual void mousePressEvent(QMouseEvent *e);

private:
    bool moveFinger(int from, int to);
    void setLength(int l);
	int horizDelta(uint n);

    TabSong *song;
    TabTrack *curt;

    uchar lastnumber;
};

#endif
