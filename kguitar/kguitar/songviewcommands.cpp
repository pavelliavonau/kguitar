#include "songviewcommands.h"
#include "tabsong.h"
#include "tabtrack.h"
#include "trackview.h"
#include "tracklist.h"
#include "trackpane.h"

#include <klocale.h>

SongView::SetSongPropCommand::SetSongPropCommand(SongView *_sv, QMap<QString, QString> _info, int _tempo)
	: KNamedCommand(i18n("Set song properties"))
{
    sv       = _sv;
	info     = _info;
	tempo    = _tempo;

	oldinfo  = sv->song()->info;
	oldtempo = sv->song()->tempo;
}

void SongView::SetSongPropCommand::execute()
{
	sv->song()->info  = info;
	sv->song()->tempo = tempo;
	emit sv->songChanged();
}

void SongView::SetSongPropCommand::unexecute()
{
	sv->song()->info  = oldinfo;
	sv->song()->tempo = oldtempo;
	emit sv->songChanged();
}

SongView::SetTrackPropCommand::SetTrackPropCommand(TrackView *_tv, TrackList *_tl, TrackPane *_tp,
										 TabTrack *_trk, TabTrack *_newtrk):
	KNamedCommand(i18n("Set track properties"))
{
	tv     = _tv;
	tl     = _tl;
	tp     = _tp;
	trk    = _trk;
	x      = _newtrk->x;
	oldy   = trk->y;
	newy   = _newtrk->y;
	xsel   = _newtrk->xsel;
	sel    = _newtrk->sel;

	//Save data
	oldname    = trk->name;
	oldchannel = trk->channel;
	oldbank    = trk->bank;
	oldpatch   = trk->patch;
	oldtm      = trk->trackMode();

	oldstring  = trk->string;
	oldfrets   = trk->frets;

	for (int i = 0; i < trk->string; i++)
		oldtune[i] = trk->tune[i];

	newname    = _newtrk->name;
	newchannel = _newtrk->channel;
	newbank    = _newtrk->bank;
	newpatch   = _newtrk->patch;
	newtm      = _newtrk->trackMode();

	newstring  = _newtrk->string;
	newfrets   = _newtrk->frets;

	for (int i = 0; i < _newtrk->string; i++)
		newtune[i] = _newtrk->tune[i];
}

void SongView::SetTrackPropCommand::execute()
{
	trk->x = x;
	trk->y = newy;
	trk->xsel = xsel;
	trk->sel  = sel;

	trk->name    = newname;
	trk->channel = newchannel;
	trk->bank    = newbank;
	trk->patch   = newpatch;
	trk->setTrackMode(newtm);

	trk->string = newstring;
	trk->frets  = newfrets;

	for (int i = 0; i < newstring; i++)
		trk->tune[i] = newtune[i];

	tv->selectTrack(trk); // artificially needed to emit track selection
	tl->updateList();
	tp->updateList();
}

void SongView::SetTrackPropCommand::unexecute()
{
	trk->x = x;
	trk->y = oldy;
	trk->xsel = xsel;
	trk->sel  = sel;

	trk->name    = oldname;
	trk->channel = oldchannel;
	trk->bank    = oldbank;
	trk->patch   = oldpatch;
	trk->setTrackMode(oldtm);

	trk->string = oldstring;
	trk->frets  = oldfrets;

	for (int i = 0; i < oldstring; i++)
		trk->tune[i] = oldtune[i];

	tv->selectTrack(trk); // artificially needed to emit track selection
	tl->updateList();
	tp->updateList();
}

SongView::InsertTabsCommand::InsertTabsCommand(TrackView *_tv, TabTrack *_trk, TabTrack *_tabs)
	: KNamedCommand(i18n("Insert from clipboard"))
{
	trk  = _trk;
	tv   = _tv;
	tabs = _tabs;
	x    = trk->x;
	y    = trk->y;
	xsel = trk->xsel;
	sel  = trk->sel;
}

void SongView::InsertTabsCommand::execute()
{
	trk->x = x;
	trk->y = y;

	uint col = tabs->c.size();
	uint _x  = trk->x;

	for (uint i = 1; i <= col; i++)
		trk->insertColumn(1);

	for (uint i = 0; i <= col - 1; i++) {
		trk->c[_x].l = tabs->c[i].l;
		trk->c[_x].flags = tabs->c[i].flags;

		for (uint k = 0; k < trk->string; k++) {
			trk->c[_x].a[k] = tabs->c[i].a[k];
			trk->c[_x].e[k] = tabs->c[i].e[k];
		}
		_x++;
	}
	tv->update();
}

void SongView::InsertTabsCommand::unexecute()
{
	trk->x = x;
	trk->y = y;
	trk->xsel = xsel;
	trk->sel  = sel;

	uint col = tabs->c.size();
	trk->removeColumn(col);

	tv->update();
}

