#ifndef TRACKVIEW_H
#define TRACKVIEW_H

#include <qtableview.h>
#include <qscrollview.h>


class TabSong;
class TabTrack;
//##class DeviceManager;
class QListViewItem;
class KXMLGUIClient;
class KCommandHistory;
class QFont;

class TrackView: public QTableView {
	Q_OBJECT
public:
	TrackView(TabSong* s, KXMLGUIClient *_XMLGUIClient, KCommandHistory* _cmdHist,
			  /*DeviceManager *_dm,*/ QWidget *parent = 0, const char *name = 0); //##
	~TrackView();

	TabTrack* trk() { return curt; }
	void setCurt(TabTrack *);

	void setFinger(int num, int fret);
	int finger(int num);

	void updateRows();
	void arrangeBars();

	void repaintCurrentCell();
	void repaintCurrentColumn();

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
	void addSlide();
	void insertChord();
	void keyLeft();
	void keyRight();
	void moveUp();
	void moveDown();
	void transposeUp();
	void transposeDown();
	void selectLeft();
	void selectRight();
	void deadNote();
	void deleteNote();
	void deleteColumn();
	void deleteColumn(QString name);
	void insertColumn();
	void palmMute();
	void keyPeriod();
	void keyPlus();
	void keyMinus();
	void arrangeTracks();
	void key1() { insertTab(1); }
	void key2() { insertTab(2); }
	void key3() { insertTab(3); }
	void key4() { insertTab(4); }
	void key5() { insertTab(5); }
	void key6() { insertTab(6); }
	void key7() { insertTab(7); }
	void key8() { insertTab(8); }
	void key9() { insertTab(9); }
	void key0() { insertTab(0); }

	void selectTrack(TabTrack *);
	void selectBar(uint);
	void ensureCurrentVisible();

signals:
	void statusBarChanged();
	void paneChanged();
	void newTrackSelected();

protected:
	void repaintCellNumber(int n);

	virtual void paintCell(QPainter *, int row, int col);
	virtual void resizeEvent(QResizeEvent *e);
	virtual void mousePressEvent(QMouseEvent *e);

private:
	bool moveFinger(int from, int to);
	void setLength(int l);
	int horizDelta(uint n);
	void insertTab(int num);

	void moveLeft();
	void moveRight();

	TabSong *song;
	TabTrack *curt;
//##	DeviceManager *midi;
	KXMLGUIClient *m_XMLGUIClient;
	KCommandHistory *m_cmdHist;

	QFont *timeSigFont;
	QFont *smallCaptionFont;

	char lastnumber;
	int selxcoord;
};

#endif
