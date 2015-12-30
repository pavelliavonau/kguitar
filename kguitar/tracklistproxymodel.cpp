#include "tracklistproxymodel.h"

#include "data/tabsong.h"
#include <klocale.h>

QVariant TrackListProxyModel::data(const QModelIndex &index, int role) const
{
	if( role == Qt::DisplayRole ) {
		TrackListColumns col = static_cast<TrackListColumns>( index.column() );
		auto track = sourceModel()->data(sourceModel()->index(index.row(), 0), TabSong::TrackPtrRole).value<TabTrack*>();
		Q_ASSERT(track);
		switch (col) {
			case Number : return /*QString::number(*/index.row() + 1/*)*/;
			case Name   : return track->name;
			case Channel: return track->channel;
			case Bank   : return track->bank;
			case Patch  : return track->patch;
			case ColumnCount:
			default:
			Q_ASSERT(false);    // unknown horizontal section
		}
	}

    return QVariant();
}

QVariant TrackListProxyModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if( orientation == Qt::Horizontal && role == Qt::DisplayRole ) {
		TrackListColumns col = static_cast<TrackListColumns>( section );

	switch (col) {
		case Number : return "N";
		case Name   : return i18n("Title");
		case Channel: return i18n("Chn");
		case Bank   : return i18n("Bank");
		case Patch  : return i18n("Patch");
		case ColumnCount:
		//default:
		Q_ASSERT(false);    // unknown horizontal section
	}
	}

	return QVariant();
}

TrackListProxyModel::TrackListProxyModel(QObject *parent) : QAbstractProxyModel(parent)
{
}

QModelIndex TrackListProxyModel::index(int row, int column, const QModelIndex &) const
{
	return createIndex(row,column);
}

QModelIndex TrackListProxyModel::parent(const QModelIndex &) const
{
	return QModelIndex();
}

int TrackListProxyModel::rowCount(const QModelIndex &) const
{
	return sourceModel()->rowCount();
}

int TrackListProxyModel::columnCount(const QModelIndex &) const
{
	return ColumnCount;
}

QModelIndex TrackListProxyModel::mapToSource(const QModelIndex &proxyIndex) const
{
	return sourceModel()->index(proxyIndex.row(), 0);
}

QModelIndex TrackListProxyModel::mapFromSource(const QModelIndex &sourceIndex) const
{
	return createIndex(sourceIndex.row(), 0);
}

void TrackListProxyModel::sourceDataChanged(QModelIndex topLeft, QModelIndex bottomRight)
{
	emit dataChanged(index(topLeft.row(), 0),index(bottomRight.row(), ColumnCount - 1));
}

void TrackListProxyModel::sourceRowsInserted(const QModelIndex &, int first, int last)
{
	beginInsertRows(QModelIndex(), first, last);
	endInsertRows();
}

void TrackListProxyModel::sourceRowsRemoved(const QModelIndex &, int first, int last)
{
	beginRemoveRows(QModelIndex(), first, last);
	endRemoveRows();
}
