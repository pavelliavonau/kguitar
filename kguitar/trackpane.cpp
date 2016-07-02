#include "trackpane.h"

#include "data/tabsong.h"

#include <QPainter>
#include <QMouseEvent>
#include <QStyledItemDelegate>
#include <QApplication>
#include <QHeaderView>

namespace {
	class TrackPaneDelegate: public QStyledItemDelegate
	{
	public:
		TrackPaneDelegate(QObject* parent) : QStyledItemDelegate(parent)
		{}

	// QAbstractItemDelegate interface
		void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
	};

	// Draws that pretty squares for track pane.
	void TrackPaneDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
	{
		QStyledItemDelegate::paint(painter, option, index);

		TabTrack* trk = index.data(TabSong::TrackPtrRole).value<TabTrack*>();

		QStyle* style = QApplication::style();

		if (trk->barStatus(index.column())) {
			style->drawPrimitive(QStyle::PE_FrameButtonBevel, &option, painter);
		}
		//draw current
		//      if (trk->xb == index.column()) {
		//          //style->drawPrimitive(QStyle::PE_FrameFocusRect, &option, painter);
		//          painter->fillRect(option.rect, Qt::DiagCrossPattern);
		//      }
	}
}

TrackPane::TrackPane(int cs, QWidget *parent)
	: QTableView(parent)
{
	setBackgroundRole(QPalette::Light);

	setFocusPolicy(Qt::StrongFocus);
	horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
	verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
	horizontalHeader()->setDefaultSectionSize(cs);
	verticalHeader()->setDefaultSectionSize(cs);
	setVerticalScrollMode(ScrollPerPixel);
	setHorizontalScrollMode(ScrollPerPixel);

	setItemDelegate( new TrackPaneDelegate( this ) );
}

void TrackPane::updateList()
{
	viewport()->update();
}

void TrackPane::mousePressEvent(QMouseEvent *e)
{
	if (e->button() == Qt::LeftButton) {

		auto index = indexAt(e->pos());
		if(!index.isValid())
			return;

		if( e->modifiers() & Qt::ControlModifier )
			selectionModel()->setCurrentIndex(index, QItemSelectionModel::ClearAndSelect);
		else
			selectionModel()->setCurrentIndex(index, QItemSelectionModel::Current | QItemSelectionModel::Clear);

		viewport()->update();
	}
}
