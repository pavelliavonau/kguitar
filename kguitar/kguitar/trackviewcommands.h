/*
  Undo/Redo commands for TrackView
*/
#ifndef TRACKVIEWCOMMANDS_H
#define TRACKVIEWCOMMANDS_H

#include <kcommand.h>
#include <klocale.h>

// Set the duration for the notes
class SetLengthCommand : public KCommand
{
public:
	SetLengthCommand(int l);
	virtual ~SetLengthCommand();

	virtual void execute();
	virtual void unexecute();

private:
	int len, //Length
		idx, //Track index in QList (TabSong)
		x;   //Position

};



#endif

