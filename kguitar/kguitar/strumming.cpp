#include "strumming.h"

#include "strum.h"

#include <kapp.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qcombobox.h>

Strumming::Strumming(int default_scheme, QWidget *parent=0, const char *name=0):
	QDialog(parent, name, TRUE)
{
    QVBoxLayout *l = new QVBoxLayout(this, 10);

    QGridLayout *g = new QGridLayout(1, 2, 10);
    l->addLayout(g);

	// STRUMMING OPTIONS CONTROLS

	pattern = new QComboBox(FALSE, this);
	for (int i = 0; lib_strum[i].len[0]; i++)
		pattern->insertItem(lib_strum[i].name);
	pattern->setCurrentItem(default_scheme);

	QLabel *pattern_l = new QLabel(pattern, i18n("Strum &pattern:"), this);

	g->addWidget(pattern_l, 0, 0);
	g->addWidget(pattern, 0, 1);

	g->addRowSpacing(0, 30);

    g->addColSpacing(0, 80);
    g->addColSpacing(1, 200);
    g->setColStretch(1, 1);

    // DIALOG BUTTONS

    QHBoxLayout *butt = new QHBoxLayout();
    l->addLayout(butt);

    QPushButton *ok = new QPushButton(i18n("OK"), this);
    connect(ok,SIGNAL(clicked()),SLOT(accept()));
    QPushButton *cancel = new QPushButton(i18n("Cancel"), this);
    connect(cancel, SIGNAL(clicked()), SLOT(reject()));

    butt->addWidget(ok);
    butt->addWidget(cancel);
    butt->addStrut(30);
    
    l->activate();

    setCaption(i18n("Strumming pattern"));
	resize(0, 0);
}

int Strumming::scheme()
{
	return pattern->currentItem();
}
