#ifndef TRACKVIEW_H
#define TRACKVIEW_H

#include "global.h"

#include <qgridview.h>

#ifdef WITH_TSE3
#include <tse3/MidiScheduler.h>
#endif

class TabSong;
class TabTrack;
class Fretboard;
class QListViewItem;
class KXMLGUIClient;
class KCommandHistory;
class QFont;

class TrackView: public QGridView {
	Q_OBJECT
public:
	TrackView(TabSong *s, KXMLGUIClient *_XMLGUIClient, KCommandHistory *_cmdHist,
#ifdef WITH_TSE3
	          TSE3::MidiScheduler *_scheduler,
#endif
			  QWidget *parent = 0, const char *name = 0);

	~TrackView();

	TabTrack* trk() { return curt; }
	void setCurrentTrack(TabTrack *);

 	void setFinger(int num, int fret);
	int finger(int num);

	void setX(int x);

	void updateRows();
	void arrangeBars();

	void repaintCurrentCell();
	void repaintCurrentColumn();

	// Forwards declarations of all undo/redo commands
	class SetLengthCommand;
	class InsertTabCommand;
	class MoveFingerCommand;
	class AddFXCommand;
	class SetFlagCommand;
	class DeleteNoteCommand;
	class AddColumnCommand;
	class DeleteColumnCommand;
	class SetTimeSigCommand;
	class InsertColumnCommand;
	class InsertStrumCommand;
	class InsertRhythm;

public slots:
	void setLength1() { setLength(480); };
	void setLength2() { setLength(240); };
	void setLength4() { setLength(120); };
	void setLength8() { setLength(60); };
	void setLength16() { setLength(30); };
	void setLength32() { setLength(15); };
	void keySig();
	void timeSig();
	void linkPrev();
	void addHarmonic();
	void addArtHarm();
	void addLegato();
	void addSlide();
	void addLetRing();
	void insertChord();
	void rhythmer();
	void keyLeft();
	void keyRight();
	void keyHome();
	void keyEnd();
	void keyCtrlHome();
	void keyCtrlEnd();
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
	void dotNote();
	void tripletNote();
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

	void zoomIn();
	void zoomOut();
	void zoomLevelDialog();

	void melodyEditorPress(int num, int fret, ButtonState button);
	void melodyEditorRelease(ButtonState button);

	void selectTrack(TabTrack *);
	void selectBar(uint);
	void ensureCurrentVisible();

	void setPlaybackCursor(bool);

signals:
	void statusBarChanged();
	void paneChanged();
	void trackChanged(TabTrack *);
	void columnChanged();
	void songChanged();

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
    void melodyEditorAction(int num, int fret, int action);

	void moveLeft();
	void moveRight();
	void moveHome();
	void moveEnd();
	void moveCtrlHome();
	void moveCtrlEnd();

	void setZoomLevel(int);

	TabSong *song;
	TabTrack *curt;
	Fretboard *fretboard;

	bool playbackCursor;

#ifdef WITH_TSE3
	TSE3::MidiScheduler *scheduler;
#endif

	KXMLGUIClient *xmlGUIClient;
	KCommandHistory *cmdHist;

	void drawLetRing(QPainter *p, int x, int y);

	QFont *timeSigFont;
	QFont *smallCaptionFont;

	char lastnumber;
	int selxcoord;

	int zoomLevel;
};

#endif
