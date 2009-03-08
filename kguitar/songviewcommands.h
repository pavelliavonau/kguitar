#ifndef SONGVIEWCOMMANDS_H
#define SONGVIEWCOMMANDS_H

#include "global.h"
#include "tabtrack.h"
#include "songview.h"

#include <kcommand.h>
#include <qmap.h>
#include <qstring.h>

class TabTrack;
class TrackView;
class TrackList;
class TrackPane;
class SongView;

/**
 * Undo/redo command to set song properties
 */
class SongView::SetSongPropCommand: public KNamedCommand {
public:
	SetSongPropCommand(SongView *_song, QMap<QString, QString> _info, int _tempo);
	virtual ~SetSongPropCommand() {};

	virtual void execute();
	virtual void unexecute();

private:
	QMap<QString, QString> info, oldinfo;
	int tempo, oldtempo;
	SongView *sv;
};

/**
 * Undo/redo command to set track properties
 */
class SongView::SetTrackPropCommand: public KNamedCommand {
public:
	SetTrackPropCommand(TrackView *_tv, TrackList *_tl, TrackPane *_tp,
						TabTrack *_trk, TabTrack *_newtrk);
	virtual ~SetTrackPropCommand() {};

	virtual void execute();
	virtual void unexecute();

private:
	int x, oldy, newy, xsel, oldbank, newbank;
	bool sel;
	uchar oldstring, oldfrets, oldchannel, oldpatch,
		newstring, newfrets, newchannel, newpatch;
	uchar oldtune[MAX_STRINGS];
	uchar newtune[MAX_STRINGS];
	QString oldname, newname;
	TabTrack::TrackMode oldtm, newtm;
	TabTrack *trk;
	TrackView *tv;
	TrackList *tl;
	TrackPane *tp;
};

/**
 * Undo/redo command to insert tabs
 */
class SongView::InsertTabsCommand: public KNamedCommand {
public:
	InsertTabsCommand(TrackView *_tv, TabTrack *_trk, TabTrack *_tabs);
	virtual ~InsertTabsCommand() {};

	virtual void execute();
	virtual void unexecute();

private:
	int x, y, xsel;
	bool sel;
	TabTrack *trk, *tabs;
	TrackView *tv;
};
#endif
