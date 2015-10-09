#ifndef TRACKVIEWCOMMANDS_H
#define TRACKVIEWCOMMANDS_H

#include "global.h"
#include "data/tabtrack.h"
#include "trackview.h"
#include <QUndoCommand>

// Set the duration for the notes
class TrackView::SetLengthCommand: public QUndoCommand {
public:
	SetLengthCommand(TrackView *_tv, TabTrack *&_trk, int l);
	virtual ~SetLengthCommand() {}

	virtual void redo();
	virtual void undo();

private:
	int len, oldlen,  //Length
		x, y, xsel;   //Position
	bool sel;
	TabTrack *trk;
	TrackView *tv;
};

// Insert tabs from keyboard
class TrackView::InsertTabCommand: public QUndoCommand {
public:
	InsertTabCommand(TrackView *_tv, TabTrack *&_trk, int t);
	virtual ~InsertTabCommand() {}

	virtual void redo();
	virtual void undo();

private:
	int totab, oldtab;   //Tab
	int x, y, xsel;      //Position
	int oldflags;
	bool sel;
	TabTrack *trk;
	TrackView *tv;
};

// Moves the finger
class TrackView::MoveFingerCommand: public QUndoCommand {
public:
	MoveFingerCommand(TrackView *_tv, TabTrack *&_trk, int _from, int _to, int _tune);
	virtual ~MoveFingerCommand() {}

	virtual void redo();
	virtual void undo();

private:
	int from, to, oldtune, tune, x, y, xsel;
	bool sel;
	TabTrack *trk;
	TrackView *tv;
};

// Add FX
class TrackView::AddFXCommand: public QUndoCommand {
public:
	AddFXCommand(TrackView *_tv, TabTrack *&_trk, char _fx);
	virtual ~AddFXCommand() {}

	virtual void redo();
	virtual void undo();

private:
	int x, y, xsel;
	char fx;
	bool sel;
	TabTrack *trk;
	TrackView *tv;
};

// Set a flag
class TrackView::SetFlagCommand: public QUndoCommand {
public:
	SetFlagCommand(TrackView *_tv, TabTrack *&_trk, int _flag);
	virtual ~SetFlagCommand() {}

	virtual void redo();
	virtual void undo();

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
class TrackView::DeleteNoteCommand : public QUndoCommand {
public:
	DeleteNoteCommand(TrackView *_tv, TabTrack *&_trk);
	virtual ~DeleteNoteCommand() {}

	virtual void redo();
	virtual void undo();

private:
	int x, y, xsel;
	char a, e;
//	char e;
	bool sel;
	TabTrack *trk;
	TrackView *tv;
};

// Add a column at end of track
class TrackView::AddColumnCommand: public QUndoCommand {
public:
	AddColumnCommand(TrackView *_tv, TabTrack *&_trk);
	virtual ~AddColumnCommand() {}

	virtual void redo();
	virtual void undo();

private:
	int x, y, xsel;
	bool sel;
	bool addBar;
	TabTrack *trk;
	TrackView *tv;
};

// Delete column
class TrackView::DeleteColumnCommand: public QUndoCommand {
public:
	DeleteColumnCommand(TrackView *_tv, TabTrack *&_trk);
	DeleteColumnCommand(QString name, TrackView *_tv, TabTrack *&_trk);
	virtual ~DeleteColumnCommand() {}

	virtual void redo();
	virtual void undo();

private:
	int x, y, xsel;
	uint p_delta, p_del, p_start;
	QVector<TabColumn> c;
	bool p_all, sel;
	TabTrack *trk;
	TrackView *tv;
};

// Set time sig
class TrackView::SetTimeSigCommand : public QUndoCommand {
public:
	SetTimeSigCommand(TrackView *_tv, TabTrack *&_trk, bool _toend, int _time1, int _time2);
	virtual ~SetTimeSigCommand() {}

	virtual void redo();
	virtual void undo();

private:
	int x, y, xb, xsel, time1, time2;
	bool sel, toend;
	QVector<TabBar> b;
	TabTrack *trk;
	TrackView *tv;
};

// Insert a column at cursor pos
class TrackView::InsertColumnCommand: public QUndoCommand {
public:
	InsertColumnCommand(TrackView *_tv, TabTrack *&_trk);
	virtual ~InsertColumnCommand() {}

	virtual void redo();
	virtual void undo();

private:
	int x, y, xsel;
	bool sel;
	TabTrack *trk;
	TrackView *tv;
};

// Insert strum
class TrackView::InsertStrumCommand: public QUndoCommand {
public:
	InsertStrumCommand(TrackView *_tv, TabTrack *&_trk, int _sch, int *_chord);
	virtual ~InsertStrumCommand() {}

	virtual void redo();
	virtual void undo();

private:
	int sch, x, y, xsel, len, toadd;
	int chord[MAX_STRINGS];
	QVector<TabColumn> c;
	bool sel;
	TabTrack *trk;
	TrackView *tv;
};

// Insert rhythm from rhythmer
class TrackView::InsertRhythm: public QUndoCommand {
public:
	InsertRhythm(TrackView *_tv, TabTrack *&_trk, QList<int> quantized);

	virtual void redo();
	virtual void undo();

private:
	int x;
	QList<int> newdur, olddur;
	TabTrack *trk;
	TrackView *tv;
};

#endif
