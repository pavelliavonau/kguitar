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

void TabTrack::arrangeBars()
{
    int barlen = 480 * b[0].time1 / b[0].time2;
    int barnum = 1;
    int cl = 0;                         // Current length

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
    bool firstnote = TRUE, dot = FALSE;

    for (nn=0;nn<an.size();nn++) {
	cl = an[nn].l;
	firstnote = TRUE;
	cbl = 480;

//	while (cbl>0) {
//	    printf("New bar...\n");
	    while (cl>0) {
		if (cl>=720) {
		    dot = TRUE;  ln = 480; cl -= 720;
		} else if (cl>=480) {
		    dot = FALSE; ln = 480; cl -= 480;
		} else if (cl>=360) {
		    dot = TRUE;  ln = 240; cl -= 360;
		} else if (cl>=240) {
		    dot = FALSE; ln = 240; cl -= 240;
		} else if (cl>=180) {
		    dot = TRUE;  ln = 120; cl -= 180;
		} else if (cl>=120) {
		    dot = FALSE; ln = 120; cl -= 120;
		} else if (cl>=90) {
		    dot = TRUE;  ln = 60; cl -= 90;
		} else if (cl>=60) {
		    dot = FALSE; ln = 60; cl -= 60;
		} else if (cl>=45) {
		    dot = TRUE;  ln = 30; cl -= 45;
		} else if (cl>=30) {
		    dot = FALSE; ln = 30; cl -= 30;
		} else if (cl>=23) {
		    dot = TRUE;  ln = 15; cl -= 23;
		} else if (cl>=15) {
		    dot = FALSE; ln = 15; cl -= 15;
		}
		
		i++;
		printf("Col %d, duration: %d",i,ln);
		if (dot)  printf(", dotted");
		if (!firstnote)  printf(", arc");
		printf("\n");
		firstnote = FALSE;
	    }
    }

    cl = 0;
    b[0].start = 0;

    for (uint i=0;i<c.size();i++) {
	cl += c[i].l;

	if (c[i].flags & FLAG_DOT)      // Dotted are 1.5 times larger
	    cl += c[i].l/2;

	if (cl>barlen) {
	    b.resize(barnum+1);
	    b[barnum].start = i;
	    // GREYFIX - preserve other possible time signatures
	    b[barnum].time1 = b[barnum-1].time1;
	    b[barnum].time2 = b[barnum-1].time2;

	    if ((b[barnum].time1 != b[barnum-1].time1) ||
		(b[barnum].time2 != b[barnum-1].time2))
		b[barnum].showsig=TRUE;
	    else 
		b[barnum].showsig=FALSE;

	    barnum++;
	    cl-=barlen;
	}
    }
}
