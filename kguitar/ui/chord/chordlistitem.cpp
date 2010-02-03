#include "chordlistitem.h"
#include "settings.h"

int ChordListItem::toneshift[6] = { 3, 7, 10, 2, 5, 9 };

ChordListItem::ChordListItem(int _tonic, int _bass, int s3, int s5, int s7,
							 int s9, int s11, int s13)
	: QListWidgetItem()
{
	// MEMORIZING STEPS INFO IN THE COMBOBOX SELECTION FORM
	t = _tonic;
	s[0] = s3;
	s[1] = s5;
	s[2] = s7;
	s[3] = s9;
	s[4] = s11;
	s[5] = s13;

	setText(name());
}

int ChordListItem::step(int i) {
	if (s[i] == -1) {
		return 0;
	} else {
		return s[i] - ChordListItem::toneshift[i] + 2;
	}
}

bool ChordListItem::operator<(const QListWidgetItem &other) const
{
	return text().length() < other.text().length();
}

QString ChordListItem::name()
{
	QString name = Settings::noteName(t);

	int s3 = s[0];
	int s5 = s[1];
	int s7 = s[2];
	int s9 = s[3];
	int s11 = s[4];
	int s13 = s[5];

	// Special cases
	if ((s3 == -1) && (s5 == 7) && (s7 == -1) &&
		(s9 == -1) && (s11 == -1) && (s13 == -1)) {
		return name + "5";
	}
	if ((s3 == 4) && (s5 == 8) && (s7 == -1) &&
		(s9 == -1) && (s11 == -1) && (s13 == -1)) {
		return name + "aug";
	}

	if ((s3 == 3) && (s5 == 6) && (s7 == 9)) {
		name = name + "dim";
	} else {
		if (s3 == 3)
			name = name + "m";

		if (s5 == 6)
			name = name + "/5" + Settings::flatName();
		if (s5 == 8)
			name = name + "/5" + Settings::sharpName();
		if (((s5 == 6) || (s5 == 8)) && ((s7 != -1) || (s9 != -1) ||
										 (s11 != -1) || (s13 != -1)))
			name = name + "/";

		if ((s7 == 10) && (s9 == -1))
			name = name + "7";
		if (s7 == 11)
			name = name + Settings::maj7Name();
		if (s7 == 9)
			name = name + "6";
		if (((s7 == 11) || (s7 == 9))
			&& ((s9 != -1) || (s11 != -1) || (s13 != -1)))
			name = name + "/";
	}

	if ((s7 == -1) && (s9 != -1))
		name = name + "add";
	if ((s9 == 2) && (s11 == -1))
		name = name + "9";
	if (s9 == 1)
		name = name + "9" + Settings::flatName();
	if (s9 == 3)
		name = name + "9" + Settings::sharpName();
	if (((s9 == 1) || (s9 == 3)) && ((s11 != -1) || (s13 != -1)))
		name = name + "/";

	if ((s9 == -1) && (s11 != -1))
		name = name + "add";
	if ((s11 == 5) && (s13 == -1))
		name = name + "11";
	if (s11 == 4)
		name = name + "11" + Settings::flatName();
	if (s11 == 6)
		name = name + "11" + Settings::sharpName();
	if (((s11 == 4) || (s11 == 6)) && (s13 != -1))
		name = name + "/";

	if ((s11 == -1) && (s13 != -1))
		name = name + "add";
	if (s13 == 9)
		name = name + "13";
	if (s13 == 8)
		name = name + "13" + Settings::flatName();
	if (s13 == 10)
		name = name + "13" + Settings::sharpName();

	if (s3 == 2)
		name = name + "sus2";
	if (s3 == 5)
		name = name + "sus4";

	if ((s3 == -1) && (s5 == -1)) {
		name = name + " (no3no5)";
	} else {
		if (s3 == -1)
			name = name + " (no3)";
		if (s5 == -1)
			name = name + " (no5)";
	}

	return name;
}
