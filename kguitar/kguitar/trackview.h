#ifndef TRACKVIEW_H
#define TRACKVIEW_H

#include <qtableview.h>

#include "midilist.h"

class TabSong;
class TabTrack;
class DeviceManager;
class QListViewItem;

class TrackView: public QTableView {
    Q_OBJECT
public:
    TrackView(TabSong* s, QWidget *parent = 0, const char *name = 0);
    ~TrackView();

//     TabSong* sng() { return song; }

    TabTrack* trk() { return curt; }
    void setCurt(TabTrack *trk) { curt = trk; }

	DeviceManager* devMan() { return midi; }

    void setFinger(int num, int fret);
    int finger(int num);

    void updateRows();
    void arrangeBars();

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
	void insertChord();
    void keyLeft();
    void keyRight();
    void keyUp();
    void keyDown();
    void keyCtrlUp();
    void keyCtrlDown();
	void deadNote();
	void keyDelete();
    void keyCtrlDelete();
	void keyInsert();
	void keyM();
	void keyPeriod();
	void keyPlus();
	void keyMinus();
	void arrangeTracks();
    void key1();
    void key2();
    void key3();
    void key4();
    void key5();
    void key6();
    void key7();
    void key8();
    void key9();
    void key0();
	void selectTrack(QListViewItem *);
    void playTrack();
    void stopPlayTrack();

private slots:
    void playMidi(MidiList &ml);

signals:
	void statusBarChanged();

protected:
	virtual void paintCell(QPainter *, int row, int col);
	virtual void resizeEvent(QResizeEvent *e);
	virtual void mousePressEvent(QMouseEvent *e);

private:
    bool moveFinger(int from, int to);
    void setLength(int l);
	int horizDelta(uint n);
    void insertTab(int num);

    TabSong *song;
    TabTrack *curt;
	DeviceManager *midi;

    uchar lastnumber;

    // MIDI stuff
    MidiList midiList;
    bool midiInUse, midiStopPlay;
};

#endif
