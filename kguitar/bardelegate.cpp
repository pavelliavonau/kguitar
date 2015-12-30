#include "bardelegate.h"

#include "data/tabtrack.h"
#include "data/tabsong.h"
#include "trackprint.h"

#include <QPainter>

#ifdef USE_BOTH_OLD_AND_NEW
#define VERTSPACE                      230 // between top of cell and first line
#define VERTLINE                        10 // between horizontal tabulature lines
#define HORDUR                          4
#define HORCELL                         14 // horizontal size of tab numbers column
#define TIMESIGSIZE                     14 // horizontal time sig size
#define ABBRLENGTH                      25 // drum abbreviations horizontal size

#define BOTTOMDUR   VERTSPACE+VERTLINE*(s+1)
#endif

QSize BarDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	int r = index.row();
	int c = index.column();

	if( r < 0 || c < 0 )
		return QItemDelegate::sizeHint(option, index);

	auto model = index.model();
//    int curtnum = model->data(index, TabSong::CurTrackNumRole).toInt();

//    if(curtnum < 0)
//        return QItemDelegate::sizeHint(option, index);

	TabTrack *curt = model->data(model->index(0, 0), TabSong::TrackPtrRole).value<TabTrack *>();

	int bn = r * model->columnCount() + c;

	int selx2coord = -1;
	selxcoord = -1;

	trp->xpos = -1;
	trp->calcYPosSt();
	if (trp->viewscore && trp->fetaFontPtr()) {
			trp->initPrStyle(2);
			//trp->drawStLns(option.rect.width());
	} else {
			trp->initPrStyle(0);
	}

	trp->calcYPosTb(curt->string);

	bool doDraw = false;//!c ? true : false;
	bool fbol = true;
	bool flop = (bn == 0);

	if( bn < curt->bars().size() && curt->bars().at(bn).isValid() ) {
		trp->drawKKsigTsig(bn, curt, doDraw, fbol, flop);
		trp->drawBar(bn, curt, 0, selxcoord, selx2coord, false);
	}

	int size_w = trp->xpos;
	return QSize(size_w, trp->yPosTb() + trp->bottomTbMargin());
}

BarDelegate::BarDelegate(QObject *parent) : QItemDelegate(parent),
	trp(nullptr),
	playbackCursor(false)
{}

void BarDelegate::paint(QPainter *p, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	QItemDelegate::paint(p, option, index);

	int r = index.row();
	int c = index.column();

	// Drawing only this bar
	int bn =  /*r * index.model()->columnCount() +*/ c;

	int selx2coord = -1;
	selxcoord = -1;

	auto model = index.model();
//    int curtnum = model->data(index, TabSong::CurTrackNumRole).toInt();

//    if(curtnum < 0)
//        return;

	TabTrack *curt = model->data(model->index(r, 0), TabSong::TrackPtrRole).value<TabTrack *>();
	if(!curt) return;

	if (curt->bars().size() <= bn || !curt->bars().at(bn).isValid())  return;

	trp->setPainter(p);
    // LVIFIX: initmetrics may be expensive but depends on p, init only once ?
//	trp->initMetrics();
    // LVIFIX: do following calculations for the current bar only
	curt->calcVoices();
	curt->calcStepAltOct();
	curt->calcBeams();
	trp->calcYPosSt(option.rect.top());
	trp->xpos = option.rect.left();//-1;
	if (trp->viewscore && trp->fetaFontPtr()) {
		trp->initPrStyle(2);
		trp->drawStLns(option.rect);
	} else {
		trp->initPrStyle(0);
	}
	trp->calcYPosTb(curt->string);
#ifdef USE_BOTH_OLD_AND_NEW
    // force new tabbar position close to old one
    trp->yPosTb() = (int) ((TOPSPST + NLINEST - 1) * trp->ystepst)
                  + (int) (BOTSPST * trp->ystepst)
                  + (int) ((TOPSPTB + curt->string) * trp->ysteptb);
#endif
	trp->drawBarLns(option.rect.width(), curt);
//	trp->drawKey(bn, curt);	// LVIFIX: make (some more) room between key and time sig
	bool doDraw = true; // !c ? true : false;
	bool fbol = true;
	bool flop = (bn == 0);
	(void) trp->drawKKsigTsig(bn, curt, doDraw, fbol, flop);
	trp->drawBar(bn, curt, 0, selxcoord, selx2coord);

    // connect tabbar and staff with vertical line at end of bar
//	if (viewscore && fetaFont) {
//		p->setPen(trp->pLnBl);
////		p->drawLine(trp->xpos - 1, trp->yposst, trp->xpos - 1, trp->ypostb);
//	}

	// DEBUG: DRAW VARIOUS GUIDE BORDERS

//	p->setBrush(NoBrush);
//	p->setPen(red);
//	p->drawRect(cellRect());
//	p->setPen(blue);
//	p->drawRect(0, TOPSPTB * trp->ysteptb, cellWidth(), curt->string * trp->ysteptb);

	// DRAW SELECTION

	p->setCompositionMode(QPainter::RasterOp_SourceXorDestination);
	p->setBrush(option.palette.color(QPalette::Base));

	const int horcell = static_cast<int>(2.6 * trp->br8w);
	const int vertline = trp->ysteptb;
	const int vertspace = trp->yPosTb();

	if (playbackCursor) {
		// Draw MIDI playback cursor
		if (selxcoord != -1)
			p->drawRect(selxcoord - horcell / 2, 0, horcell + 1, option.rect.height());

	} else {

		// Draw selection between selxcoord and selx2coord (if it exists)
		if (curt->sel) {
			if ((selxcoord != -1) && (selx2coord != -1)) {
				int x1 = std::min(selxcoord, selx2coord);
				int wid = abs(selx2coord - selxcoord) + horcell + 1;
				p->drawRect(x1 - horcell / 2, 0, wid, option.rect.height());
			} else if ((selxcoord == -1) && (selx2coord != -1)) {
				if (curt->x > curt->lastColumn(bn))
					p->drawRect(selx2coord - horcell / 2, 0, option.rect.width(), option.rect.height());
				else
					p->drawRect(0, 0, selx2coord + horcell / 2 + 1, option.rect.height());
			} else if ((selxcoord != -1) && (selx2coord == -1)) {
				if (curt->xsel > curt->lastColumn(bn))
					p->drawRect(selxcoord - horcell / 2, 0, option.rect.width(), option.rect.height());
				else
					p->drawRect(0, 0, selxcoord + horcell / 2 + 1, option.rect.height());
			} else { // both are -1
				int x1 = std::min(curt->x, curt->xsel);
				int x2 = std::max(curt->x, curt->xsel);
				if ((x1 < curt->bars()[bn].start) && (x2 > curt->lastColumn(bn)))
					p->drawRect(0, 0, option.rect.width(), option.rect.height());
			}
		}
		// Draw original cursor (still inverted)
		if (selxcoord != -1) {
			p->drawRect(selxcoord - horcell / 2,
					vertspace + (0 - curt->y) * vertline - vertline / 2 - 2,
					horcell,
					vertline + 3);
		}
	}

	p->setCompositionMode(QPainter::CompositionMode_Source);
#ifdef USE_BOTH_OLD_AND_NEW
	QString tmp;
	bool ringing[MAX_STRINGS];
	int trpCnt = 0;                     // triplet count
	int lastPalmMute = 0;

	int s = curt->string - 1;

	for (int i = 0; i <= s; i++) {
		p->drawLine(0, VERTSPACE + (s - i) * VERTLINE,
		            width(), VERTSPACE + (s - i) * VERTLINE);
		ringing[i] = FALSE;
	}

	int xpos = 40, lastxpos = 20, xdelta;

	selxcoord = -1;

	// Starting bars - very thick and thick one

	if (bn == 0) {
		p->setBrush(Qt::SolidPattern);
		p->drawRect(0, VERTSPACE, 5, VERTLINE * s);
		p->drawRect(8, VERTSPACE, 2, VERTLINE * s);
	}

	// Time signature

	if (curt->showBarSig(bn)) {
 		p->setFont(*timeSigFont);
		tmp.setNum(curt->b[bn].time1);
		p->drawText(20, VERTSPACE + VERTLINE * s / 4 - TIMESIGSIZE / 2,
					TIMESIGSIZE, TIMESIGSIZE, Qt::AlignCenter, tmp);
		tmp.setNum(curt->b[bn].time2);
		p->drawText(20, VERTSPACE + VERTLINE * s * 3 / 4 - TIMESIGSIZE / 2,
					TIMESIGSIZE, TIMESIGSIZE, Qt::AlignCenter, tmp);
	}

	p->setFont(*normalFont);
	p->setBrush(KGlobalSettings::activeTextColor());

	// Drum abbreviations markings

	if (curt->trackMode() == TabTrack::DrumTab) {
		p->setPen(Qt::NoPen);
		for (int i = 0; i <= s; i++) {
			p->drawRect(xpos, VERTSPACE + (s - i) * VERTLINE - VERTLINE / 2,
						ABBRLENGTH, VERTLINE + 1);
			p->drawText(xpos, VERTSPACE + (s - i) * VERTLINE - VERTLINE / 2,
						ABBRLENGTH, VERTLINE, Qt::AlignCenter, drum_abbr[curt->tune[i]]);
		}
		xpos += ABBRLENGTH + 10; lastxpos += ABBRLENGTH + 10;
		p->setPen(Qt::SolidLine);
	}

	for (int t = curt->b[bn].start; t <= curt->lastColumn(bn); t++) {

		// triplet handling:
		// - reset after third note of triplet
		// - count notes while inside triplet
		if (trpCnt >= 3) {
			trpCnt = 0;
		}
		if (curt->c[t].flags & FLAG_TRIPLET) {
			trpCnt++;
		} else {
			trpCnt = 0;
		}

		// Drawing duration marks

		// Draw connection with previous, if applicable
		if ((t > 0) && (t > curt->b[bn].start) && (curt->c[t - 1].l == curt->c[t].l))
			xdelta = lastxpos + HORCELL / 2;
		else
			xdelta = xpos + HORCELL / 2 + HORDUR;

		switch (curt->c[t].l) {
		case 15:  // 1/32
			p->drawLine(xpos + HORCELL / 2, BOTTOMDUR + VERTLINE - 4,
						xdelta, BOTTOMDUR + VERTLINE - 4);
		case 30:  // 1/16
			p->drawLine(xpos + HORCELL / 2, BOTTOMDUR + VERTLINE - 2,
						xdelta, BOTTOMDUR + VERTLINE - 2);
		case 60:  // 1/8
			p->drawLine(xpos + HORCELL / 2, BOTTOMDUR + VERTLINE,
						xdelta, BOTTOMDUR + VERTLINE);
		case 120: { // 1/4 - a long vertical line, so we need to find the highest note
			int i;
			for (i = s; ((i >= 0) && (curt->c[t].a[i] == -1)); i--);

			// If it's an empty measure at all - draw the vertical line from bottom
			if (i < 0)  i = s / 2;

			p->drawLine(xpos + HORCELL / 2, VERTSPACE + VERTLINE * (s - i) + VERTLINE / 2,
						xpos + HORCELL / 2, BOTTOMDUR + VERTLINE);
		}
		case 240: // 1/2
			p->drawLine(xpos + HORCELL / 2, BOTTOMDUR + 3,
						xpos + HORCELL / 2, BOTTOMDUR + VERTLINE);
		case 480:; // whole
		}

		// Draw dot

		if (curt->c[t].flags & FLAG_DOT)
			p->drawRect(xpos + HORCELL / 2 + 3, BOTTOMDUR + 5, 2, 2);

		// Draw triplet - GREYFIX: ugly code, needs to be fixed
		// somehow... Ideally, triplets should be drawn in a second
		// loop, after everything else would be done.

		/*
		if (curt->c[t].flags & FLAG_TRIPLET) {
 			if ((curt->c.size() >= t + 1) && (t) &&
 				(curt->c[t - 1].flags & FLAG_TRIPLET) &&
 				(curt->c[t + 1].flags & FLAG_TRIPLET) &&
				(curt->c[t - 1].l == curt->c[t].l) &&
				(curt->c[t + 1].l == curt->c[t].l)) {
				p->drawLine(lastxpos + HORCELL / 2, BOTTOMDUR + VERTLINE + 5,
							xpos * 2 - lastxpos + HORCELL / 2, BOTTOMDUR + VERTLINE + 5);
				p->drawLine(lastxpos + HORCELL / 2, BOTTOMDUR + VERTLINE + 2,
							lastxpos + HORCELL / 2, BOTTOMDUR + VERTLINE + 5);
				p->drawLine(xpos * 2 - lastxpos + HORCELL / 2, BOTTOMDUR + VERTLINE + 2,
							xpos * 2 - lastxpos + HORCELL / 2, BOTTOMDUR + VERTLINE + 5);
				p->setFont(*smallCaptionFont);
				p->drawText(xpos, BOTTOMDUR + VERTLINE + 7, HORCELL, VERTLINE, AlignHCenter | AlignTop, "3");
				p->setFont(*normalFont);
 			} else {
				if (!(((curt->c.size() >= t + 2) &&
					   (curt->c[t + 1].flags & FLAG_TRIPLET) &&
					   (curt->c[t + 2].flags & FLAG_TRIPLET) &&
					   (curt->c[t + 1].l == curt->c[t].l) &&
					   (curt->c[t + 2].l == curt->c[t].l)) ||
					  ((t >= 2) &&
					   (curt->c[t - 1].flags & FLAG_TRIPLET) &&
					   (curt->c[t - 2].flags & FLAG_TRIPLET) &&
					   (curt->c[t - 1].l == curt->c[t].l) &&
					   (curt->c[t - 2].l == curt->c[t].l)))) {
					p->setFont(*smallCaptionFont);
					p->drawText(xpos, BOTTOMDUR + VERTLINE + 7, HORCELL, VERTLINE, AlignHCenter | AlignTop, "3");
					p->setFont(*normalFont);
				}
			}
		}
		*/

		// Length of interval to next column - adjusted if dotted
		// calculated here because it is required by triplet code

		xdelta = horizDelta(t);

		// Draw triplet - improved (? :-)) code
		if ((trpCnt == 1) || (trpCnt == 2)) {
			// draw horizontal line to next note
			p->drawLine(xpos + HORCELL / 2, BOTTOMDUR + VERTLINE + 5,
						xpos + HORCELL / 2 + xdelta, BOTTOMDUR + VERTLINE + 5);
		}
		if ((trpCnt == 1) || (trpCnt == 3)) {
			// draw vertical line
			p->drawLine(xpos + HORCELL / 2, BOTTOMDUR + VERTLINE + 2,
						xpos + HORCELL / 2, BOTTOMDUR + VERTLINE + 5);
		}
		if (trpCnt == 2) {
			// draw "3"
			p->setFont(*smallCaptionFont);
			p->drawText(xpos, BOTTOMDUR + VERTLINE + 7, HORCELL, VERTLINE, Qt::AlignHCenter | Qt::AlignTop, "3");
			p->setFont(*normalFont);
		}

		// Draw arcs to backward note

		if (curt->c[t].flags & FLAG_ARC)
			p->drawArc(lastxpos + HORCELL / 2, BOTTOMDUR + 9,
					   xpos - lastxpos, 10, 0, -180 * 16);

		// Draw palm muting

		if (curt->c[t].flags & FLAG_PM) {
			if (lastPalmMute == 0)  {     // start drawing with "P.M."
				p->setFont(*smallCaptionFont);
				p->drawText(xpos, VERTSPACE / 2, VERTLINE * 2, VERTLINE,
							Qt::AlignCenter, "P.M.");
				p->setFont(*normalFont);
				lastPalmMute = 1;
			} else if (lastPalmMute == 1) {
				p->drawLine(lastxpos + VERTLINE * 2, VERTSPACE / 2 + VERTLINE / 2,
							xpos + HORCELL / 2, VERTSPACE / 2 + VERTLINE / 2);
				lastPalmMute = 2;
			} else {
				p->drawLine(lastxpos + HORCELL / 2, VERTSPACE / 2 + VERTLINE / 2,
							xpos + HORCELL / 2, VERTSPACE / 2 + VERTLINE / 2);
			}
		} else {
			if (lastPalmMute == 2) {
				p->drawLine(lastxpos + HORCELL / 2, VERTSPACE / 2 + VERTLINE / 2,
				            lastxpos + HORCELL / 2, VERTSPACE / 2 + VERTLINE);
			}
			lastPalmMute = 0;
		}

		// Draw the number column

		p->setPen(Qt::NoPen);
		for (int i = 0; i <= s; i++) {
			if (curt->c[t].a[i] != -1) {
				if (curt->c[t].a[i] == DEAD_NOTE)
					tmp = "X";
				else
					tmp.setNum(curt->c[t].a[i]);
				p->drawRect(xpos, VERTSPACE + (s - i) * VERTLINE - VERTLINE / 2,
							HORCELL, VERTLINE);
				p->drawText(xpos, VERTSPACE + (s - i) * VERTLINE - VERTLINE / 2,
							HORCELL, VERTLINE, Qt::AlignCenter, tmp);
				if (ringing[i]) {
					drawLetRing(p, xpos, VERTSPACE + (s - i) * VERTLINE);
					ringing[i] = FALSE;
				}
			}
			if ((curt->c[t].a[i] == -1)
			     && (curt->c[t].e[i] == EFFECT_STOPRING)) {
				if (ringing[i]) {
					drawLetRing(p, xpos, VERTSPACE + (s - i) * VERTLINE);
					ringing[i] = FALSE;
				}
			}

			if (t == curt->x)
				selxcoord = xpos;

			if (t == curt->xsel)
				selx2coord = xpos;

			// Draw effects
			// GREYFIX - use lastxpos, not xdelta

			switch (curt->c[t].e[i]) {
			case EFFECT_HARMONIC:
 				p->setFont(*smallCaptionFont);
				p->drawText(xpos + VERTLINE + 2, VERTSPACE + (s - i) * VERTLINE - VERTLINE * 2 / 3,
							HORCELL, VERTLINE, Qt::AlignCenter, "H");
 				p->setFont(*normalFont);
				break;
			case EFFECT_ARTHARM:
 				p->setFont(*smallCaptionFont);
				p->drawText(xpos + VERTLINE + 2, VERTSPACE + (s - i) * VERTLINE - VERTLINE * 2 / 3,
							HORCELL * 2, VERTLINE, Qt::AlignCenter, "AH");
 				p->setFont(*normalFont);
				break;
			case EFFECT_LEGATO:
 				p->setPen(Qt::SolidLine);
				p->drawArc(xpos + HORCELL, VERTSPACE + (s - i) * VERTLINE - VERTLINE / 2,
						   xdelta - HORCELL, 10, 0, 180 * 16);
				if ((t < curt->c.size() - 1) && (curt->c[t + 1].a[i] >= 0)) {
 					p->setFont(*smallCaptionFont);
					if (curt->c[t + 1].a[i] > curt->c[t].a[i]) {
						p->drawText(xpos + xdelta / 2 - HORCELL / 2, VERTSPACE + (s - i) * VERTLINE - VERTLINE / 3,
									HORCELL * 2, VERTLINE, Qt::AlignCenter, "HO");
					} else if (curt->c[t + 1].a[i] < curt->c[t].a[i]) {
						p->drawText(xpos + xdelta / 2 - HORCELL / 2, VERTSPACE + (s - i) * VERTLINE - VERTLINE / 3,
									HORCELL * 2, VERTLINE, Qt::AlignCenter, "PO");
					}
 					p->setFont(*normalFont);
				}
				p->setPen(Qt::NoPen);
				break;
			case EFFECT_SLIDE:
				p->setPen(Qt::SolidLine);
				if ((t < curt->c.size() - 1) && (curt->c[t + 1].a[i] >= 0)) {
					if (curt->c[t + 1].a[i] > curt->c[t].a[i]) {
						p->drawLine(xpos + HORCELL + 2, VERTSPACE + (s - i) * VERTLINE + VERTLINE / 2 - 1,
									xpos + xdelta, VERTSPACE + (s - i) * VERTLINE - VERTLINE / 2 + 1);
					} else {
						p->drawLine(xpos + HORCELL + 2, VERTSPACE + (s - i) * VERTLINE - VERTLINE / 2 + 1,
									xpos + xdelta, VERTSPACE + (s - i) * VERTLINE + VERTLINE / 2 - 1);
					}
				}
				p->setPen(Qt::NoPen);
				break;
			case EFFECT_LETRING:
				ringing[i] = TRUE;
				break;
			}
		}

		p->setPen(Qt::SolidLine);

		lastxpos = xpos;
		xpos += xdelta;
	}

	// Show notes still ringing at end of bar
	for (int i = 0; i <= s; i++) {
		if (ringing[i]) {
			drawLetRing(p, xpos - HORCELL / 3, VERTSPACE + (s - i) * VERTLINE);
			ringing[i] = FALSE;
		}
	}

	// End bar with vertical line
	p->setPen(Qt::SolidLine);
	p->drawRect(xpos, VERTSPACE, 1, VERTLINE * s);

	// Draw original cursor (still inverted)
	p->setCompositionMode(QPainter::RasterOp_SourceXorDestination);
// 	p->setBrush(KGlobalSettings::highlightColor());
	if (selxcoord != -1) {
		p->drawRect(selxcoord, VERTSPACE + (s - curt->y) * VERTLINE - VERTLINE / 2 - 1,
					HORCELL + 1, VERTLINE + 2);
	}

// 	p->setBrush(KGlobalSettings::baseColor());
	p->setCompositionMode(QPainter::CompositionMode_Source);
	p->setBrush(Qt::SolidPattern);
#endif // USE_BOTH_OLD_AND_NEW
}
