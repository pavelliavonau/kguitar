#include "tabtrack.h"

// GREYFIX
#include <stdio.h>

TabTrack::TabTrack(TrackMode _tm, QString _name, int _channel,
		   int _bank, uchar _patch, uchar _string, uchar _frets)
{
    tm=_tm;
    name=_name;
    channel=_channel;
    bank=_bank;
    patch=_patch;
    string=_string;
    frets=_frets;
};

// Pretty sophisticated expression that determines if we can omit the time sig

bool TabTrack::showBarSig(uint n)
{
    return !((n>0) && (b[n-1].time1==b[n].time1) && (b[n-1].time2==b[n].time2));
}

void TabTrack::insertColumn(uint n)
{
    c.resize(c.size()+1);
    for (uint i=c.size()-1;i>n;i--)
	c[i]=c[i-1];
    for (uint i=0;i<MAX_STRINGS;i++)
	c[x].a[i]=-1;
}

void TabTrack::removeColumn(uint n)
{
    for (uint i=n;i<c.size()-1;i++)
	c[i]=c[i+1];
    c.resize(c.size()-1);
}

//////////////////////////////////////////////////////////////////////
//
// Okay, down below is the probably the *messiest* and the *ugliest*
// code in the whole program. It's not for the faint of heart. It's
// really hard to understand (I doubt if I understand it myself), but
// it should work. If you'll proceed, please read it and think of
// any way of improving it...
//
//////////////////////////////////////////////////////////////////////



// Well, if you insist on reading... Then try...



#define ADD_NEW_COLUMN(value);                                  \
        toput = value;						\
	while (toput>0) {					\
	    if (toput>=720) {					\
		dot = TRUE;  ln = 480; toput -= 720;		\
	    } else if (toput>=480) {				\
		dot = FALSE; ln = 480; toput -= 480;		\
	    } else if (toput>=360) {				\
		dot = TRUE;  ln = 240; toput -= 360;		\
	    } else if (toput>=240) {				\
		dot = FALSE; ln = 240; toput -= 240;		\
	    } else if (toput>=180) {				\
		dot = TRUE;  ln = 120; toput -= 180;		\
	    } else if (toput>=120) {				\
		dot = FALSE; ln = 120; toput -= 120;		\
	    } else if (toput>=90) {				\
		dot = TRUE;  ln = 60; toput -= 90;		\
	    } else if (toput>=60) {				\
		dot = FALSE; ln = 60; toput -= 60;		\
	    } else if (toput>=45) {				\
		dot = TRUE;  ln = 30; toput -= 45;		\
	    } else if (toput>=30) {				\
		dot = FALSE; ln = 30; toput -= 30;		\
	    } else if (toput>=23) {				\
		dot = TRUE;  ln = 15; toput -= 23;		\
	    } else if (toput>=15) {				\
		dot = FALSE; ln = 15; toput -= 15;		\
	    }							\
	    i++;						\
            c.resize(i); c[i-1] = an[nn]; c[i-1].l = ln;	\
	    if (dot)  c[i-1].flags |= FLAG_DOT;			\
	    if (!firstnote) {					\
                c[i-1].flags |= FLAG_ARC;			\
		for (uint k=0;k<MAX_STRINGS;k++)		\
		    c[i-1].a[k]=-1;				\
            }							\
	    firstnote = FALSE;					\
	}

void TabTrack::arrangeBars()
{
    int barlen = 480 * b[0].time1 / b[0].time2;
    int barnum = 1;
    uint cl = 0;                        // Current length

    // COLLECT ALL NOTES INFORMATION

    QArray<TabColumn> an;               // Collected columns information
    uint nn = 0;                        // Number of already made columns
    uint i;

    for (i=0;i<c.size();i++) {
	cl = c[i].l;
	if (c[i].flags & FLAG_DOT)      // Dotted are 1.5 times larger
	    cl = cl + cl/2;
	if (!(c[i].flags & FLAG_ARC)) { // Add the note, if it's not arc
	    nn++;
	    an.resize(nn);
	    an[nn-1] = c[i];
	    an[nn-1].l = cl;
	} else {                        // Add to duration of previous note
	    an[nn-1].l += cl;
	}
    }

    // RECONSTRUCTING BARS & COLUMNS ARRAYS

    i = 0;
    uint ln;
    uint cbl;
    uint toput;
    bool firstnote = TRUE, dot = FALSE;

    cbl = barlen;
    b[0].start = 0;
    barnum = 0;

    for (nn=0; nn<an.size(); nn++) {
	cl = an[nn].l;
	firstnote = TRUE;

	while (cl>0) {
	    if (cl<cbl) {
		ADD_NEW_COLUMN(cl);
		cbl -= cl;
		cl = 0;
	    } else {
		ADD_NEW_COLUMN(cbl);
		cl -= cbl;
		cbl = barlen;

		barnum++;
		b.resize(barnum+1);
		b[barnum].start = i;
		// GREYFIX - preserve other possible time signatures
		b[barnum].time1 = b[barnum-1].time1;
		b[barnum].time2 = b[barnum-1].time2;
	    }
	}
    }

    // Clean up last bar if it's empty
    if (b[barnum].start == i)
	b.resize(barnum);

    // Make sure that cursor x is in legal range
    if (x>=c.size())
	x = c.size()-1;

    // Find the bar the cursor in
    if (x>=b[b.size()-1].start)
	xb = b.size()-1;
    else
	for (i=0; i<b.size()-1; i++)
	    if ((x>=b[i].start) && (x<b[i+1].start)) {
		xb = i;
		break;
	    }

    // All should be done now.
}
