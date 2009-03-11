#include "tracklist.h"
#include "global.h"

#include "tabsong.h"

#include <q3header.h>
#include <qcursor.h>
//Added by qt3to4:
#include <QMouseEvent>

#include <kdebug.h>
#include <klocale.h>
#include <kmenu.h>
#include <kxmlguiclient.h>
#include <kxmlguifactory.h>

TrackList::TrackList(TabSong *s, KXMLGUIClient *_XMLGUIClient, QWidget *parent, const char *name)
	: Q3ListView(parent, name)
{
	song = s;
	xmlGUIClient = _XMLGUIClient;

	setFocusPolicy(Qt::StrongFocus);
	setAllColumnsShowFocus(TRUE);

	addColumn("N");
	addColumn(i18n("Title"));
	addColumn(i18n("Chn"));
	addColumn(i18n("Bank"));
	addColumn(i18n("Patch"));

	updateList();

	connect(this, SIGNAL(selectionChanged(Q3ListViewItem *)), SLOT(selectNewTrack(Q3ListViewItem *)));

	show();
}

void TrackList::updateList()
{
	clear();

	// For every track
	for (int i = 0; i < song->t.size(); i++) {// For every track
		TabTrack *trk = song->t.at(i);

		(void) new Q3ListViewItem(this, QString::number(i + 1), trk->name,
								 QString::number(trk->channel),
								 QString::number(trk->bank),
								 QString::number(trk->patch));
	}

// 	setMaximumHeight(header()->height() + viewport()->height());
}

void TrackList::contentsMousePressEvent(QMouseEvent *e)
{
	Q3ListView::contentsMousePressEvent(e);

	if (e->button() == Qt::RightButton) {
		QWidget *tmpWidget = 0;
		tmpWidget = xmlGUIClient->factory()->container("tracklistpopup", xmlGUIClient);

		if (!tmpWidget || !tmpWidget->inherits("KPopupMenu")) {
			kdDebug() << "TrackList::contentsMousePressEvent => wrong container widget" << endl;
			return;
		}

		KMenu *menu(static_cast<KMenu*>(tmpWidget));
		menu->popup(QCursor::pos());
	}

	setSelected(currentItem(), TRUE);
}

void TrackList::selectNewTrack(Q3ListViewItem *item)
{
	if (!item)
		return;

	int num = item->text(0).toInt() - 1;
	emit trackSelected(song->t.at(num));
}
