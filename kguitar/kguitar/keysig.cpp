/***************************************************************************
 * keysig.cpp: implementation of SetKeySig class
 *
 * A very simple-minded set key signature dialog.
 * Requires entering the number of flats or sharps.
 * LVIFIX:
 * A more advanced implementation would (also) show the key name
 * and the graphical representation  (e.g. A major = F#,C#,G#).
 * It would also return the key number instead of an index in a list.
 *
 * This file is part of KGuitar, a KDE tabulature editor
 *
 * copyright (C) 2003 the KGuitar development team
 ***************************************************************************/

/***************************************************************************
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * See the file COPYING for more information.
 ***************************************************************************/

#include "keysig.h"

#include <klocale.h>

#include <qspinbox.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qlayout.h>

// list of item names for sig->insertStrList()
// note that sig->currentItem() return 0 for "7 sharps", 1 for "6 sharps" etc.
// use 7 - sig->currentItem() to get the key number

static const char * items[] = {
	"7 sharps",
	"6 sharps",
	"5 sharps",
	"4 sharps",
	"3 sharps",
	"2 sharps",
	"1 sharp",
	"none",
	"1 flat",
	"2 flats",
	"3 flats",
	"4 flats",
	"5 flats",
	"6 flats",
	"7 flats",
	0
};

SetKeySig::SetKeySig(QWidget *parent, const char *name):
    QDialog(parent, name, TRUE)
{
    sig = new QComboBox(TRUE,this);
	sig->setEditable(false);
    sig->setInsertionPolicy(QComboBox::NoInsertion);
    sig->insertStrList(items);

    QLabel *sig_l = new QLabel(sig,i18n("Flats/sharps:"),this);

    QPushButton *ok = new QPushButton(i18n("OK"),this);
    connect(ok,SIGNAL(clicked()),SLOT(accept()));
    QPushButton *cancel = new QPushButton(i18n("Cancel"),this);
    connect(cancel,SIGNAL(clicked()),SLOT(reject()));

    QVBoxLayout *l = new QVBoxLayout(this,10);

    QGridLayout *g = new QGridLayout(2,2,5);
    l->addLayout(g,1);
    g->addWidget(sig_l,1,0);
    g->addWidget(sig,1,1);
    g->setColStretch(0,2);
    g->setColStretch(1,1);
    g->addColSpacing(0,150);
    g->addColSpacing(1,50);
    g->addRowSpacing(0,25); g->addRowSpacing(1,25);

// LVIFIX: may need this later
//    toend = new QCheckBox(i18n("Apply till the &end"),this);
//    toend->setMinimumSize(100,25);
//    l->addWidget(toend,1);

    QHBoxLayout *b = new QHBoxLayout(10);
    l->addLayout(b);
    b->addWidget(ok);
    b->addWidget(cancel);
    b->addStrut(30);

    l->activate();

    resize(0,0);
    setCaption(i18n("Key signature"));
}
