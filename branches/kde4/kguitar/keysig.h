/***************************************************************************
 * keysig.h: definition of SetKeySig class
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

#ifndef KEYSIG_H
#define KEYSIG_H

#include <kdialog.h>
#include "global.h"

class QComboBox;

class SetKeySig: public KDialog {
	Q_OBJECT

public:
	SetKeySig(int keySig, QWidget *parent = 0, const char *name = 0);
	int keySignature();

protected:
	QComboBox *sig;
};

#endif
