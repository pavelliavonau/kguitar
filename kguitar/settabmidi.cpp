#include "settabmidi.h"

#include <qslider.h>


SetTabMidi::SetTabMidi(QWidget* parent)
	: QWidget(parent)
{
	ui.setupUi(this);
}

SetTabMidi::~SetTabMidi()
{
}

void SetTabMidi::setVolume(int vol)
{
	ui.SliderVolume->setValue(vol);
}

void SetTabMidi::setPan(int pan)
{
	ui.SliderPan->setValue(pan);
}

void SetTabMidi::setReverb(int rev)
{
	ui.SliderReverb->setValue(rev);
}

void SetTabMidi::setTranspose(int trans)
{
	ui.SliderTranspose->setValue(trans);
}

void SetTabMidi::setChorus(int chor)
{
	ui.SliderChorus->setValue(chor);
}

