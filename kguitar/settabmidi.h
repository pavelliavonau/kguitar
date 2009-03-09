#ifndef SETTABMIDI_H
#define SETTABMIDI_H

#include "ui_settabmidibase.h"

class SetTabMidi: public SetTabMidiBase {
	Q_OBJECT

public:
	SetTabMidi(QWidget* parent = 0, const char* name = 0);
	~SetTabMidi();

	void setChorus(int);
	void setPan(int);
	void setReverb(int);
	void setTranspose(int);
	void setVolume(int);
};

#endif // SETTABMIDI_H
