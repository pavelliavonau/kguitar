#ifndef SONGVIEW_H
#define SONGVIEW_H

#include <qwidget.h>

#include <kcommand.h>

#include "midilist.h"

class TrackView;
class TrackList;
class TrackPane;
class DeviceManager;
class TabSong;
class QSplitter;
class KXMLGUIClient;
class KCommandHistory;

class SongView: public QWidget {
	Q_OBJECT
public:
	SongView(KXMLGUIClient *_XMLGUIClient, KCommandHistory* _cmdHist,
			 QWidget *parent = 0, const char *name = 0);
	~SongView();
	void refreshView();

	TrackView *tv;
	TrackList *tl;
	TrackPane *tp;

	TabSong* sng() { return song; }
	DeviceManager* devMan() { return midi; }

public slots:
	bool trackNew();
	void trackDelete();
	bool trackProperties();
	void trackBassLine();
	void songProperties();
	void playTrack();
	void playSong();
	void stopPlayTrack();
	void slotCut();
	void slotCopy();
	void slotPaste();
	void slotSelectAll();

private slots:
	void playMidi(MidiList &ml, bool playSong = TRUE);

private:
	TabTrack *highlightedTabs();
	void insertTabs(TabTrack* trk);
	bool setTrackProperties();

	QSplitter *split, *splitv;
	DeviceManager *midi;
	TabSong *song;
	KCommandHistory *m_cmdHist;

	// MIDI stuff
	MidiList midiList;
	bool midiInUse, midiStopPlay;
};

#endif
