#ifndef TRACKLIST_H
#define TRACKLIST_H

#include <QTableWidget>

class TabSong;
class TabTrack;
class QTableWidgetItem;
class QMouseEvent;
class KXMLGUIClient;
class QMouseEvent;

/**
 * Part of main editor window, shows a list of tracks in a song.
 *
 * Has signals and slots to change display on selecting a new track
 * and mouse event handlers to make selection of tracks by mouse
 * possible.
 */
class TrackList: public QTableWidget {
	Q_OBJECT

public:
	TrackList(TabSong *s, KXMLGUIClient *_XMLGUIClient, QWidget *parent = 0);
	void updateList();

signals:
	void trackSelected(TabTrack *);

protected:
    virtual void mousePressEvent(QMouseEvent *e) override;

public slots:
        void selectTrack(TabTrack*);

private slots:
        void selectNewTrack(QTableWidgetItem * current, QTableWidgetItem * previous);

private:
	void makeHeader();

	TabSong *song;
    KXMLGUIClient *xmlGUIClient;
};

#endif
