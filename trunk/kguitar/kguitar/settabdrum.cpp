#include "settabdrum.h"

#include <klocale.h>
#include <qcombobox.h>
#include <qspinbox.h>
#include <qlabel.h>

SetTabDrum::SetTabDrum(QWidget *parent=0, const char *name=0)
	: QWidget(parent, name)
{
    // Controls

    dr = new QSpinBox(1, MAX_STRINGS, 1, this);
    connect(dr, SIGNAL(valueChanged(int)), SLOT(stringChanged(int)));
    dr->setGeometry(90, 50, 40, 20);

    QLabel *dr_l = new QLabel(i18n("Drums:"), this);
    dr_l->setGeometry(10, 50, 50, 20);

    // Tuners

    for (int i = 0; i < MAX_STRINGS; i++) {
		tuner[i] = new QSpinBox(this);
		//		connect(tuner[i], SIGNAL(valueChanged(int)), SLOT(tuneChanged()));
    }
    oldst = MAX_STRINGS;
}

void SetTabDrum::stringChanged(int n)
{
    if (oldst == n)
		return;
	
    if (oldst<n) {       // Need to add
		for (int i = oldst; i < n; i++)
			tuner[i]->show();
    } else {             // Need to delete
		for (int i = n; i < oldst; i++)
			tuner[i]->hide();
    }
    oldst = n;

    setMinimumSize(330, 90+30*n);
    reposTuners();
}

void SetTabDrum::resizeEvent(QResizeEvent *e)
{
    reposTuners();
}

void SetTabDrum::reposTuners()
{
    int s = dr->value();                // Current number of tuners

    int tw = (width() - 20) / s;        // Width of one tuner
    int th = height() - 90;             // Height of one tuner

    for (int i = 0; i < s; i++)
		tuner[i]->setGeometry(10 + i * tw, 80, tw, th);
}
