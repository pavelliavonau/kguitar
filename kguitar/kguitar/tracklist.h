#ifndef TRACKLIST_H
#define TRACKLIST_H

#include <qlistview.h>

class TabSong;
class QListViewItem;
class QMouseEvent;
class KXMLGUIClient;

class TrackList: public QListView {
	Q_OBJECT

public:
	TrackList(TabSong *s, KXMLGUIClient *_XMLGUIClient, QWidget *parent = 0, const char *name = 0);
	~TrackList();
	void updateList();

protected:
    virtual void contentsMousePressEvent(QMouseEvent *e);

private:
	TabSong *song;
    KXMLGUIClient *m_XMLGUIClient;
};

#endif
