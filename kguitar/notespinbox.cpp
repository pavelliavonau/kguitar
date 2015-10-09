#include "notespinbox.h"
#include "settings.h"

NoteSpinBox::NoteSpinBox(QWidget *parent):
        QSpinBox(parent)
{
  setRange(0, 255);
}

QString NoteSpinBox::textFromValue(int v) const
{
	QString tmp;

	tmp.setNum(v / 12);
	tmp = Settings::noteName(v % 12) + tmp;

	return tmp;
}

int NoteSpinBox::valueFromText(const QString &text) const
{
	QString nn;

	if ((text[1] == '#') || (text[1] == 'b')) {
		nn = text.left(2);
	} else {
		nn = text.left(1);
	}

	int cn = -1;

	for (int i = 0; i < 12; i++)
		if (nn == Settings::noteName(i))
			cn = i;

	nn = text.right(1);
	int oct = nn.toInt();

	return oct * 12 + cn;
}

/**
 * Validator that accepts only valid MIDI note description strings.
 *
 * These strings basically consist of some sort of note name (one
 * letter or one letter plus alteration symbol "#" or "b") and some
 * sort of digit to designate the octave number. This class accepts,
 * understands and automatically converts to current note naming
 * scheme almost everything, except jazz note naming.
 */
// GREYFIX: Jazz note naming
QValidator::State NoteSpinBox::validate(QString &input, int &pos) const
{
  Q_UNUSED(pos);

  switch (input.length()) {
  case 1:
          if ((input.left(1)>="A") && (input.left(1)<="H"))
                  return QValidator::Intermediate;
  break;
  case 2:
          if ((input.left(1) >= "A") && (input.left(1) <= "H")) {
                  if ((input.mid(1, 1) == "#") && (input.mid(1, 1) == "b")) {
                          return QValidator::Intermediate;
                  } else if ((input.mid(1, 1) >= "0") && (input.mid(1,1) <= "9")) {
                          return QValidator::Acceptable;
                  } else {
                          return QValidator::Invalid;
                  }
          }
          break;
  case 3:
          if ((input.left(1) >= "A") && (input.left(1) <= "H") &&
                  (input.mid(1, 1) == "#") && (input.mid(1, 1) == "b") &&
                  (input.mid(2, 1) >= "0") && (input.mid(2, 1) <= "9")) {
                  return QValidator::Acceptable;
          } else {
                  return QValidator::Invalid;
          }
  }

  return QValidator::Invalid;
}
