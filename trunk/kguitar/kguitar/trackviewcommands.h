/*
  Undo/Redo commands for TrackView
*/
#ifndef TRACKVIEWCOMMANDS_H
#define TRACKVIEWCOMMANDS_H

#include "global.h"
#include "tabtrack.h"

#include <kcommand.h>
#include <klocale.h>

class TabTrack;
class TrackView;

// Set the duration for the notes
class SetLengthCommand : public KCommand
{
public:
	SetLengthCommand(TrackView *_tv, TabTrack *&_trk, int l);
	virtual ~SetLengthCommand();

	virtual void execute();
	virtual void unexecute();

private:
	int len, oldlen,  //Length
		x, y, xsel;   //Position
	bool sel;
	TabTrack *trk;
	TrackView *tv;
};


// Insert tabs from keyboard
class InsertTabCommand : public KCommand
{
public:
	InsertTabCommand(TrackView *_tv, TabTrack *&_trk, int t);
	virtual ~InsertTabCommand();

	virtual void execute();
	virtual void unexecute();

private:
	int totab, oldtab,  //Tab
		x, y, xsel;      //Position
	bool sel;
	TabTrack *trk;
	TrackView *tv;
};

// Moves the finger
class MoveFingerCommand : public KCommand
{
public:
	MoveFingerCommand(TrackView *_tv, TabTrack *&_trk, int _from, int _to, int _tune);
	virtual ~MoveFingerCommand();

	virtual void execute();
	virtual void unexecute();

private:
	int from, to, oldtune, tune, x, y, xsel;
	bool sel;
	TabTrack *trk;
	TrackView *tv;
};

// Add FX
class AddFXCommand : public KCommand
{
public:
	AddFXCommand(TrackView *_tv, TabTrack *&_trk, char _fx);
	virtual ~AddFXCommand();

	virtual void execute();
	virtual void unexecute();

private:
	int x, y, xsel;
	char fx;
	bool sel;
	TabTrack *trk;
	TrackView *tv;
};

// Set a flag
class SetFlagCommand : public KCommand
{
public:
	SetFlagCommand(TrackView *_tv, TabTrack *&_trk, int _flag);
	virtual ~SetFlagCommand();

	virtual void execute();
	virtual void unexecute();

private:
	int x, y, xsel, flag, oldflag;
	char a[MAX_STRINGS];
	char e[MAX_STRINGS];
	char oldtab;
	bool sel;
	TabTrack *trk;
	TrackView *tv;
};

// Delete Note
class DeleteNoteCommand : public KCommand
{
public:
	DeleteNoteCommand(TrackView *_tv, TabTrack *&_trk);
	virtual ~DeleteNoteCommand();

	virtual void execute();
	virtual void unexecute();

private:
	int x, y, xsel;
	char a, e;
//	char e;
	bool sel;
	TabTrack *trk;
	TrackView *tv;
};

// Add a column at end of track
class AddColumnCommand : public KCommand
{
public:
	AddColumnCommand(TrackView *_tv, TabTrack *&_trk);
	virtual ~AddColumnCommand();

	virtual void execute();
	virtual void unexecute();

private:
	int x, y, xsel;
	bool sel;
	TabTrack *trk;
	TrackView *tv;
};

// Delete column
class DeleteColumnCommand : public KCommand
{
public:
	DeleteColumnCommand(TrackView *_tv, TabTrack *&_trk);
	DeleteColumnCommand(QString name, TrackView *_tv, TabTrack *&_trk);
	virtual ~DeleteColumnCommand();

	virtual void execute();
	virtual void unexecute();

private:
	int x, y, xsel;
	uint p_delta, p_del, p_start;
	QArray<TabColumn> c;
	bool p_all, sel;
	TabTrack *trk;
	TrackView *tv;
};

// Set time sig
class SetTimeSigCommand : public KCommand
{
public:
	SetTimeSigCommand(TrackView *_tv, TabTrack *&_trk, bool _toend, int _time1, int _time2);
	virtual ~SetTimeSigCommand();

	virtual void execute();
	virtual void unexecute();

private:
	int x, y, xb, xsel, time1, time2;
	bool sel, toend;
	QArray<TabBar> b;
	TabTrack *trk;
	TrackView *tv;
};

// Insert a column at cursor pos
class InsertColumnCommand : public KCommand
{
public:
	InsertColumnCommand(TrackView *_tv, TabTrack *&_trk);
	virtual ~InsertColumnCommand();

	virtual void execute();
	virtual void unexecute();

private:
	int x, y, xsel;
	bool sel;
	TabTrack *trk;
	TrackView *tv;
};
#endif

