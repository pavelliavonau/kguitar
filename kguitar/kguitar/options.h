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
//##class DeviceManager;

class Options: public KDialogBase
{
    Q_OBJECT
public:
    Options(/*DeviceManager *_dm,*/ QWidget *parent = 0, char *name = 0,//##
			bool modal = TRUE);

    QButtonGroup *maj7gr,*flatgr, *texlygr, *texsizegr, *texexpgr;
    QRadioButton *maj7[3],*flat[2], *tabsize[4], *expmode[2];
    QCheckBox *showbarnumb, *showstr, *showpagenumb;

protected:
    void setupTheoryTab();
    void setupMusixtexTab();
    void setupAlsaTab();
	void setupKmidTab();

// Though these aren't used in any compilation, they're here
private slots:
	void fillAlsaBox();
    void applyBtnClicked();
    void defaultBtnClicked();

private:
	QListBox *kmidport;
	QListView *alsaport;

//##	DeviceManager *dm;
};

#endif
