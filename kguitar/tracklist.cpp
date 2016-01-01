#include "tracklist.h"
#include "global.h"
#include "tracklistproxymodel.h"

#include "data/tabsong.h"

#include <qcursor.h>
#include <QMouseEvent>
#include <QHeaderView>
#include <QScrollBar>

#include <kdebug.h>
#include <klocale.h>
#include <kmenu.h>
#include <kxmlguiclient.h>
#include <kxmlguifactory.h>

TrackList::TrackList(TabSong *s, KXMLGUIClient *_XMLGUIClient, QWidget *parent)
	: QTableView(parent)
	, sourceSelectionModel(nullptr)
{
	auto p = new TrackListProxyModel;
	p->setSourceModel(s);
	setModel(p);
	// TODO: after Qt5 migration refactor with lambdas
	connect(s, SIGNAL(dataChanged(QModelIndex,QModelIndex)),        p, SLOT(sourceDataChanged(QModelIndex, QModelIndex)));
	connect(s, SIGNAL(rowsInserted(const QModelIndex &, int, int)), p, SLOT(sourceRowsInserted(const QModelIndex &, int, int)));
	connect(s, SIGNAL(rowsRemoved(const QModelIndex &, int, int)) , p, SLOT(sourceRowsRemoved(const QModelIndex &, int, int)));

	xmlGUIClient = _XMLGUIClient;

	setFocusPolicy(Qt::StrongFocus);

	setSelectionBehavior(QAbstractItemView::SelectRows);
	setSelectionMode(QAbstractItemView::SingleSelection);
	setEditTriggers(QAbstractItemView::NoEditTriggers);
	verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
	verticalHeader()->setResizeMode(QHeaderView::Fixed);
	horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
	setVerticalScrollMode(ScrollPerPixel);
	setHorizontalScrollMode(ScrollPerPixel);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

	updateList();
}

void TrackList::updateList()
{
	int max_w = 0;
	for(int i = 0; i < model()->columnCount(); ++i)
	{
		qDebug() << i;
		max_w += columnWidth(i);
	}
	// TODO: investigate and remove magic offset
	max_w += verticalHeader()->sizeHint().width() + 6;
	setMaximumWidth(max_w);


	int max_h = 0;
	for(int i = 0; i < model()->rowCount(); ++i)
	{
		qDebug() << i;
		max_h += rowHeight(i);
	}
	// TODO: investigate and remove magic offset
	max_h += horizontalHeader()->sizeHint().height() + horizontalScrollBar()->size().height() + 6;
	setMaximumHeight(max_h);

	viewport()->update();
}

void TrackList::mousePressEvent(QMouseEvent *e)
{
	QTableView::mousePressEvent(e);

	if (e->button() == Qt::RightButton) {
		QWidget *tmpWidget = nullptr;
		tmpWidget = xmlGUIClient->factory()->container("tracklistpopup", xmlGUIClient);

		if (!tmpWidget) {
			kDebug() << "TrackList::contentsMousePressEvent => no container widget" << endl;
			return;
		}

		if (!tmpWidget->inherits("QMenu")) {
			kDebug() << "TrackList::contentsMousePressEvent => container widget is not QMenu" << endl;
			return;
		}

		QMenu *menu(static_cast<QMenu*>(tmpWidget));
		menu->popup(QCursor::pos());
	  }
}

void TrackList::currentChangedSlot(QModelIndex current, QModelIndex)
{
	selectRow(current.row());
}

void TrackList::privateCurrentChangedSlot(QModelIndex current, QModelIndex)
{
	auto newCurrentIndex = sourceSelectionModel->model()->index(current.row(),sourceSelectionModel->currentIndex().column());
	sourceSelectionModel->setCurrentIndex(newCurrentIndex, QItemSelectionModel::Current);
}

void TrackList::setSourceSelectionModel(QItemSelectionModel *selectionModel)
{
	sourceSelectionModel = selectionModel;
	// from source
	connect(selectionModel        , SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(currentChangedSlot(QModelIndex,QModelIndex)));
	// to source
	connect(this->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(privateCurrentChangedSlot(QModelIndex,QModelIndex)));
}
