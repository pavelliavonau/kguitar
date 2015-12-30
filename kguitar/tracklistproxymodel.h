#ifndef TRACKLISTPROXYMODEL_H
#define TRACKLISTPROXYMODEL_H

#include <QAbstractProxyModel>

class TrackListProxyModel : public QAbstractProxyModel
{
	Q_OBJECT
public:
	explicit TrackListProxyModel(QObject *parent = 0);

	enum TrackListColumns {
		Number = 0,
		Name,
		Channel,
		Bank,
		Patch,
		ColumnCount // invalid column
	};

	// QAbstractItemModel interface
public:
	QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
	QModelIndex parent(const QModelIndex &child) const override;
	int rowCount(const QModelIndex &parent = QModelIndex()) const override;
	int columnCount(const QModelIndex &parent = QModelIndex()) const override;
	QVariant data(const QModelIndex &index, int role) const override;
	QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

	// QAbstractProxyModel interface
public:
	QModelIndex mapToSource(const QModelIndex &proxyIndex) const override;
	QModelIndex mapFromSource(const QModelIndex &sourceIndex) const override;

public slots:
	void sourceDataChanged(QModelIndex, QModelIndex);
	void sourceRowsInserted(const QModelIndex & parent, int first, int last);
	void sourceRowsRemoved(const QModelIndex & parent, int first, int last);
};

#endif //TRACKLISTPROXYMODEL_H
