#include "tracklist.h"
#include "global.h"

#include "tabsong.h"

#include <qheader.h>

#include <kdebug.h>
#include <kpopupmenu.h>
#include <kxmlgui.h>
#include <kxmlguiclient.h>

#include <iostream.h>

TrackList::TrackList(TabSong *s, KXMLGUIClient *_XMLGUIClient, QWidget *parent = 0, const char *name = 0):
	QListView(parent, name)
{
	song = s;
    m_XMLGUIClient = _XMLGUIClient;

	setFocusPolicy(QWidget::StrongFocus);
    setAllColumnsShowFocus(TRUE);

	addColumn("N");
	addColumn("Title");
	addColumn("Chn");
	addColumn("Bank");
	addColumn("Patch");

	updateList();

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
        tmpWidget = m_XMLGUIClient->factory()->container("tracklistpopup", m_XMLGUIClient);

        if (!tmpWidget || !tmpWidget->inherits("KPopupMenu")) {
            kdDebug() << "TrackList::contentsMousePressEvent => wrong container widget" << endl;
            return;
        }

        KPopupMenu *menu(static_cast<KPopupMenu*>(tmpWidget));
        menu->popup(QCursor::pos());
    }
}



