#ifndef OPTIONS_H
#define OPTIONS_H

#include <kdialogbase.h>
#include "global.h"
#include "qcheckbox.h"

class QButtonGroup;
class QRadioButton;
class QListView;
class QListBox;
class QSlider;

#ifdef WITH_TSE3
#include <tse3/MidiScheduler.h>
#endif

class Options: public KDialogBase {
    Q_OBJECT
public:
    Options(
#ifdef WITH_TSE3
			TSE3::MidiScheduler *_sch,
#endif
			QWidget *parent = 0, char *name = 0,//##
			bool modal = TRUE);

    QButtonGroup *maj7gr,*flatgr, *texlygr, *texsizegr, *texexpgr;
    QRadioButton *maj7[3],*flat[2], *tabsize[4], *expmode[2];
    QCheckBox *showbarnumb, *showstr, *showpagenumb;

protected:
    void setupTheoryTab();
    void setupMusixtexTab();
    void setupMidiTab();

protected slots:
	void fillMidiBox();
	void applyBtnClicked();
	void defaultBtnClicked();

private:
#ifdef WITH_TSE3
	QListView *midiport;
	TSE3::MidiScheduler *sch;
#endif
};

#endif
