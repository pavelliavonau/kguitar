#include "tracklist.h"
#include "global.h"

#include "tabsong.h"

#include <qheader.h>
#include <qcursor.h>

#include <kdebug.h>
#include <klocale.h>
#include <kpopupmenu.h>
#include <kxmlgui.h>
#include <kxmlguiclient.h>

TrackList::TrackList(TabSong *s, KXMLGUIClient *_XMLGUIClient, QWidget *parent, const char *name):
	QListView(parent, name)
{
	song = s;
	xmlGUIClient = _XMLGUIClient;

	setFocusPolicy(QWidget::StrongFocus);
	setAllColumnsShowFocus(TRUE);

	addColumn("N");
	addColumn(i18n("Title"));
	addColumn(i18n("Chn"));
	addColumn(i18n("Bank"));
	addColumn(i18n("Patch"));

	updateList();

	connect(this, SIGNAL(selectionChanged(QListViewItem *)), SLOT(selectNewTrack(QListViewItem *)));

	show();
}

TrackList::~TrackList()
{
}

void TrackList::updateList()
{
	clear();

	QListIterator<TabTrack> it(song->t);
	for (int n = 1; it.current(); ++it) {		// For every track
		TabTrack *trk = it.current();

		(void) new QListViewItem(this, QString::number(n), trk->name,
								 QString::number(trk->channel),
								 QString::number(trk->bank),
								 QString::number(trk->patch));
		n++;
	}

// 	setMaximumHeight(header()->height() + viewport()->height());
}

void TrackList::contentsMousePressEvent(QMouseEvent *e)
{
	QListView::contentsMousePressEvent(e);

	if (e->button() == RightButton) {
		QWidget *tmpWidget = 0;
		tmpWidget = xmlGUIClient->factory()->container("tracklistpopup", xmlGUIClient);

		if (!tmpWidget || !tmpWidget->inherits("KPopupMenu")) {
			kdDebug() << "TrackList::contentsMousePressEvent => wrong container widget" << endl;
			return;
		}

		KPopupMenu *menu(static_cast<KPopupMenu*>(tmpWidget));
		menu->popup(QCursor::pos());
	}

	setSelected(currentItem(), TRUE);
}

void TrackList::selectNewTrack(QListViewItem *item)
{
	if (!item)
		return;

	int num = item->text(0).toInt() - 1;
	emit trackChanged(song->t.at(num));
}
