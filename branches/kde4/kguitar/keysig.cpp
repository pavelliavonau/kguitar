/***************************************************************************
 * keysig.cpp: implementation of SetKeySig class
 *
 * A very simple-minded set key signature dialog.
 * Requires entering the number of flats or sharps.
 * LVIFIX:
 * A more advanced implementation would (also) show the key name
 * and the graphical representation  (e.g. A major = F#,C#,G#).
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

#include <qcombobox.h>
#include <qlabel.h>
#include <qlayout.h>
//Added by qt3to4:
#include <Q3HBoxLayout>

SetKeySig::SetKeySig(int keySig, QWidget *parent)
	: KDialog(parent)
{
	setCaption(i18n("Key signature"));
	setButtons(Ok | Cancel);
	setModal(true);

	QWidget *page = new QWidget(this);
	setMainWidget(page);

	QStringList signatures;
	signatures
		<< i18n("7 sharps") + " (C#/A#m)"
		<< i18n("6 sharps") + " (F/D#m)"
		<< i18n("5 sharps") + " (B/G#m)"
		<< i18n("4 sharps") + " (E/C#m)"
		<< i18n("3 sharps") + " (A/F#m)"
		<< i18n("2 sharps") + " (D/Bm)"
		<< i18n("1 sharp")  + " (G/Em)"
		<< i18n("none")     + " (C/Am)"
		<< i18n("1 flat")   + " (F/Dm)"
		<< i18n("2 flats")  + " (Bb/Gm)"
		<< i18n("3 flats")  + " (Eb/Cm)"
		<< i18n("4 flats")  + " (Ab/Fm)"
		<< i18n("5 flats")  + " (Db/Bbm)"
		<< i18n("6 flats")  + " (Gb/Ebm)"
		<< i18n("7 flats")  + " (Cb/Abm)";

	sig = new QComboBox(TRUE, page);
	sig->setEditable(false);
	sig->setInsertionPolicy(QComboBox::NoInsertion);
	sig->insertStringList(signatures);
	sig->setCurrentItem(7 - keySig);

	QLabel *sig_l = new QLabel(sig, i18n("Flats / sharps:"), page);

	Q3HBoxLayout *l = new Q3HBoxLayout(page, 0, spacingHint());
    l->addWidget(sig_l);
	l->addWidget(sig);
    l->activate();
}

// note that sig->currentItem() return 0 for "7 sharps", 1 for "6 sharps" etc.
// use 7 - sig->currentItem() to get the key number
int SetKeySig::keySignature()
{
	return 7 - sig->currentItem();
}
