#ifndef SONGVIEW_H
#define SONGVIEW_H

#include <qwidget.h>

#include "midilist.h"

class TrackView;
class TrackList;
class TrackPane;
class DeviceManager;
class TabSong;
class QSplitter;
class KXMLGUIClient;

class SongView: public QWidget {
	Q_OBJECT
public:
	SongView(KXMLGUIClient *_XMLGUIClient, QWidget *parent = 0, const char *name = 0);
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
	void stopPlayTrack();

private slots:
	void playMidi(MidiList &ml);

private:
	QSplitter *split, *splitv;
	DeviceManager *midi;
	TabSong *song;

	// MIDI stuff
	MidiList midiList;
	bool midiInUse, midiStopPlay;
};

#endif
