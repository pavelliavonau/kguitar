#ifndef SETTABMIDI_H
#define SETTABMIDI_H

#include "ui_settabmidibase.h"

class SetTabMidi: public KDialog {
	Q_OBJECT

public:
	SetTabMidi(QWidget* parent = 0);
	~SetTabMidi();

	void setChorus(int);
	void setPan(int);
	void setReverb(int);
	void setTranspose(int);
	void setVolume(int);

private:
        Ui::SetTabMidiBase ui;
};

#endif // SETTABMIDI_H
