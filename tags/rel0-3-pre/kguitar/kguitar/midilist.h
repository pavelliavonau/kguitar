#ifndef MIDILIST_H
#define MIDILIST_H

#include <qlist.h>

#include "tabtrack.h"


// GREYFIX!
#include <stdio.h>


enum {
	MIDI_NOTEOFF			= 0x80,
	MIDI_NOTEON				= 0x90,
	MIDI_POLY_AFTERTOUCH	= 0xa0,
	MIDI_CONTROL_CHANGE		= 0xb0,
	MIDI_PROGRAM_CHANGE		= 0xc0,
	MIDI_CHANNEL_AFTERTOUCH	= 0xd0,
	MIDI_PITCH_WHEEL		= 0xe0,
	MIDI_SYSEX				= 0xf0,
	MIDI_META				= 0xff
};

enum {
	META_SEQUENCE_NUMBER	= 0x00,
	META_TEXT				= 0x01,
	META_COPYRIGHT			= 0x02,
	META_SEQUENCE_NAME		= 0x03,
	META_INSTRUMENT_NAME	= 0x04,
	META_LYRIC				= 0x05,
	META_MARKER				= 0x06,
	META_CUE_POINT			= 0x07,
	META_END_TRACK     		= 0x2f,
	META_TEMPO				= 0x51,
	META_TIMESIG			= 0x58,
	META_KEYSIG				= 0x59
};



class MidiEvent
{
public:
	MidiEvent(long _timestamp, uchar _type, uchar _data1, uchar _data2) {
		timestamp = _timestamp;
		type = _type;
		data1 = _data1;
		data2 = _data2;
	};
	long timestamp;
	Q_UINT8 type, data1, data2;
};



class MidiList: public QList<MidiEvent>
{
public:
	MidiList() { setAutoDelete(TRUE); }
protected:
	virtual int compareItems(QCollection::Item e1, QCollection::Item e2);
};



class MidiData {

public:
    static void getMidiList(TabTrack *&trk, MidiList &ml);

};


#endif // MIDILIST_H
