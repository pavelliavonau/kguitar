/*
  Undo/Redo commands for SongView
*/
#ifndef SONGVIEWCOMMANDS_H
#define SONGVIEWCOMMANDS_H

#include "global.h"
#include "tabtrack.h"

#include <kcommand.h>
#include <klocale.h>

class TabSong;
class TabTrack;
class TrackView;
class TrackList;
class TrackPane;

// Set the song properties
class SetSongPropCommand : public KCommand
{
public:
	SetSongPropCommand(TabSong* _song, QString _title, QString _author, QString _trans, QString _com);
	virtual ~SetSongPropCommand();

	virtual void execute();
	virtual void unexecute();

private:
	QString title, author, transcriber, comments,
		oldtitle, oldauthor, oldtranscriber, oldcomments;
	TabSong* song;
};

// Set track properties
class SetTrackPropCommand : public KCommand
{
public:
	SetTrackPropCommand(TrackView *_tv, TrackList *_tl, TrackPane *_tp,
						TabTrack *_trk, TabTrack *_newtrk);
	virtual ~SetTrackPropCommand();

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
	TrackMode oldtm, newtm;
	TabTrack *trk;
	TrackView *tv;
	TrackList *tl;
	TrackPane *tp;
};

// Insert tabs
class InsertTabsCommand : public KCommand
{
public:
	InsertTabsCommand(TrackView *_tv, TabTrack *_trk, TabTrack *_tabs);
	virtual ~InsertTabsCommand();

	virtual void execute();
	virtual void unexecute();

private:
	int x, y, xsel;
	bool sel;
	TabTrack *trk, *tabs;
	TrackView *tv;
};
#endif

