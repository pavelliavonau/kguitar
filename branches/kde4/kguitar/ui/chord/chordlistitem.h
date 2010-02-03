#ifndef CHORDLISTITEM_H
#define CHORDLISTITEM_H

#include "global.h"
#include <QListWidgetItem>

/**
 * Item for a list of chords - a chord fingering.
 *
 * This item is able to hold all chord-specific data, such as tonic
 * and all steps presence. Automatically forms proper text chord name
 * from this data.
 */
class ChordListItem: public QListWidgetItem {
public:
	ChordListItem(int _tonic, int _bass, int s3, int s5, int s7,
				  int s9, int s11, int s13);
	int tonic() { return t; }
	int step(int x);
	virtual bool operator<(const QListWidgetItem &other) const;

private:
	/**
	 * Generates human-readable name of a chord from chord data.
	 * This name is readable as text().
	 */
	QString name();

	int t;
	int s[6];

	static int toneshift[6];
};

#endif
