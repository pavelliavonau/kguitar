#include "rhythmeditor.h"

#include <kdebug.h>
#include <KLocale>

RhythmEditor::RhythmEditor(QWidget *parent) :
	QDialog(parent)
{
	setupUi(this);
	connect(autoTempo, SIGNAL(toggled(bool)), SLOT(tempoState(bool)));
	connect(tapButton, SIGNAL(pressed()), SLOT(tap()));
	connect(quantizeButton, SIGNAL(clicked()), SLOT(quantize()));
}

void RhythmEditor::changeEvent(QEvent *e)
{
	QDialog::changeEvent(e);
	switch (e->type()) {
	case QEvent::LanguageChange:
		retranslateUi(this);
		break;
	default:
		break;
	}
}

void RhythmEditor::tap()
{
	if (original->count() == 0) {
		time.start();
		original->addItem(i18n("< STARTED >"));
	} else {
		int ms = time.restart();
		original->addItem(QString::number(ms));
	}
}

void RhythmEditor::tempoState(bool state)
{
	tempo->setEnabled(!state);
}

// Extremely funky and clever algorithm to find out the most probable
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

void RhythmEditor::quantize()
{
	double L4, newL4, sumL4 = 0;

	quantized->clear();
	quantized->addItem(i18n("< STARTED >"));

	if (autoTempo->isChecked()) {
		L4 = original->item(1)->text().toDouble();
	} else {
		L4 = 60000.0 / tempo->value();
	}

	for (int i = 1; i < original->count(); i++) {
		double t = original->item(i)->text().toDouble();
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

		kDebug() << "t=" << t << ", L4=" << L4 << ", so it looks like " << d;

		quantized->addItem(QString::number(d));

		newL4 = t / d * 120;
		sumL4 += newL4;
		L4 = sumL4 / i;

		kDebug() << "newL4=" << newL4 << ", so shift works, now L4=" << L4;
	}

	tempo->setValue(int(60000.0 / L4));
	autoTempo->setChecked(false);
}

QList<int> RhythmEditor::quantizedDurations()
{
	QList<int> list;
	for (int i = 1; i < quantized->count(); i++)
		list.append(quantized->item(i)->text().toInt());
	return list;
}
