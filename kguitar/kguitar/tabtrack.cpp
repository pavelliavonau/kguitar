#include "tabtrack.h"

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

void TabTrack::insertColumn(uint x)
{
    c.resize(c.size()+1);
    for (uint i=c.size()-1;i>x;i--)
	c[i]=c[i-1];
    for (uint i=0;i<MAX_STRINGS;i++)
	c[x].a[i]=-1;
}

void TabTrack::removeColumn(uint x)
{
    for (uint i=x;i<c.size()-1;i++)
	c[i]=c[i+1];
    c.resize(c.size()-1);
}

void TabTrack::arrangeBars()
{
    int barlen = 480;
    int barnum = 1;
    int cbl = 0;                        // Current bar length

    b[0].start=0;

    for (uint i=0;i<c.size();i++) {
	cbl += c[i].l;

	if (c[i].flags & FLAG_DOT)      // Dotted are 1.5 times larger
	    cbl += c[i].l/2;

	if (cbl>barlen) {
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
	    cbl-=barlen;
	}
    }
}
