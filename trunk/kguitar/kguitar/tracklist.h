#ifndef TRACKLIST_H
#define TRACKLIST_H

#include <qlistview.h>

class TabSong;

class TrackList: public QListView {
	Q_OBJECT

public:
	TrackList(TabSong *s, QWidget *parent = 0, const char *name = 0);
	~TrackList();
	void updateList();

private:
	TabSong *song;
};

#endif
