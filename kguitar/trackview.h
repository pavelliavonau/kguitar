#ifndef TRACKVIEW_H
#define TRACKVIEW_H

#include "global.h"

#include <QTableView>
#include <QResizeEvent>
#include <QMouseEvent>

#ifdef WITH_TSE3
#include <tse3/MidiScheduler.h>
#endif

class TabSong;
class TabTrack;
class Fretboard;
class KXMLGUIClient;
class QUndoStack;
class QFont;
class TrackPrint;
class SongPrint;

class TrackView: public QTableView {
	Q_OBJECT
public:
	TrackView(TabSong *s, KXMLGUIClient *_XMLGUIClient, QUndoStack *_cmdHist,
#ifdef WITH_TSE3
	          TSE3::MidiScheduler *_scheduler,
#endif
			  QWidget *parent = nullptr);

	~TrackView();

	TabTrack* trk();

	void setFinger(int num, int fret);
	int finger(int num);

	void setX(int x);

	void updateRows();
	void arrangeBars();

	void repaintCurrentBar();
	void repaintCurrentColumn();

	SongPrint *buildSongPrintHelper();

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
	void setLength1() { setLength(480); }
	void setLength2() { setLength(240); }
	void setLength4() { setLength(120); }
	void setLength8() { setLength(60); }
	void setLength16() { setLength(30); }
	void setLength32() { setLength(15); }
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

	/**
	 * Move cursor left one column, breaking selection
	 */
	void keyLeft();

	/**
	 * Move cursor right one column, breaking selection
	 */
	void keyRight();

	/**
	 * Move cursor to previous bar, breaking selection
	 */
	void keyLeftBar();

	/**
	 * Move cursor to next bar, breaking selection
	 */
	void keyRightBar();

	/**
	 * Move cursor to the beginning of bar, breaking selection
	 */
	void keyHome();

	/**
	 * Move cursor to the ending of bar, breaking selection
	 */
	void keyEnd();

	/**
	 * Move cursor to the very beginning of the song, breaking selection
	 */
	void keyCtrlHome();

	/**
	 * Move cursor to the very end of the song, breaking selection
	 */
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

	void melodyEditorPress(int num, int fret, Qt::MouseButton button);
	void melodyEditorRelease(Qt::MouseButton button);

	void currentBarChangedSlot(QModelIndex current, QModelIndex previous);
	/**
	 * Checks is current bar is fully visible, and, if it's not, tries
	 * to do minimal scrolling to ensure the full visibility.
	 */
	void ensureCurrentVisible();

	void disablePlaybackCursor();
	void setPlaybackCursor(bool);

	void viewScore(bool);

signals:
	void paneChanged();
	/**
	 * Emitted when other column became current, i.e. curt->x is
	 * changed.
	 */
	void columnChanged();
	/**
	 * Emitted when other bar became current, i.e. curt->xb is
	 * changed. Updates status bar's "current bar" and track pane's
	 * current bar cursor.
	 */
	void barChanged();
	/**
	 * Emitted when something changed in a song that will be lost
	 * without saving. Any executed commands, any changes in
	 * tabulatures or song- or track-wide options trigger this
	 * thing. Usually it updates "document modified" flag.
	 */
	void songChanged();
	void playbackCursorChanged(bool);

protected:
	virtual void resizeEvent(QResizeEvent *e) override;
	virtual void mousePressEvent(QMouseEvent *e) override;
	// QAbstractItemView interface
	bool isIndexHidden(const QModelIndex &index) const override;

private:
	bool moveFinger(int from, int to);
	void setLength(int l);
	int horizDelta(uint n);
	void insertTab(int num);
	void melodyEditorAction(int num, int fret, int action);
	void initFonts();

	void moveLeft();
	void moveRight();
	void moveLeftBar();
	void moveRightBar();
	void moveHome();
	void moveEnd();
	void moveCtrlHome();
	void moveCtrlEnd();

	/**
	 * Set new horizontal zoom level and update display accordingly
	 */
	void setZoomLevel(int);

	/**
	 * @return row index of cell in the display that matches designated bar
	 */
	int rowBar(int bar);
	/**
	 * @return column index of cell in the display that matches designated bar
	 */
	int colBar(int bar);
	/**
	 * @return bar number by (row, col) cell indexes
	 */
	int barByRowCol(int row, int col);

	TabSong *song;
	TabTrack *curt;
	TrackPrint *trp;
	Fretboard *fretboard;

#ifdef WITH_TSE3
	TSE3::MidiScheduler *scheduler;
#endif

	KXMLGUIClient *xmlGUIClient;
	QUndoStack *cmdHist;

	void drawLetRing(QPainter *p, int x, int y);

	int barsPerRow;

	QFont *normalFont;
	QFont *timeSigFont;
	QFont *smallCaptionFont;
	QFont *fetaFont;
	QFont *fetaNrFont;

	char lastnumber;
	int selxcoord;
};
#endif
