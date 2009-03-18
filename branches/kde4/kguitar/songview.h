#ifndef SONGVIEW_H
#define SONGVIEW_H

#include "config.h"

#include <qwidget.h>

#include <k3command.h>

class TrackView;
class TrackList;
class TrackPane;
class TabSong;
class QSplitter;
class KXMLGUIClient;
class K3CommandHistory;
class QPrinter;
class SongPrint;
class TabTrack;
class MelodyEditor;
class PlaybackTracker;

#ifdef WITH_TSE3
#include <tse3/MidiScheduler.h>
#include <tse3/Transport.h>
#include <tse3/Metronome.h>
#endif

class SongView: public QWidget {
	Q_OBJECT
public:
	SongView(KXMLGUIClient *_XMLGUIClient, K3CommandHistory *_cmdHist,
	         QWidget *parent = 0, const char *name = 0);
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
	// GREYFIX
	TSE3::MidiScheduler* midiScheduler() { return 0; }
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

	void setReadOnly(bool _ro) { ro = _ro; };

	void playbackColumn(int track, int advance);

signals:
	void songChanged();

private:
	TabTrack *highlightedTabs();
	void insertTabs(TabTrack* trk);
	bool setTrackProperties();

	QSplitter *split, *splitv;
	TabSong *m_song;
	K3CommandHistory *cmdHist;

	bool ro;

#ifdef WITH_TSE3
	PlaybackTracker *playThread;
	bool initMidi();
#endif
};

#endif
