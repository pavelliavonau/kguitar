#ifndef CHORDANALYZER_H
#define CHORDANALYZER_H

#include "global.h"

class ChordAnalyzer {
public:
	ChordAnalyzer(QString name);
	bool analyze();

	int tonic;
	int step[6];
	bool result;
	QString msg;

private:
	QString name;
	bool fixed[6];
	int pos;

	bool checkNext(QString sample);
	bool setStep(int step, int value, QString reason);
};

#endif
