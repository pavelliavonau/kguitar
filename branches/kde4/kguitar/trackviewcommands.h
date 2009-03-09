#ifndef TRACKVIEWCOMMANDS_H
#define TRACKVIEWCOMMANDS_H

#include "global.h"
#include "tabtrack.h"
#include "trackview.h"
#include <k3command.h>
//Added by qt3to4:
#include <Q3MemArray>

// Set the duration for the notes
class TrackView::SetLengthCommand: public KNamedCommand {
public:
	SetLengthCommand(TrackView *_tv, TabTrack *&_trk, int l);
	virtual ~SetLengthCommand() {};

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
class TrackView::InsertTabCommand: public KNamedCommand {
public:
	InsertTabCommand(TrackView *_tv, TabTrack *&_trk, int t);
	virtual ~InsertTabCommand() {};

	virtual void execute();
	virtual void unexecute();

private:
	int totab, oldtab,   //Tab
	    x, y, xsel;      //Position
	bool sel;
	TabTrack *trk;
	TrackView *tv;
};

// Moves the finger
class TrackView::MoveFingerCommand: public KNamedCommand {
public:
	MoveFingerCommand(TrackView *_tv, TabTrack *&_trk, int _from, int _to, int _tune);
	virtual ~MoveFingerCommand() {};

	virtual void execute();
	virtual void unexecute();

private:
	int from, to, oldtune, tune, x, y, xsel;
	bool sel;
	TabTrack *trk;
	TrackView *tv;
};

// Add FX
class TrackView::AddFXCommand: public KNamedCommand {
public:
	AddFXCommand(TrackView *_tv, TabTrack *&_trk, char _fx);
	virtual ~AddFXCommand() {};

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
class TrackView::SetFlagCommand: public KNamedCommand {
public:
	SetFlagCommand(TrackView *_tv, TabTrack *&_trk, int _flag);
	virtual ~SetFlagCommand() {};

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
class TrackView::DeleteNoteCommand : public KNamedCommand {
public:
	DeleteNoteCommand(TrackView *_tv, TabTrack *&_trk);
	virtual ~DeleteNoteCommand() {};

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
class TrackView::AddColumnCommand: public KNamedCommand {
public:
	AddColumnCommand(TrackView *_tv, TabTrack *&_trk);
	virtual ~AddColumnCommand() {};

	virtual void execute();
	virtual void unexecute();

private:
	int x, y, xsel;
	bool sel;
	bool addBar;
	TabTrack *trk;
	TrackView *tv;
};

// Delete column
class TrackView::DeleteColumnCommand: public KNamedCommand {
public:
	DeleteColumnCommand(TrackView *_tv, TabTrack *&_trk);
	DeleteColumnCommand(QString name, TrackView *_tv, TabTrack *&_trk);
	virtual ~DeleteColumnCommand() {};

	virtual void execute();
	virtual void unexecute();

private:
	int x, y, xsel;
	uint p_delta, p_del, p_start;
	Q3MemArray<TabColumn> c;
	bool p_all, sel;
	TabTrack *trk;
	TrackView *tv;
};

// Set time sig
class TrackView::SetTimeSigCommand : public KNamedCommand {
public:
	SetTimeSigCommand(TrackView *_tv, TabTrack *&_trk, bool _toend, int _time1, int _time2);
	virtual ~SetTimeSigCommand() {};

	virtual void execute();
	virtual void unexecute();

private:
	int x, y, xb, xsel, time1, time2;
	bool sel, toend;
	Q3MemArray<TabBar> b;
	TabTrack *trk;
	TrackView *tv;
};

// Insert a column at cursor pos
class TrackView::InsertColumnCommand: public KNamedCommand {
public:
	InsertColumnCommand(TrackView *_tv, TabTrack *&_trk);
	virtual ~InsertColumnCommand() {};

	virtual void execute();
	virtual void unexecute();

private:
	int x, y, xsel;
	bool sel;
	TabTrack *trk;
	TrackView *tv;
};

// Insert strum
class TrackView::InsertStrumCommand: public KNamedCommand {
public:
	InsertStrumCommand(TrackView *_tv, TabTrack *&_trk, int _sch, int *_chord);
	virtual ~InsertStrumCommand() {};

	virtual void execute();
	virtual void unexecute();

private:
	int sch, x, y, xsel, len, toadd;
	int chord[MAX_STRINGS];
	Q3MemArray<TabColumn> c;
	bool sel;
	TabTrack *trk;
	TrackView *tv;
};

class Q3ListBox;

// Insert rhythm from rhythmer
class TrackView::InsertRhythm: public KNamedCommand {
public:
	InsertRhythm(TrackView *_tv, TabTrack *&_trk, Q3ListBox *quantized);

	virtual void execute();
	virtual void unexecute();

private:
	int x;
	Q3MemArray<int> newdur, olddur;
	TabTrack *trk;
	TrackView *tv;
};

#endif
