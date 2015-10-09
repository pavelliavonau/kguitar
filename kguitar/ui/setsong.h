#ifndef SETSONG_H
#define SETSONG_H

#include <KDialog>
#include <QMap>

#include "ui_setsong.h"

class SetSong: public KDialog, private Ui::SetSong {
	Q_OBJECT

public:
	SetSong(QMap<QString, QString> info, int _tempo, bool ro, QWidget *parent = 0);
	QMap<QString, QString> info();
	int tempo() { return tempoEdit->value(); }

private:
	QMap<QString, QString> m_info;
};

#endif
