#include "tracklist.h"
#include "global.h"

#include "tabsong.h"

#include <qheader.h>
#include <kdebug.h>
#include <iostream.h>

TrackList::TrackList(TabSong *s, QWidget *parent = 0, const char *name = 0):
	QListView(parent, name)
{
	song = s;

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
