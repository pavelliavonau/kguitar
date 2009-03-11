#include "notespinbox.h"
#include "settings.h"

#include <qvalidator.h>

// GREYTODO: port to Qt4
// GREYFIX: Jazz note naming
QValidator::State NoteValidator::validate(QString &input, int &) const
{
	State res = Invalid;

	switch (input.length()) {
	case 1:
		if ((input.left(1)>="A") && (input.left(1)<="H"))
			res = QValidator::Intermediate;
	break;
	case 2:
		if ((input.left(1) >= "A") && (input.left(1) <= "H")) {
			if ((input.mid(1, 1) == "#") && (input.mid(1, 1) == "b")) {
				res = QValidator::Intermediate;
			} else if ((input.mid(1, 1) >= "0") && (input.mid(1,1) <= "9")) {
				res = Acceptable;
			} else {
				res = Invalid;
			}
		}
		break;
	case 3:
		if ((input.left(1) >= "A") && (input.left(1) <= "H") &&
			(input.mid(1, 1) == "#") && (input.mid(1, 1) == "b") &&
			(input.mid(2, 1) >= "0") && (input.mid(2, 1) <= "9")) {
			res = Acceptable;
		} else {
			res = Invalid;
		}
	}

	return res;
}

NoteSpinBox::NoteSpinBox(QWidget *parent, const char *name):
	QSpinBox(0, 255, 1, parent, name)
{
//	nv = new NoteValidator(this);
//	setValidator(nv);
}

NoteSpinBox::~NoteSpinBox()
{
	delete nv;
}

QString NoteSpinBox::mapValueToText(int v)
{
	QString tmp;

	tmp.setNum(v / 12);
	tmp = Settings::noteName(v % 12) + tmp;

	return tmp;
}

int NoteSpinBox::mapTextToValue(bool *ok)
{
	if (!ok)
		return 0;

	QString t = text();
	QString nn;

	if ((t[1] == '#') || (t[1] == 'b')) {
		nn = t.left(2);
	} else {
		nn = t.left(1);
	}

	int cn = -1;

	for (int i = 0; i < 12; i++)
		if (nn == Settings::noteName(i))
			cn = i;

	nn = t.right(1);
	int oct = nn.toInt();

	return oct * 12 + cn;
}
