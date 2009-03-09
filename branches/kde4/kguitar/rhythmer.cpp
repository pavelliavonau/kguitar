#include "rhythmer.h"

#include <qlayout.h>
#include <q3listbox.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qspinbox.h>
#include <qcheckbox.h>
//Added by qt3to4:
#include <Q3BoxLayout>
#include <Q3HBoxLayout>
#include <Q3VBoxLayout>

#include <klocale.h>

Rhythmer::Rhythmer(
#ifdef WITH_TSE3
                   TSE3::MidiScheduler *_scheduler,
#endif
                   QWidget *parent, const char *name)
	: QDialog(parent, name, TRUE)
{
#ifdef WITH_TSE3
	scheduler = _scheduler;
#endif

	original = new Q3ListBox(this);
	quantized = new Q3ListBox(this);
	QLabel *original_l = new QLabel(original, i18n("&Original taps:"), this);
	QLabel *quantized_l = new QLabel(quantized, i18n("Q&uantized rhythm:"), this);

	tapButton = new QPushButton(i18n("&Tap"), this);
	connect(tapButton, SIGNAL(pressed()), SLOT(tap()));

	resetButton = new QPushButton(i18n("&Reset"), this);
	connect(resetButton, SIGNAL(clicked()), SLOT(reset()));

	quantizeButton = new QPushButton(i18n("&Quantize"), this);
	connect(quantizeButton, SIGNAL(clicked()), SLOT(quantize()));

	tempo = new QSpinBox(0, 300, 1, this);
	QLabel *tempo_l = new QLabel(tempo, i18n("T&empo:"), this);

	temponew = new QCheckBox(i18n("&Determine automatically"), this);
	connect(temponew, SIGNAL(toggled(bool)), SLOT(tempoState(bool)));
	temponew->setChecked(TRUE);

	dotted = new QCheckBox(i18n("Detect d&otted"), this);
	dotted->setChecked(TRUE);

	triplet = new QCheckBox(i18n("Detect t&riplets"), this);
	triplet->setEnabled(FALSE);

	QPushButton *ok = new QPushButton(i18n("OK"), this);
	connect(ok, SIGNAL(clicked()), SLOT(accept()));

	QPushButton *cancel = new QPushButton(i18n("Cancel"), this);
	connect(cancel, SIGNAL(clicked()), SLOT(reject()));

	// LAYOUT MANAGEMENT

	// Main layout
	Q3BoxLayout *l = new Q3HBoxLayout(this, 10);

	// Original layout
	Q3BoxLayout *lorig = new Q3VBoxLayout();
	lorig->addWidget(original_l);
	lorig->addWidget(original, 1);
	l->addLayout(lorig, 1);

	// Tempo layout
	Q3BoxLayout *ltempo = new Q3HBoxLayout();
	ltempo->addWidget(tempo_l);
	ltempo->addWidget(tempo, 1);

	// Button zone layout
	Q3BoxLayout *lbzone = new Q3VBoxLayout();
	lbzone->addLayout(ltempo);
	lbzone->addWidget(temponew);
	lbzone->addWidget(dotted);
	lbzone->addWidget(triplet);
	lbzone->addWidget(tapButton);
	lbzone->addWidget(resetButton);
	lbzone->addWidget(quantizeButton);
	l->addLayout(lbzone);

	// Quantized layout
	Q3BoxLayout *lquan = new Q3VBoxLayout();
	lquan->addWidget(quantized_l);
	lquan->addWidget(quantized, 1);
	l->addLayout(lquan, 1);

	// Dialog buttons layout
	Q3BoxLayout *ldialog = new Q3VBoxLayout();
	ldialog->addWidget(ok);
	ldialog->addWidget(cancel);
	l->addLayout(ldialog);

	setCaption(i18n("Rhythm Constructor"));
}

void Rhythmer::reset()
{
	original->clear();
	quantized->clear();
}

void Rhythmer::tap()
{
	if (original->firstItem() == 0) {
		time.start();
		original->insertItem(i18n("< STARTED >"));
	} else {
		int ms = time.restart();
		original->insertItem(QString::number(ms));
	}
}

void Rhythmer::tempoState(bool state)
{
	tempo->setEnabled(!state);
}

// Extremly funky and clever algorithm to find out the most probable
// tempo and meanings of durations, though it's not perfect :(
//
// It works detects durations like this when in non-dotted mode:
//
// ...[L16]  [L8]  [L4] [L2] [L1]
// ... 1/4   1/2    1    2    4
// ...   0.375  0.75  1.5   3
//
// and like this in dotted mode:
//
// ...[L16.]    [L8]   [L8.]   [L4]  [L4.]  [L2] [L2.] [L1]
// ... 1/4      1/2     3/4     1     3/2    2     3    4
// ...     0.4375  0.625   0.875  1.25   1.75  2.5   3.5+

// GREYFIX: Support triplets

void Rhythmer::quantize()
{
	double L4, newL4, sumL4 = 0;

	quantized->clear();
	quantized->insertItem(i18n("< STARTED >"));

	if (temponew->isChecked()) {
		L4 = original->text(1).toDouble();
	} else {
		L4 = 60000.0 / tempo->value();
	}

	for (uint i = 1; i < original->count(); i++) {
		double t = original->text(i).toDouble();
		int d = 0;

		double coef = dotted->isChecked() ? 3.5 : 3;
		int trial = 480;

		while (trial >= 15) {
			if (t > coef * L4) {
				d = trial;
				break;
			}
			if ((dotted->isChecked()) && (t > coef / 1.4 * L4)) {
				d = trial * 3 / 4;
				break;
			}
			coef /= 2;
			trial /= 2;
		}

		if (!d)  d = 15; // we don't support stuff less than 1/32th of a bar

		kdDebug() << "t=" << t << ", L4=" << L4 << ", so it looks like " << d << endl;

		quantized->insertItem(QString::number(d));

		newL4 = t / d * 120;
		sumL4 += newL4;
		L4 = sumL4 / i;

		kdDebug() << "newL4=" << newL4 << ", so shift works, now L4=" << L4 << endl;
	}

	tempo->setValue(int(60000.0 / L4));
	temponew->setChecked(FALSE);
}
