#ifndef TRACKLIST_H
#define TRACKLIST_H

#include <qlistview.h>

class TabSong;
class TabTrack;
class QListViewItem;
class QMouseEvent;
class KXMLGUIClient;

class TrackList: public QListView {
	Q_OBJECT

public:
	TrackList(TabSong *s, KXMLGUIClient *_XMLGUIClient, QWidget *parent = 0, const char *name = 0);
	~TrackList();
	void updateList();

signals:
	void newTrackSelected(TabTrack *);

protected:
    virtual void contentsMousePressEvent(QMouseEvent *e);

private slots:
	void selectNewTrack(QListViewItem *);

private:
	TabSong *song;
    KXMLGUIClient *xmlGUIClient;
};

#endif
