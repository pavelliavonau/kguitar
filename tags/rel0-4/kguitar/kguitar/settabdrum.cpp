#include "settabdrum.h"

#include <klocale.h>
#include <qlabel.h>

SetTabDrum::SetTabDrum(QWidget *parent=0, const char *name=0)
	: QWidget(parent, name)
{
    // Controls

    dr = new QSpinBox(1, MAX_STRINGS, 1, this);
    connect(dr, SIGNAL(valueChanged(int)), SLOT(stringChanged(int)));
    dr->setGeometry(90, 20, 40, 20);

    QLabel *dr_l = new QLabel(i18n("Drums:"), this);
    dr_l->setGeometry(10, 20, 50, 20);

    // Tuners

	for (int i = 0; i < MAX_STRINGS; i++) {
		tuner[i] = new QSpinBox(this);
		tname[i] = new QLineEdit(this);
		tname[i]->setEnabled(FALSE);
	}

    oldst = MAX_STRINGS;
}

void SetTabDrum::stringChanged(int n)
{
    if (oldst == n)
		return;
	
    if (oldst<n) {       // Need to add
		for (int i = oldst; i < n; i++) {
			tuner[i]->show();
			tname[i]->show();
		}
    } else {             // Need to delete
		for (int i = n; i < oldst; i++) {
			tuner[i]->hide();
			tname[i]->hide();
		}
    }
    oldst = n;

    setMinimumSize(200, 90 + 25*n);
    reposTuners();
}

void SetTabDrum::resizeEvent(QResizeEvent *e)
{
    reposTuners();
}

void SetTabDrum::reposTuners()
{
    for (int i = 0; i < dr->value(); i++) {
		tuner[i]->setGeometry(10, 40 + i * 25, 50, 25);
		tname[i]->setGeometry(70, 40 + i * 25, width() - 80, 25);
	}
}
