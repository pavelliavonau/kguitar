/*
  Undo/Redo commands for TrackView
*/

#include "trackviewcommands.h"

#include <klocale.h>
#include <kdebug.h>

SetLengthCommand::SetLengthCommand(int l) : KCommand(i18n("Set duration"))
{
	QString cmd(i18n("Set duration to %1"));
	QString dur;

	switch (l){
	case 15:  // 1/32
		dur = i18n("1/32");
		break;
	case 30:  // 1/16
		dur = i18n("1/16");
		break;
	case 60:  // 1/8
		dur = i18n("1/8");
		break;
	case 120: // 1/4
		dur = i18n("1/4");
		break;
	case 240: // 1/2
		dur = i18n("1/2");
		break;
	case 480: // whole
		dur = i18n("whole");
		break;
	}

	setName(cmd.arg(dur));
}

SetLengthCommand::~SetLengthCommand()
{
}

void SetLengthCommand::execute()
{
	kdDebug() << "SetLengthCommand::execute() => " << name() << endl;
//	curt->c[curt->x].l = l;
//	repaintCurrentCell();
}

void SetLengthCommand::unexecute()
{
	kdDebug() << "SetLengthCommand::unexecute() => " << name() << endl;
}
