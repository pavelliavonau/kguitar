#ifndef RHYTHMEDITOR_H
#define RHYTHMEDITOR_H

#include "ui_rhythmeditor.h"
#include <QTime>

class RhythmEditor : public QDialog, private Ui::RhythmEditor {
	Q_OBJECT
public:
	RhythmEditor(QWidget *parent = 0);
	QList<int> quantizedDurations();

public slots:
	void tap();
	void quantize();
	void tempoState(bool state);

protected:
	void changeEvent(QEvent *e);

private:
	QTime time;
};

#endif // RHYTHMEDITOR_H
