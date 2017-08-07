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

#include <KLocalizedString>

#include <qcombobox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <QHBoxLayout>
#include <QDialogButtonBox>

SetKeySig::SetKeySig(int keySig, QWidget *parent)
	: QDialog(parent)
{
	setWindowTitle(i18n("Key signature"));
	setModal(true);

	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
	                                                 | QDialogButtonBox::Cancel);

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

	sig = new QComboBox();
	sig->setInsertPolicy(QComboBox::NoInsert);
	sig->insertItems(sig->count(),signatures);
	sig->setCurrentIndex(7 - keySig);

	QLabel *sig_l = new QLabel(i18n("Flats / sharps:"));
	sig_l->setBuddy(sig);

	QHBoxLayout *l = new QHBoxLayout();
	l->addWidget(sig_l);
	l->addWidget(sig);
	l->activate();

	QVBoxLayout *mainLayout = new QVBoxLayout(this);
	mainLayout->addLayout(l);
	mainLayout->addWidget(buttonBox);
	setLayout(mainLayout);

	connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

// note that sig->currentItem() return 0 for "7 sharps", 1 for "6 sharps" etc.
// use 7 - sig->currentItem() to get the key number
int SetKeySig::keySignature()
{
        return 7 - sig->currentIndex();
}
