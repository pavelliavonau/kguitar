#ifndef RHYTHMER_H
#define RHYTHMER_H

#include "global.h"

#include <qdialog.h>
#include <qdatetime.h>

#ifdef WITH_TSE3
#include <tse3/MidiScheduler.h>
#endif

class Q3ListBox;
class QPushButton;
class QSpinBox;
class QCheckBox;

class Rhythmer: public QDialog {
    Q_OBJECT
public:
	Rhythmer(
#ifdef WITH_TSE3
			 TSE3::MidiScheduler *_scheduler,
#endif
			 QWidget *parent = 0, const char *name = 0);

	Q3ListBox *quantized;

public slots:
    void tap();
	void reset();
	void quantize();
	void tempoState(bool state);

private:
	Q3ListBox *original;
	QSpinBox *tempo;
	QCheckBox *temponew, *dotted, *triplet;

	QPushButton *resetButton, *tapButton, *quantizeButton;

	QTime time;

#ifdef WITH_TSE3
	TSE3::MidiScheduler *scheduler;
#endif
};

#endif
