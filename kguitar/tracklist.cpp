#include "tracklist.h"
#include "global.h"

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
	: QTableWidget(parent)
{
	song = s;
	xmlGUIClient = _XMLGUIClient;

	setFocusPolicy(Qt::StrongFocus);

	setSelectionBehavior(QAbstractItemView::SelectRows);
	setSelectionMode(QAbstractItemView::SingleSelection);
	setEditTriggers(QAbstractItemView::NoEditTriggers);
	verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
	verticalHeader()->setResizeMode(QHeaderView::Fixed);
	setVerticalScrollMode(ScrollPerPixel);
	setHorizontalScrollMode(ScrollPerPixel);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

	updateList();

	connect(this, SIGNAL(currentItemChanged(QTableWidgetItem *, QTableWidgetItem *)),
		this, SLOT(selectNewTrack(QTableWidgetItem *, QTableWidgetItem *)));

	show();
}

void TrackList::updateList()
{
	clear();

	makeHeader();

	setRowCount( song->t.size() );
	// For every track
	for (int i = 0; i < song->t.size(); i++) {// For every track
		TabTrack *trk = song->t.at(i);

		setItem(i, 0, new QTableWidgetItem( QString::number( i + 1      ) ) );
		setItem(i, 1, new QTableWidgetItem( trk->name			  ) );
		setItem(i, 2, new QTableWidgetItem( QString::number(trk->channel) ) );
		setItem(i, 3, new QTableWidgetItem( QString::number(trk->bank   ) ) );
		setItem(i, 4, new QTableWidgetItem( QString::number(trk->patch  ) ) );
	}

	resizeColumnsToContents();

	int max_w = 0;
	for(int i = 0; i < columnCount(); ++i)
	{
	  qDebug() << i;
	  max_w += columnWidth(i);
	}
	// TODO: investigate and remove magic offset
	max_w += verticalHeader()->sizeHint().width() + 6;
	setMaximumWidth(max_w);


	int max_h = 0;
	for(int i = 0; i < rowCount(); ++i)
	{
	  qDebug() << i;
	  max_h += rowHeight(i);
	}
	// TODO: investigate and remove magic offset
	max_h += horizontalHeader()->sizeHint().height() + horizontalScrollBar()->size().height() + 6;
	setMaximumHeight(max_h);
}

void TrackList::mousePressEvent(QMouseEvent *e)
{
	QTableWidget::mousePressEvent(e);

	if (e->button() == Qt::RightButton) {
		QWidget *tmpWidget = nullptr;
		tmpWidget = xmlGUIClient->factory()->container("tracklistpopup", xmlGUIClient);

		if (!tmpWidget) {
			kdDebug() << "TrackList::contentsMousePressEvent => no container widget" << endl;
			return;
		}

		if (!tmpWidget->inherits("QMenu")) {
			kdDebug() << "TrackList::contentsMousePressEvent => container widget is not QMenu" << endl;
			return;
		}

		QMenu *menu(static_cast<QMenu*>(tmpWidget));
		menu->popup(QCursor::pos());
	  }
}

void TrackList::selectTrack(TabTrack * t)
{
      setCurrentCell(song->t.indexOf(t), 0);
}

void TrackList::selectNewTrack(QTableWidgetItem *current, QTableWidgetItem *previous)
{
	Q_UNUSED(previous)
	if (!current)
		return;

	int num = current->row();
	emit trackSelected(song->t.at(num));
}

void TrackList::makeHeader()
{
  QStringList hlabels;
  hlabels << "N"
          << i18n("Title")
          << i18n("Chn")
          << i18n("Bank")
          << i18n("Patch");

  setColumnCount( hlabels.size() );
  setHorizontalHeaderLabels( hlabels );
}
