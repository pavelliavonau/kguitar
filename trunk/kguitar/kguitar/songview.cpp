#include "songview.h"
#include "global.h"
#include "trackview.h"
#include "tracklist.h"
#include "trackpane.h"
#include "tabsong.h"
#include "tabtrack.h"
#include "settrack.h"
#include "settabfret.h"
#include "settabdrum.h"
#include "setsong.h"
#include "chord.h"
#include "chordlist.h"
#include "chordlistitem.h"

#include <klocale.h>
#include <kdebug.h>
#include <kxmlgui.h>
#include <kxmlguiclient.h>
#include <knuminput.h>
#include <kmessagebox.h>

#include <qsplitter.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qmultilinedit.h>
#include <qheader.h>


SongView::SongView(KXMLGUIClient *_XMLGUIClient, QWidget *parent = 0, const char *name = 0)
    : QWidget(parent, name)
{
	song = new TabSong(i18n("Unnamed"), 120);
	song->t.append(new TabTrack(FretTab, i18n("Guitar"), 1, 0, 25, 6, 24));

 	split = new QSplitter(this);
 	split->setOrientation(QSplitter::Vertical);

	tv = new TrackView(song, _XMLGUIClient, split);
	splitv = new QSplitter(split);
 	splitv->setOrientation(QSplitter::Horizontal);

	tl = new TrackList(song, _XMLGUIClient, splitv);
    tl->setSelected(tl->firstChild(), TRUE);
	tp = new TrackPane(song, tl->header()->height(), tl->firstChild()->height(), splitv);

	connect(tl, SIGNAL(selectionChanged(QListViewItem*)), tv, SLOT(selectTrack(QListViewItem*)));

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
    tl->setSelected(tl->firstChild(), TRUE);
	tp->updateList();
}

// Creates a new track in the song
bool SongView::trackNew()
{
	TabTrack* oldtr = tv->trk();
	TabTrack* newtr = new TabTrack(FretTab, "", song->freeChannel(), 0, 25, 6, 24);

	song->t.append(newtr);
	tv->setCurt(newtr);

	// Special case - if user declined track properties dialog during
	// track creation, then he doesn't seem to want the new track, so
	// we'll destroy it.

	if (!trackProperties()) {
		tv->setCurt(oldtr);
		song->t.removeLast();
		return FALSE;
	}

	return TRUE;
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
		tv->update();
		tl->updateList();
		tp->updateList();
	}
}

// Generates a new track with a basic bass line, derived from current
// track's rhythm
void SongView::trackBassLine()
{
	if (tv->trk()->trackMode() == DrumTab) {
		KMessageBox::sorry(this, i18n("Can't generate a bass line from drum track"));
		return;
	}

	TabTrack *origtrk = tv->trk();

	if (trackNew()) {
		TabTrack *newtrk = tv->trk();
		newtrk->c.resize(origtrk->c.size());
		ChordSelector cs(tv->devMan(), origtrk);

		int note;
        bool havenote;

		for (uint i = 0; i < origtrk->c.size(); i++) {
			for (uint k = 0; k < origtrk->string; k++) {
				cs.setApp(k, origtrk->c[i].a[k]);
			}

			cs.detectChord();
            havenote = ((ChordListItem *) cs.chords->item(0));

            if (havenote) {
                note = ((ChordListItem *) cs.chords->item(0))->tonic();
                kdDebug() << "Column " << i << ", detected tonic " << note_name(note) << endl;
            }
            else
                kdDebug() << "Column " << i << ", EMPTY " << endl;

			for (uint k = 0; k < newtrk->string; k++) {
				newtrk->c[i].a[k] = -1;
				newtrk->c[i].e[k] = 0;
			}

			newtrk->c[i].l = origtrk->c[i].l;
			newtrk->c[i].flags = 0;

			// GREYFIX: make a better way of choosing a fret. This way
			// it can, for example, be over max frets number.
            if (havenote) {
                newtrk->c[i].a[0] = note - newtrk->tune[0] % 12;
                if (newtrk->c[i].a[0] < 0)  newtrk->c[i].a[0] += 12;
            }
		}
	};

    tv->arrangeTracks();
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
			tv->trk()->string = drum->drums();
			tv->trk()->frets = 0;
			for (int i = 0; i < tv->trk()->string; i++)
				tv->trk()->tune[i] = drum->tune(i);
		}

		tv->setCurt(tv->trk()); // artificially needed to emit newTrackSelected()
		tl->updateList();
		tp->updateList();
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
