#ifndef TRACKLIST_H
#define TRACKLIST_H

#include <QTableView>

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
class TrackList: public QTableView {
	Q_OBJECT

public:
	TrackList(TabSong *s, KXMLGUIClient *_XMLGUIClient, QWidget *parent = 0);
	void updateList();
	void setSourceSelectionModel(QItemSelectionModel *selectionModel);

protected:
	virtual void mousePressEvent(QMouseEvent *e) override;

private slots:
	void currentChangedSlot(QModelIndex current, QModelIndex previous);
	void privateCurrentChangedSlot(QModelIndex current, QModelIndex previous);

private:
	KXMLGUIClient *xmlGUIClient;
	QItemSelectionModel *sourceSelectionModel;
};
#endif
