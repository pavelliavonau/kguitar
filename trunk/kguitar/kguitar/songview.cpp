#include "songview.h"
#include "global.h"
#include "trackview.h"
#include "tracklist.h"
#include "tabsong.h"
#include "settrack.h"
#include "settabfret.h"
#include "setsong.h"

#include <qsplitter.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <knuminput.h>
#include <qmultilinedit.h>

SongView::SongView(QWidget *parent = 0, const char *name = 0): QWidget(parent, name)
{
	song = new TabSong("Unnamed", 120);
	song->t.append(new TabTrack(FretTab, "Guitar", 1, 0, 25, 6, 24));

 	split = new QSplitter(this);
 	split->setOrientation(QSplitter::Vertical);

	tv = new TrackView(song, split);
	tl = new TrackList(song, split);

	connect(tl, SIGNAL(clicked(QListViewItem*)), tv, SLOT(selectTrack(QListViewItem*)));

	QBoxLayout *l = new QVBoxLayout(this);
	l->addWidget(split);
}

SongView::~SongView()
{
	delete song;
}

// Refreshes all the views and resets all minor parameters in the
// song. Should be called every time when the song's just got loaded
// or imported.
void SongView::refreshView()
{
	tv->setCurt(song->t.first());
	tv->updateRows();
	tv->repaint();
	tl->updateList();
}

// Creates a new track in the song
void SongView::trackNew()
{
	TabTrack* oldtr = tv->trk();
	TabTrack* newtr = new TabTrack(FretTab, "", 1, 0, 25, 6, 24);

	song->t.append(newtr);
	tv->setCurt(newtr);

	// Special case - if user declined track properties dialog during
	// track creation, then he doesn't seem to want the new track, so
	// we'll destroy it.

	if (!trackProperties()) {
		tv->setCurt(oldtr);
		song->t.removeLast();
	}
}

// Deletes the currently selected track in the song
void SongView::trackDelete()
{
	// Check that we won't delete the only last track in the list
	if (song->t.getFirst() != song->t.getLast()) {
		TabTrack *newsel;

		// If we delete the last track, make sure we'll get the item
		if (song->t.last() == tv->trk()) {
			newsel = song->t.prev();
		} else {
			song->t.findRef(tv->trk());
			newsel = song->t.next();
		}

		song->t.remove(tv->trk());
		tv->setCurt(newsel);
		tv->updateRows();
		tv->repaint();
		tl->updateList();
	}
}

// Sets current track's properties
bool SongView::trackProperties()
{
	bool res = FALSE;
	SetTrack *st = new SetTrack(tv->trk());

	if (st->exec()) {
		tv->trk()->name = st->title->text();
		tv->trk()->channel = st->channel->value();
		tv->trk()->bank = st->bank->value();
		tv->trk()->patch = st->patch->value();
		tv->trk()->setTrackMode((TrackMode) st->mode->currentItem());

		// Fret tab
		if (st->mode->currentItem() == FretTab) {
			SetTabFret *fret = (SetTabFret *) st->modespec;
			tv->trk()->string = fret->string();
			tv->trk()->frets = fret->frets();
			for (int i = 0; i < tv->trk()->string; i++)
				tv->trk()->tune[i] = fret->tune(i);
		}

		// Drum tab
		if (st->mode->currentItem() == DrumTab) {
			SetTabDrum *drum = (SetTabDrum *) st->modespec;
		}

		tl->updateList();
		res = TRUE;
	}

	delete st;
	return res;
}

void SongView::songProperties()
{
	SetSong *ss = new SetSong();
	ss->title->setText(song->title);
	ss->title->setReadOnly(isBrowserView);
	ss->author->setText(song->author);
	ss->author->setReadOnly(isBrowserView);
	ss->transcriber->setText(song->transcriber);
	ss->transcriber->setReadOnly(isBrowserView);
	ss->comments->setText(song->comments);
	ss->comments->setReadOnly(isBrowserView);

	if (ss->exec()) {
		song->title = ss->title->text();
		song->author = ss->author->text();
		song->transcriber = ss->transcriber->text();
		song->comments = ss->comments->text();
	}

	delete ss;
}
