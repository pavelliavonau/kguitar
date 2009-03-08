#include "settabmidi.h"

#include <qslider.h>


SetTabMidi::SetTabMidi(QWidget* parent, const char* name)
	: SetTabMidiBase(parent, name)
{
}

SetTabMidi::~SetTabMidi()
{
}

void SetTabMidi::setVolume(int vol)
{
	SliderVolume->setValue(vol);
}

void SetTabMidi::setPan(int pan)
{
	SliderPan->setValue(pan);
}

void SetTabMidi::setReverb(int rev)
{
	SliderReverb->setValue(rev);
}

void SetTabMidi::setTranspose(int trans)
{
	SliderTranspose->setValue(trans);
}

void SetTabMidi::setChorus(int chor)
{
	SliderChorus->setValue(chor);
}

