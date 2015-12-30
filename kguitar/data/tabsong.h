#ifndef TABSONG_H
#define TABSONG_H

#include "global.h"

#include <qstring.h>
#include <qmap.h>
#include <QAbstractTableModel>

#ifdef WITH_TSE3
#include <tse3/Song.h>
#endif

#include "tabtrack.h"

/**
 * Represents tabulature-based song in memory.
 *
 * Stores a collection of TabTracks and misc song info, such as
 * metainfo and tempo info.
 */
class TabSong : public QAbstractTableModel{
	Q_OBJECT
public:

	enum SongDataRoles {
		BarRole   = Qt::UserRole + 1,
		TrackPtrRole = Qt::UserRole + 2,
	};

	TabSong(QString _title, int _tempo);
	int tempo;

	/**
	 * Map of metainformation. Can hold lots of strings, referenced by
	 * another lot of strings as a key. Generally, we follow Ogg
	 * Vorbis comment recommendations for key names, for example,
	 * info["TITLE"] should return title of the song.
	 */
	QMap<QString, QString> info;

	/**
	 * Find the minimal free channel, for example, to use for new
	 * track
	 */
	int freeChannel();

	/**
	 * Returns length of longest track in bars
	 */
	uint maxLen() const;

	void arrangeBars();
	void addEmptyTrack();
#ifdef WITH_TSE3
	TSE3::Song *midiSong(bool tracking = FALSE);
#endif

	// QAbstractItemModel interface
public:
	int rowCount(const QModelIndex &parent = QModelIndex()) const override;
	int columnCount(const QModelIndex &parent = QModelIndex()) const override;
	QVariant data(const QModelIndex &index, int role) const override;
	bool insertColumns(int column, int count, const QModelIndex &parent = QModelIndex()) override;
	bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
	bool setData(const QModelIndex &index, const QVariant &value, int role) override;
	bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
	bool removeColumns(int column, int count, const QModelIndex &parent = QModelIndex()) override;

private:
	void normalizeBarCount(TabTrack* track_) const;
	/**
	 * Holds a list of tracks that the song consists of.
	 */
	QList<TabTrack*> t;
};

#endif
