/*
  Undo/Redo commands for TrackView
*/

#include "trackviewcommands.h"
#include "tabtrack.h"
#include "trackview.h"
#include "strumlib.h"

#include <klocale.h>

//GREYFIX
#include <stdio.h>

extern strummer lib_strum[];

SetLengthCommand::SetLengthCommand(TrackView *_tv, TabTrack *&_trk, int l):
	KNamedCommand(i18n("Set duration"))
{
	QString cmd(i18n("Set duration to %1"));
	QString dur;

	switch (l){
	case 15:  // 1/32
		dur = "1/32";
		break;
	case 30:  // 1/16
		dur = "1/16";
		break;
	case 60:  // 1/8
		dur = "1/8";
		break;
	case 120: // 1/4
		dur = "1/4";
		break;
	case 240: // 1/2
		dur = "1/2";
		break;
	case 480: // whole
		dur = i18n("whole");
		break;
	}

	setName(cmd.arg(dur));

	//Store important data
	trk = _trk;
	tv = _tv;
	len = l;
	oldlen = trk->c[trk->x].l;
	x = trk->x;
	y = trk->y;
	xsel = trk->xsel;
	sel = trk->sel;
}

SetLengthCommand::~SetLengthCommand()
{
}

void SetLengthCommand::execute()
{
	trk->x = x;
	trk->y = y;
	trk->sel = FALSE;
	trk->c[x].l = len;
	tv->repaintCurrentCell();
}

void SetLengthCommand::unexecute()
{
	trk->x = x;
	trk->y = y;
	trk->xsel = xsel;
	trk->sel = sel;
	trk->c[x].l = oldlen;
	tv->repaintCurrentCell();
}

InsertTabCommand::InsertTabCommand(TrackView *_tv, TabTrack *&_trk, int t):
	KNamedCommand(i18n("Insert tab"))
{
	setName(i18n("Insert tab %1").arg(QString::number(t)));
	//Store important data
	trk = _trk;
	tv = _tv;
	x = trk->x;
	y = trk->y;
	xsel = trk->xsel;
	sel = trk->sel;
    totab = t;
	oldtab = trk->c[x].a[y];
}

InsertTabCommand::~InsertTabCommand()
{

}

void InsertTabCommand::execute()
{
	trk->x = x;
	trk->y = y;
	trk->sel = FALSE;
	trk->c[x].a[y] = totab;
	tv->repaintCurrentCell();
}

void InsertTabCommand::unexecute()
{
	trk->x = x;
	trk->y = y;
	trk->xsel = xsel;
	trk->sel = sel;
	trk->c[x].a[y] = oldtab;
	tv->repaintCurrentCell();
}

MoveFingerCommand::MoveFingerCommand(TrackView *_tv, TabTrack *&_trk, int _from,
									 int _to, int _tune): KNamedCommand(i18n("Transpose"))
{
    from = _from;
	to = _to;
	tune = _tune;
	trk = _trk;
	tv = _tv;
	x = trk->x;
	y = trk->y;
	xsel = trk->xsel;
	sel = trk->sel;
	oldtune = trk->c[x].a[from];

	if (to < from)
		setName(i18n("Transpose down"));
	else setName(i18n("Transpose up"));
}

MoveFingerCommand::~MoveFingerCommand()
{
}

void MoveFingerCommand::execute()
{
	trk->c[x].a[from] = -1;
	trk->c[x].a[to] = tune;

    // ...also for the effect parameter
	trk->c[x].e[to] = trk->c[x].e[from];
	trk->c[x].e[from] = 0;

	trk->x = x;
	trk->y = to;
	trk->sel = FALSE;

	tv->repaintCurrentCell();
}

void MoveFingerCommand::unexecute()
{
 	trk->c[x].a[from] = oldtune;
	trk->c[x].a[to] = -1;

    // ...also for the effect parameter
	trk->c[x].e[from] = trk->c[x].e[to];
	trk->c[x].e[to] = 0;

	trk->y = y;
	trk->x = x;
	trk->xsel = xsel;
	trk->sel = sel;

	tv->repaintCurrentCell();
}

AddFXCommand::AddFXCommand(TrackView *_tv, TabTrack *&_trk, char _fx):
	KNamedCommand(i18n("Add effect"))
{
	trk = _trk;
	tv = _tv;
	x = trk->x;
	y = trk->y;
	xsel = trk->xsel;
	sel = trk->sel;
	fx = _fx;

	QString cmd(i18n("Add %1 effect"));
	QString p_fx;

	switch (fx) {
	case EFFECT_HARMONIC: p_fx = i18n("nat. harmonic");
		break;
	case EFFECT_ARTHARM: p_fx = i18n("art. harmonic");
		break;
	case EFFECT_LEGATO: p_fx = i18n("legato");
		break;
	case EFFECT_SLIDE: p_fx = i18n("slide");
		break;
	case EFFECT_LETRING: p_fx = i18n("let ring");
		break;
	case EFFECT_STOPRING: p_fx = i18n("stop ring");
		break;
	default:
		break;
	}

	setName(cmd.arg(p_fx));
}

AddFXCommand::~AddFXCommand()
{
}

void AddFXCommand::execute()
{
	trk->x = x;
	trk->y = y;
    trk->addFX(fx);
	tv->repaintCurrentCell();
}

void AddFXCommand::unexecute()
{
	trk->x = x;
	trk->y = y;
	trk->xsel = xsel;
	trk->sel = sel;

    trk->addFX(fx);
	tv->repaintCurrentCell();
}

SetFlagCommand::SetFlagCommand(TrackView *_tv, TabTrack *&_trk, int _flag):
	KNamedCommand(i18n("Set flag"))
{
    flag = _flag;
	trk = _trk;
	tv = _tv;
	x = trk->x;
	y = trk->y;
	xsel = trk->xsel;
	sel = trk->sel;
	oldflag = trk->c[x].flags;

	QString cmd(i18n("Set flag"));

	switch (flag) {
	case FLAG_PM: cmd = i18n("Palm muting");
		break;
	case FLAG_DOT: cmd = i18n("Dotted note");
		break;
	case FLAG_ARC: cmd = i18n("Link with previous column");
		for (uint i = 0; i < MAX_STRINGS; i++) {
			a[i] = trk->c[x].a[i];
			e[i] = trk->c[x].e[i];
		}
		break;
	case DEAD_NOTE: cmd = i18n("Dead note");
		oldtab = trk->c[x].a[y];
		break;
	case FLAG_TRIPLET: cmd = i18n("Triplet");
		break;
	}

	setName(cmd);
}

SetFlagCommand::~SetFlagCommand()
{
}

void SetFlagCommand::execute()
{
	trk->x = x;
	trk->y = y;
	trk->sel = FALSE;

	if (flag == DEAD_NOTE) {
		if (trk->c[x].flags & FLAG_ARC)
			trk->c[x].flags -= FLAG_ARC;
		trk->c[x].a[y] = DEAD_NOTE;
	} else {
		trk->c[x].flags ^= flag;
		if (flag == FLAG_ARC)
			for (uint i = 0; i < MAX_STRINGS; i++) {
				trk->c[x].a[i] = -1;
				trk->c[x].e[i] = 0;
			}
	}

	tv->repaintCurrentCell();
}

void SetFlagCommand::unexecute()
{
	trk->x = x;
	trk->y = y;
	trk->xsel = xsel;
	trk->sel = sel;

	if (flag == DEAD_NOTE) {
		trk->c[x].flags = oldflag;
		trk->c[x].a[y] = oldtab;
	} else {
		trk->c[x].flags ^= flag;
		if (flag == FLAG_ARC)
			for (uint i = 0; i < MAX_STRINGS; i++) {
				trk->c[x].a[i] = a[i];
				trk->c[x].e[i] = e[i];
			}
	}

	tv->repaintCurrentCell();
}

DeleteNoteCommand::DeleteNoteCommand(TrackView *_tv, TabTrack *&_trk):
	KNamedCommand(i18n("Delete note"))
{
	trk = _trk;
	tv = _tv;
	x = trk->x;
	y = trk->y;
	xsel = trk->xsel;
	sel = trk->sel;
	a = trk->c[x].a[y];
	e = trk->c[x].e[y];

	setName(i18n("Delete note %1").arg(a));
}

DeleteNoteCommand::~DeleteNoteCommand()
{
}

void DeleteNoteCommand::execute()
{
	trk->x = x;
	trk->y = y;
	trk->c[x].a[y] = -1;
	trk->c[x].e[y] = 0;
	trk->sel = FALSE;
	tv->repaintCurrentCell();
}

void DeleteNoteCommand::unexecute()
{
	trk->x = x;
	trk->y = y;
	trk->xsel = xsel;
	trk->sel = sel;
	trk->c[x].a[y] = a;
	trk->c[x].e[y] = e;
	tv->repaintCurrentCell();
}

AddColumnCommand::AddColumnCommand(TrackView *_tv, TabTrack *&_trk):
	KNamedCommand(i18n("Add column"))
{
	trk = _trk;
	tv = _tv;
	x = trk->x;
	y = trk->y;
	xsel = trk->xsel;
	sel = trk->sel;
	addBar = trk->currentBarDuration() == trk->maxCurrentBarDuration();
}

AddColumnCommand::~AddColumnCommand()
{
}

void AddColumnCommand::execute()
{
	trk->x = x;
	trk->y = y;
	trk->xb = trk->b.size() - 1;
	trk->c.resize(trk->c.size()+1);
	trk->x++;
	for (uint i = 0; i < MAX_STRINGS; i++) {
		trk->c[trk->x].a[i] = -1;
		trk->c[trk->x].e[i] = 0;
	}
	trk->c[trk->x].l = trk->c[trk->x - 1].l;
	trk->c[trk->x].flags = 0;

	// Check if we need to close this bar and open a new one
	if (addBar) {
		trk->b.resize(trk->b.size()+1);
		trk->xb++;
		trk->b[trk->xb].start = trk->x;
		trk->b[trk->xb].time1 = trk->b[trk->xb-1].time1;
		trk->b[trk->xb].time2 = trk->b[trk->xb-1].time2;
	}

	tv->ensureCurrentVisible();
	tv->updateRows();
	tv->repaintCurrentCell();
}

void AddColumnCommand::unexecute()
{
	trk->x = x + 1;
	trk->y = y;
	trk->removeColumn(1);
	trk->x = x;
	trk->xsel = xsel;
	trk->sel = sel;

	tv->ensureCurrentVisible();
	tv->updateRows();
	tv->repaintCurrentCell();
}

DeleteColumnCommand::DeleteColumnCommand(TrackView *_tv, TabTrack *&_trk):
	KNamedCommand(i18n("Delete column"))
{
	trk = _trk;
	tv = _tv;
	x = trk->x;
	y = trk->y;
	xsel = trk->xsel;
	sel = trk->sel;

	p_all = FALSE;
	p_start = x;
	p_delta = 1;

	if ((trk->c.size() > 1) && (trk->sel)) {
		if (trk->x <= trk->xsel) {
			p_delta = trk->xsel - trk->x;
			p_start = trk->x;
		} else {
			p_delta = trk->x - trk->xsel;
			p_start = trk->xsel;
		}

		p_delta++;
	}

	if (p_delta > 1)
		setName(i18n("Delete %1 columns").arg(QString::number(p_delta)));
	p_del = p_delta;

	c.resize(1);
}

//This is the constructor called by cutToClipboard
DeleteColumnCommand::DeleteColumnCommand(QString name, TrackView *_tv, TabTrack *&_trk):
	KNamedCommand(name)
{
	trk = _trk;
	tv = _tv;
	x = trk->x;
	y = trk->y;
	xsel = trk->xsel;
	sel = trk->sel;

	p_all = FALSE;
	p_start = x;
	p_delta = 1;

	if ((trk->c.size() > 1) && (trk->sel)) {
		if (trk->x <= trk->xsel) {
			p_delta = trk->xsel - trk->x;
			p_start = trk->x;
		} else {
			p_delta = trk->x - trk->xsel;
			p_start = trk->xsel;
		}

		p_delta++;
	}

	p_del = p_delta;

	c.resize(1);
}

DeleteColumnCommand::~DeleteColumnCommand()
{
}

void DeleteColumnCommand::execute()
{
	p_all = FALSE;
	trk->x = x;
	trk->y = y;

	//Save column data
	c.resize(p_del);
	for (uint i = 0; i < c.size() - 1; i++)
		for (uint k = 0; k < MAX_STRINGS; k++) {
			c[i].a[k] = -1;
			c[i].e[k] = 0;
		}

	int _s = p_start;

	for (uint i = 0; i < p_del; i++) {
		c[i].l = trk->c[_s].l;
		c[i].flags = trk->c[_s].flags;

		for (uint k = 0; k < trk->string; k++) {
			c[i].a[k] = trk->c[_s].a[k];
			c[i].e[k] = trk->c[_s].e[k];
		}
		_s++;
	}

	//Delete columns
	if (trk->c.size() > 1) {
		if ((trk->sel) && (p_delta == trk->c.size())) {
			p_delta--;
			p_all = TRUE;
		}

		trk->removeColumn(p_delta);

		trk->sel = FALSE;
		trk->xsel = 0;

		tv->updateRows();
	} else
		p_all = (trk->c.size() == 1);

	//If all of the track was selected then
	//delete all notes of the first column
	if (p_all) {
		trk->x = 0;
		for (uint i = 0; i < MAX_STRINGS; i++) {
			trk->c[trk->x].a[i] = -1;
			trk->c[trk->x].e[i] = 0;
		}
		trk->sel = FALSE;
		trk->xsel = 0;
	}

	tv->update();
	tv->repaintCurrentCell();
}

void DeleteColumnCommand::unexecute()
{
	//Only the first column was deleted
	if ((p_del == 1) && p_all) {
		trk->x = 0;
		trk->y = 0;
		trk->c[0].l = c[0].l;
		trk->c[0].flags = c[0].flags;

		for (uint k = 0; k < trk->string; k++) {
			trk->c[0].a[k] = c[0].a[k];
			trk->c[0].e[k] = c[0].e[k];
		}
	} else {
		//Whole track was deleted
		if ((p_del > 1) && p_all) {
			trk->x = p_start;
			for (uint k = 0; k < p_del - 1; k++)
				trk->insertColumn(1);            //only "1" works correct
			for (uint k = 0; k < p_del; k++) {
				trk->c[k].l = c[k].l;
				trk->c[k].flags = c[k].flags;

				for (uint i = 0; i < trk->string; i++) {
					trk->c[k].a[i] = c[k].a[i];
					trk->c[k].e[i] = c[k].e[i];
				}
			}
		} else { //One or more columns are deleted
			// Test if we deleted the last columns
			bool m_add = (p_start == trk->c.size());
			trk->x = p_start;

			if (m_add) { //Add first one column at end of track
				trk->c.resize(trk->c.size()+1);
				for (uint i = 0; i < MAX_STRINGS; i++) {
					trk->c[trk->x].a[i] = -1;
					trk->c[trk->x].e[i] = 0;
				}
				trk->c[trk->x].l = trk->c[trk->x - 1].l;
				trk->c[trk->x].flags = 0;
				trk->x++;
			}

			//Now insert columns if needed
			for(uint k = 0; k < (m_add ? p_del - 1 : p_del); k++)
				trk->insertColumn(1);           //only "1" works correct

			//Copy data
			int _s = p_start;
			for (uint k = 0; k < p_del; k++) {
				trk->c[_s].l = c[k].l;
				trk->c[_s].flags = c[k].flags;

				for (uint i = 0; i < trk->string; i++) {
					trk->c[_s].a[i] = c[k].a[i];
					trk->c[_s].e[i] = c[k].e[i];
				}
				_s++;
			}
		}
	}
	trk->x = x;
	trk->y = y;
	trk->xsel = xsel;
	trk->sel = sel;
	tv->updateRows();
	tv->update();
	tv->repaintCurrentCell();
}

SetTimeSigCommand::SetTimeSigCommand(TrackView *_tv, TabTrack *&_trk, bool _toend,
									 int _time1, int _time2): KNamedCommand(i18n("Set time sig."))
{
	trk = _trk;
	tv = _tv;
	x = trk->x;
	y = trk->y;
	xb = trk->xb;
	xsel = trk->xsel;
	sel = trk->sel;
	toend = _toend;
	time1 = _time1;
	time2 = _time2;

	b.resize(trk->b.size());
	for (uint i = 0; i < trk->b.size(); i++)
		b[i] = trk->b[i];
}

SetTimeSigCommand::~SetTimeSigCommand()
{
}

void SetTimeSigCommand::execute()
{
	// Sophisticated construction to mark all or only one bar with
	// new sig, depending on user's selection of checkbox

	for (uint i = xb; i < (toend ? trk->b.size() : trk->xb+1); i++) {
		trk->b[i].time1 = time1;
		trk->b[i].time2 = time2;
	}

	trk->sel = FALSE;
	tv->update();
	tv->repaintCurrentCell(); //for emit paneChanded
}

void SetTimeSigCommand::unexecute()
{
	int k;
	if (b.size() <= trk->b.size())
		k = b.size();
	else k = trk->b.size();

	for (int i = 0; i < k; i++)
		trk->b[i] = b[i];

	trk->x = x;
	trk->y = y;
	trk->xsel = xsel;
	trk->sel = sel;
	trk->xb = xb;
	tv->update();
	tv->repaintCurrentCell(); //for emit paneChanded
}

InsertColumnCommand::InsertColumnCommand(TrackView *_tv, TabTrack *&_trk)
	: KNamedCommand(i18n("Insert column"))
{
	trk = _trk;
	tv = _tv;
	x = trk->x;
	y = trk->y;
	xsel = trk->xsel;
	sel = trk->sel;
}

InsertColumnCommand::~InsertColumnCommand()
{
}

void InsertColumnCommand::execute()
{
	trk->x = x;
	trk->y = y;
	trk->insertColumn(1);
	trk->sel = FALSE;
	tv->update();
	tv->repaintCurrentCell();
}

void InsertColumnCommand::unexecute()
{
	trk->x = x;
	trk->y = y;
	trk->xsel = xsel;
	trk->sel = sel;
	trk->removeColumn(1);
	tv->update();
    tv->repaintCurrentCell();
}

InsertStrumCommand::InsertStrumCommand(TrackView *_tv, TabTrack *&_trk, int _sch, int *_chord)
	: KNamedCommand(i18n("Insert strum"))
{
	trk   = _trk;
	tv    = _tv;
	x     = trk->x;
	y     = trk->y;
	xsel  = trk->xsel;
	sel   = trk->sel;
	sch   = _sch;

	c.resize(1);

	for (int i = 0; i < MAX_STRINGS; i++) {
		c[0].a[i] = -1;
		c[0].e[i] = 0;
	}

	c[0].l     = trk->c[x].l;
	c[0].flags = trk->c[x].flags;

	for (int i = 0; i < trk->string; i++) {
		chord[i]  = _chord[i];
		c[0].a[i] = trk->c[x].a[i];
		c[0].e[i] = trk->c[x].e[i];
	}

	if (sch == 0)
		setName(i18n("Insert Chord"));
}

InsertStrumCommand::~InsertStrumCommand()
{
}

void InsertStrumCommand::execute()
{
	trk->x = x;
	trk->y = y;
	trk->sel = FALSE;

	toadd = 0;
	c.resize(1);

	if (sch == 0) { // Special "chord" scheme
		for (int i = 0; i < trk->string; i++)
			trk->c[x].a[i] = chord[i];
	} else { // Normal strum pattern scheme
		int mask, r;
		bool inv;

		for (int j = 1; lib_strum[sch].len[j]; j++) {
			c.resize(c.size() + 1);
			for (int k = 0; k < MAX_STRINGS; k++) {
				c[j].a[k] = -1;
				c[j].e[k] = 0;
			}
		}

		for (int j = 0; lib_strum[sch].len[j]; j++) {
			if (x + j + 1 > trk->c.size()) {
				trk->c.resize(trk->c.size() + 1);
				toadd++;
				for (int k = 0; k < MAX_STRINGS; k++)
					trk->c[x + j].a[k] = -1;
			}
			c[j].flags = trk->c[x + j].flags;
			trk->c[x + j].flags = 0;
			inv = lib_strum[sch].len[j] < 0;
			c[j].l = trk->c[x + j].l;
			trk->c[x + j].l = inv ? -lib_strum[sch].len[j] : lib_strum[sch].len[j];

			mask = lib_strum[sch].mask[j];

			if (mask > 0) { // Treble notation
				r = 0; // "Real" string counter
				for (int i = trk->string - 1; i >= 0; i--) {
					c[j].a[i] = trk->c[x + j].a[i];
					c[j].e[i] = trk->c[x + j].e[i];
					if (inv)
						trk->c[x + j].a[i] = (mask & (1 << r)) ? -1 : chord[i];
					else
						trk->c[x + j].a[i] = (mask & (1 << r)) ? chord[i] : -1;
					trk->c[x + j].e[i] = 0;
					if (chord[i] != -1)
						r++;
				}
			} else { // Bass notation
				mask = -mask;
				r = 0; // "Real" string counter
				for (int i = 0; i < trk->string; i++) {
					c[j].a[i] = trk->c[x + j].a[i];
					c[j].e[i] = trk->c[x + j].e[i];
					if (inv)
						trk->c[x + j].a[i] = (mask & (1 << r)) ? -1 : chord[i];
					else
						trk->c[x + j].a[i] = (mask & (1 << r)) ? chord[i] : -1;
					trk->c[x + j].e[i] = 0;
					if (chord[i] != -1)
						r++;
				}
			}
		}
	}

	tv->update();
	tv->repaintCurrentCell();
}

void InsertStrumCommand::unexecute()
{
	trk->x = x;
	trk->y = y;
	trk->xsel = xsel;
	trk->sel = sel;

	if (toadd > 0) {
		trk->x++;
		for (int i = 0; i < toadd; i++)
			trk->removeColumn(1);
		trk->x = x;
	}

	for (int k = 0; k < c.size() - toadd; k++) {
		for (int i = 0; i < trk->string; i++) {
			trk->c[x + k].a[i] = c[k].a[i];
			trk->c[x + k].e[i] = c[k].e[i];
		}

		trk->c[x + k].l     = c[k].l;
		trk->c[x + k].flags = c[k].flags;
	}

	tv->update();
	tv->repaintCurrentCell();
}

InsertRhythm::InsertRhythm(TrackView *_tv, TabTrack *&_trk, QListBox *quantized)
	: KNamedCommand(i18n("Insert rhythm"))
{
	trk = _trk;
	tv  = _tv;
	x   = trk->x;

	newdur.resize(quantized->count() - 1);
	for (int i = 1; i < quantized->count(); i++)
		newdur[i - 1] = quantized->text(i).toInt();
}

void InsertRhythm::execute()
{
	trk->x = x;

	if (x + newdur.size() > trk->c.size()) {
		int end = trk->c.size();
		trk->c.resize(x + newdur.size());
		for (int i = end; i < trk->c.size(); i++) {
			for (uint j = 0; j < MAX_STRINGS; j++) {
				trk->c[i].a[j] = -1;
				trk->c[i].e[j] = 0;
			}
			trk->c[i].flags = 0;
		}
		olddur.resize(end - x);
	} else {
		olddur.resize(newdur.size());
	}

	for (int i = 0; i < newdur.size(); i++) {
		if (i < olddur.size())
			olddur[i] = trk->c[x + i].fullDuration();
		trk->c[x + i].setFullDuration(newdur[i]);
	}

	tv->repaintContents();
}

void InsertRhythm::unexecute()
{
	trk->x = x;

	for (int i = 0; i < olddur.size(); i++)
		trk->c[x + i].setFullDuration(olddur[i]);

	trk->c.resize(x + olddur.size());

	tv->repaintContents();
}
