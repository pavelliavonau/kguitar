#ifndef SONGVIEW_H
#define SONGVIEW_H

#include <qwidget.h>

class TrackView;
class TrackList;
class DeviceManager;
class TabSong;
class QSplitter;

class SongView: public QWidget {
	Q_OBJECT
public:
	SongView(QWidget *parent = 0, const char *name = 0);
	~SongView();
	void refreshView();

	TrackView *tv;
	TrackList *tl;

	TabSong* sng() { return song; }

public slots:
	bool trackNew();
	void trackDelete();
	bool trackProperties();
	void trackBassLine();
	void songProperties();

private:
	QSplitter *split;
	DeviceManager *midi;
	TabSong *song;
};

#endif
