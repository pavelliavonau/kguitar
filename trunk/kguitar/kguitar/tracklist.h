#ifndef TRACKLIST_H
#define TRACKLIST_H

#include <qlistview.h>

class TabSong;
class TabTrack;
class QListViewItem;
class QMouseEvent;
class KXMLGUIClient;

/**
 * Part of main editor window, shows a list of tracks in a song.
 *
 * Has signals and slots to change display on selecting a new track
 * and mouse event handlers to make selection of tracks by mouse
 * possible.
 */
class TrackList: public QListView {
	Q_OBJECT

public:
	TrackList(TabSong *s, KXMLGUIClient *_XMLGUIClient, QWidget *parent = 0, const char *name = 0);
	void updateList();

signals:
	void trackSelected(TabTrack *);

protected:
    virtual void contentsMousePressEvent(QMouseEvent *e);

private slots:
	void selectNewTrack(QListViewItem *);

private:
	TabSong *song;
    KXMLGUIClient *xmlGUIClient;
};

#endif
