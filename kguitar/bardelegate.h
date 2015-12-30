#ifndef BARDELEGATE_H
#define BARDELEGATE_H

#include <QItemDelegate>
#include <QAbstractProxyModel>

class TrackPrint;

class BarDelegate: public QItemDelegate
{
	Q_OBJECT

public:
	explicit BarDelegate(QObject* parent);

	void setTrackPrintPtr(TrackPrint* trp_) { trp = trp_; }

// QAbstractItemDelegate interface
	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
	QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

public slots:
	void setPlaybackCursor(bool c) { playbackCursor = c; }

private:
	TrackPrint *trp;
	bool playbackCursor;
	mutable int selxcoord;
};

#endif //BARDELEGATE_H
