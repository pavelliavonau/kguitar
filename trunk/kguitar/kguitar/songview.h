#ifndef SONGVIEW_H
#define SONGVIEW_H

#include "config.h"

#include <qwidget.h>

#include <kcommand.h>

class TrackView;
class TrackList;
class TrackPane;
class TabSong;
class QSplitter;
class KXMLGUIClient;
class KCommandHistory;
class KPrinter;
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
	SongView(KXMLGUIClient *_XMLGUIClient, KCommandHistory *_cmdHist,
	         QWidget *parent = 0, const char *name = 0);
	~SongView();
	void refreshView();
	void print(KPrinter *printer);

	TrackView *tv;
	TrackList *tl;
	TrackPane *tp;
	MelodyEditor *me;

	TabSong* sng() { return song; }
#ifdef WITH_TSE3
	TSE3::MidiScheduler* midiScheduler() { return scheduler; }
#endif

public slots:
	bool trackNew();
	void trackDelete();
	bool trackProperties();
	void trackBassLine();
	void songProperties();
	void playSong();
	void stopPlay();
	void slotCut();
	void slotCopy();
	void slotPaste();
	void slotSelectAll();

	void playbackColumn(int track, int advance);

private:
	TabTrack *highlightedTabs();
	void insertTabs(TabTrack* trk);
	bool setTrackProperties();

	QSplitter *split, *splitv;
	TabSong *song;
	KCommandHistory *cmdHist;
	SongPrint *sp;

	// MIDI stuff
	bool midiInUse, midiStopPlay;

#ifdef WITH_TSE3
	TSE3::MidiScheduler *scheduler;
	TSE3::Transport *transport;
	TSE3::Metronome *metronome;
	PlaybackTracker *tracker;
	bool initMidi();
	void playAllNoteOff();
#endif
};

#endif
