/*
  Undo/Redo commands for TrackView
*/

#include "trackviewcommands.h"
#include "strumlib.h"

#include <klocale.h>
#include <QList>
#include "data/tabsong.h"
#include <QAbstractProxyModel>

extern strummer lib_strum[];

TrackView::SetLengthCommand::SetLengthCommand(TrackView *_tv, TabTrack *&_trk, int l):
	QUndoCommand(i18n("Set duration"))
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

	setText(cmd.arg(dur));

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

void TrackView::SetLengthCommand::redo()
{
	trk->x = x;
	trk->y = y;
	trk->sel = FALSE;
	trk->c[x].l = len;
	tv->repaintCurrentBar();

	emit tv->songChanged();
}

void TrackView::SetLengthCommand::undo()
{
	trk->x = x;
	trk->y = y;
	trk->xsel = xsel;
	trk->sel = sel;
	trk->c[x].l = oldlen;
	tv->repaintCurrentBar();
}

TrackView::InsertTabCommand::InsertTabCommand(TrackView *_tv, TabTrack *&_trk, int t)
	: QUndoCommand()
{
	setText(i18n("Insert tab %1").arg(QString::number(t)));
	//Store important data
	trk = _trk;
	tv = _tv;
	x = trk->x;
	y = trk->y;
	xsel = trk->xsel;
	sel = trk->sel;
    totab = t;
	oldtab = trk->c[x].a[y];
	oldflags = trk->c[x].flags;
}

void TrackView::InsertTabCommand::redo()
{
	trk->c[x].flags &= ~FLAG_ARC;

	trk->x = x;
	trk->y = y;
	trk->sel = FALSE;
	trk->c[x].a[y] = totab;

	tv->repaintCurrentBar();
	emit tv->songChanged();
}

void TrackView::InsertTabCommand::undo()
{
	trk->x = x;
	trk->y = y;
	trk->xsel = xsel;
	trk->sel = sel;
	trk->c[x].a[y] = oldtab;

	tv->repaintCurrentBar();
	emit tv->songChanged();
}

TrackView::MoveFingerCommand::MoveFingerCommand(TrackView *_tv, TabTrack *&_trk,
												int _from, int _to, int _tune)
	: QUndoCommand(i18n("Transpose"))
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

	if (to < from) {
		setText(i18n("Transpose down"));
	} else {
		setText(i18n("Transpose up"));
	}
}

void TrackView::MoveFingerCommand::redo()
{
	trk->c[x].a[from] = -1;
	trk->c[x].a[to] = tune;

    // ...also for the effect parameter
	trk->c[x].e[to] = trk->c[x].e[from];
	trk->c[x].e[from] = 0;

	trk->x = x;
	trk->y = to;
	trk->sel = FALSE;

	emit tv->songChanged();
	tv->repaintCurrentBar();
}

void TrackView::MoveFingerCommand::undo()
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

	tv->repaintCurrentBar();
}

TrackView::AddFXCommand::AddFXCommand(TrackView *_tv, TabTrack *&_trk, char _fx)
	: QUndoCommand(i18n("Add effect"))
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

	setText(cmd.arg(p_fx));
}

void TrackView::AddFXCommand::redo()
{
	trk->x = x;
	trk->y = y;
    trk->addFX(fx);
	emit tv->songChanged();
	tv->repaintCurrentBar();
}

void TrackView::AddFXCommand::undo()
{
	trk->x = x;
	trk->y = y;
	trk->xsel = xsel;
	trk->sel = sel;

    trk->addFX(fx);
	tv->repaintCurrentBar();
}

TrackView::SetFlagCommand::SetFlagCommand(TrackView *_tv, TabTrack *&_trk, int _flag)
	: QUndoCommand(i18n("Set flag"))
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

	setText(cmd);
}

void TrackView::SetFlagCommand::redo()
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
				trk->c[x].a[i] = NULL_NOTE;
				trk->c[x].e[i] = 0;
			}
	}

	emit tv->songChanged();
	tv->repaintCurrentBar();
}

void TrackView::SetFlagCommand::undo()
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

	tv->repaintCurrentBar();
}

TrackView::DeleteNoteCommand::DeleteNoteCommand(TrackView *_tv, TabTrack *&_trk)
	: QUndoCommand(i18n("Delete note"))
{
	trk = _trk;
	tv = _tv;
	x = trk->x;
	y = trk->y;
	xsel = trk->xsel;
	sel = trk->sel;
	a = trk->c[x].a[y];
	e = trk->c[x].e[y];

	setText(i18n("Delete note %1").arg(a));
}

void TrackView::DeleteNoteCommand::redo()
{
	trk->x = x;
	trk->y = y;
	trk->c[x].a[y] = -1;
	trk->c[x].e[y] = 0;
	trk->sel = FALSE;
	emit tv->songChanged();
	tv->repaintCurrentBar();
}

void TrackView::DeleteNoteCommand::undo()
{
	trk->x = x;
	trk->y = y;
	trk->xsel = xsel;
	trk->sel = sel;
	trk->c[x].a[y] = a;
	trk->c[x].e[y] = e;
	tv->repaintCurrentBar();
}

TrackView::AddColumnCommand::AddColumnCommand(TrackView *_tv, TabTrack *&_trk)
	: QUndoCommand(i18n("Add column"))
{
	trk = _trk;
	tv = _tv;
	x = trk->x;
	y = trk->y;
	xsel = trk->xsel;
	sel = trk->sel;
	addBar = trk->currentBarDuration() == trk->maxCurrentBarDuration();
}

void TrackView::AddColumnCommand::redo()
{
	trk->x = x;
	trk->y = y;
	trk->xb = trk->bars().size() - 1;
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
		trk->xb++;
		TabBar bar;
		bar.start = trk->x;
		bar.time1 = trk->bars()[trk->xb-1].time1;
		bar.time2 = trk->bars()[trk->xb-1].time2;
		tv->model()->insertColumn(trk->bars().size());
		auto dataIndex = tv->model()->index(tv->selectionModel()->currentIndex().row(), trk->bars().size() - 1);
		tv->model()->setData(dataIndex, QVariant::fromValue(bar), TabSong::BarRole);
		emit tv->barChanged();
	}

	tv->updateRows();
	tv->ensureCurrentVisible();
	emit tv->songChanged();
	tv->repaintCurrentBar();
}

void TrackView::AddColumnCommand::undo()
{
	trk->x = x + 1;
	trk->y = y;
	trk->removeColumn(1);
	trk->x = x;
	trk->xsel = xsel;
	trk->sel = sel;

	tv->updateRows();
	tv->ensureCurrentVisible();
	tv->repaintCurrentBar();
}

TrackView::DeleteColumnCommand::DeleteColumnCommand(TrackView *_tv, TabTrack *&_trk)
	: QUndoCommand(i18n("Delete column"))
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
		setText(i18n("Delete %1 columns").arg(QString::number(p_delta)));
	p_del = p_delta;

	c.resize(1);
}

//This is the constructor called by cutToClipboard
TrackView::DeleteColumnCommand::DeleteColumnCommand(QString name, TrackView *_tv,
                                                    TabTrack *&_trk)
	: QUndoCommand(name)
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

void TrackView::DeleteColumnCommand::redo()
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
	emit tv->songChanged();
	tv->repaintCurrentBar();
}

void TrackView::DeleteColumnCommand::undo()
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
	tv->repaintCurrentBar();
}

TrackView::SetTimeSigCommand::SetTimeSigCommand(TrackView *_tv, TabTrack *&_trk,
                                                bool _toend, int _time1, int _time2)
	: QUndoCommand(i18n("Set time signature"))
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

	b.resize(trk->bars().size());
	for (uint i = 0; i < trk->bars().size(); i++)
		b[i] = trk->bars()[i];
}

void TrackView::SetTimeSigCommand::redo()
{
	// Sophisticated construction to mark all or only one bar with
	// new sig, depending on user's selection of checkbox

	for (uint i = xb; i < (toend ? trk->bars().size() : trk->xb+1); i++) {
		trk->bars()[i].time1 = time1;
		trk->bars()[i].time2 = time2;
	}

	trk->sel = FALSE;
	tv->update();
	emit tv->songChanged();
	tv->repaintCurrentBar(); //for emit paneChanded
}

void TrackView::SetTimeSigCommand::undo()
{
	int k;
	if (b.size() <= trk->bars().size())
		k = b.size();
	else k = trk->bars().size();

	for (int i = 0; i < k; i++)
		trk->bars()[i] = b[i];

	trk->x = x;
	trk->y = y;
	trk->xsel = xsel;
	trk->sel = sel;
	trk->xb = xb;
	tv->update();
	tv->repaintCurrentBar(); //for emit paneChanded
}

TrackView::InsertColumnCommand::InsertColumnCommand(TrackView *_tv, TabTrack *&_trk)
	: QUndoCommand(i18n("Insert column"))
{
	trk = _trk;
	tv = _tv;
	x = trk->x;
	y = trk->y;
	xsel = trk->xsel;
	sel = trk->sel;
}

void TrackView::InsertColumnCommand::redo()
{
	trk->x = x;
	trk->y = y;
	trk->insertColumn(1);
	trk->sel = FALSE;
	tv->update();
	emit tv->songChanged();
	tv->repaintCurrentBar();
}

void TrackView::InsertColumnCommand::undo()
{
	trk->x = x;
	trk->y = y;
	trk->xsel = xsel;
	trk->sel = sel;
	trk->removeColumn(1);
	tv->update();
    tv->repaintCurrentBar();
}

TrackView::InsertStrumCommand::InsertStrumCommand(TrackView *_tv, TabTrack *&_trk,
                                                  int _sch, int *_chord)
	: QUndoCommand(i18n("Insert strum"))
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
		setText(i18n("Insert Chord"));
}

void TrackView::InsertStrumCommand::redo()
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
	emit tv->songChanged();
	tv->repaintCurrentBar();
}

void TrackView::InsertStrumCommand::undo()
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
	tv->repaintCurrentBar();
}

TrackView::InsertRhythm::InsertRhythm(TrackView *_tv, TabTrack *&_trk, QList<int> quantized)
	: QUndoCommand(i18n("Insert rhythm"))
{
	trk = _trk;
	tv  = _tv;
	x   = trk->x;

	newdur = QList<int>(quantized);
}

void TrackView::InsertRhythm::redo()
{
	trk->x = x;
	int end = trk->c.size();

	// Create additional columns at the end of track, if we need them
	if (x + newdur.size() > trk->c.size()) {
		trk->c.resize(x + newdur.size());
		for (int i = end; i < trk->c.size(); i++) {
			for (uint j = 0; j < MAX_STRINGS; j++) {
				trk->c[i].a[j] = -1;
				trk->c[i].e[j] = 0;
			}
			trk->c[i].flags = 0;
		}
	}

	for (int i = 0; i < newdur.size(); i++) {
		if (x + i < end)
			olddur.append(trk->c[x + i].fullDuration());
		trk->c[x + i].setFullDuration(newdur[i]);
	}

	emit tv->songChanged();
	tv->viewport()->update();
}

void TrackView::InsertRhythm::undo()
{
	trk->x = x;

	for (int i = 0; i < olddur.size(); i++)
		trk->c[x + i].setFullDuration(olddur[i]);

	trk->c.resize(x + olddur.size());

	emit tv->songChanged();
	tv->viewport()->update();
}
