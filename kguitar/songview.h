#ifndef SONGVIEW_H
#define SONGVIEW_H

#include "config.h"

#include <qwidget.h>
#include "playbacktracker.h"

class TrackView;
class TrackList;
class TrackPane;
class TabSong;
class QSplitter;
class KXMLGUIClient;
class QUndoStack;
class QPrinter;
class SongPrint;
class TabTrack;
class MelodyEditor;

#ifdef WITH_TSE3
#include <tse3/MidiScheduler.h>
#include <tse3/Transport.h>
#include <tse3/Metronome.h>
#endif

class SongView: public QWidget {
	Q_OBJECT
public:
	SongView(KXMLGUIClient *_XMLGUIClient, QUndoStack *_cmdHist,
		 QWidget *parent = 0);
	~SongView();
	void refreshView();
	void print(QPrinter *printer);

	SongPrint *sp;
	TrackView *tv;
	TrackList *tl;
	TrackPane *tp;
	MelodyEditor *me;

	TabSong *song() { return m_song; }
#ifdef WITH_TSE3
	TSE3::MidiScheduler* midiScheduler() { return playThread->midiScheduler(); }
#endif

	// Forwards declarations of all undo/redo commands
	class SetSongPropCommand;
	class SetTrackPropCommand;
	class InsertTabsCommand;

public slots:
	bool trackNew();
	void trackDelete();
	bool trackProperties();
	void trackBassLine();
	/**
	 * Dialog to set song's properties
	 */
	void songProperties();
	/**
	 * Start playing the song or stop it if it already plays
	 */
	void playSong();
	void stopPlay();
	void slotCut();
	void slotCopy();
	void slotPaste();
	void slotSelectAll();

	void setReadOnly(bool _ro) { ro = _ro; }

	void playbackColumn(int track, int advance);

signals:
	void songChanged();

private:
	TabTrack *highlightedTabs();
	void insertTabs(TabTrack* trk);
	bool setTrackProperties();
	void copySelTabsToClipboard();

	QSplitter *split, *splitv;
	TabSong *m_song;
	QUndoStack *cmdHist;

	bool ro;

#ifdef WITH_TSE3
	PlaybackTracker *playThread;
	bool initMidi();
#endif
};

#endif
