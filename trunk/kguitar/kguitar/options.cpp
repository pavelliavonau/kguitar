#include "options.h"

#include <klocale.h>
#include <kiconloader.h>
#include <kiconeffect.h>

#include <qlayout.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>

#ifdef HAVE_LIBASOUND
#include <sys/asoundlib.h>
#include <qframe.h>
#include <qlistview.h>
#include <qlabel.h>
#include <qpushbutton.h>
#endif

Options::Options(QWidget *parent = 0, char *name = 0, bool modal)
	: KDialogBase(IconList, i18n("Preferences"), Ok|Cancel, Ok, parent, name,
				  modal, TRUE)
{
	//Setup Tabs
	setupChordTab();
	setupMusixtexTab();
#ifdef HAVE_LIBASOUND
	setupAlsaTab();
#endif
	resize(530,300);
}

void Options::setupChordTab()
{
    //////////////////////////////////////////////////////////////////
    // CHORD DIALOG SETTINGS TAB
    //////////////////////////////////////////////////////////////////

	// ALINXFIX: find or make a better icon for this page !!!
	QFrame *cd = addPage(i18n("Chord"), 0, DesktopIcon("looknfeel", KIcon::SizeMedium));

    // Dominant 7th name selection group

	maj7gr = new QButtonGroup(i18n("Dominant 7th"), cd);
	maj7gr->setMinimumSize(150, 110);
	maj7[0] = new QRadioButton("7M", maj7gr);
	maj7[1] = new QRadioButton("maj7", maj7gr);
	maj7[2] = new QRadioButton("dom7", maj7gr);

	QVBoxLayout *vb1 = new QVBoxLayout(maj7gr, 15, 10);
	vb1->addSpacing(5); // Cosmetic space
	for (int i = 0; i < 3; i++)
		vb1->addWidget(maj7[i]);
	vb1->activate();

    // Chord step alterations selection group

	flatgr = new QButtonGroup(i18n("Alterations"), cd);
	flatgr->setMinimumSize(150, 110);
	flat[0] = new QRadioButton(i18n("-/+ symbols"), flatgr);
	flat[1] = new QRadioButton(i18n("b/# symbols"), flatgr);

	QVBoxLayout *vb2 = new QVBoxLayout(flatgr, 15, 10);
	vb2->addSpacing(5); // Cosmetic space
	vb2->addWidget(flat[0]);
	vb2->addWidget(flat[1]);
	vb2->activate();

	QHBoxLayout *vbcd = new QHBoxLayout(cd, 15, 10);
	vbcd->addWidget(maj7gr);
	vbcd->addWidget(flatgr);
	vbcd->activate();
}

void Options::setupMusixtexTab()
{
    ///////////////////////////////////////////////////////////////////
    // MusiXTeX Settings Tab  - alinx
    ///////////////////////////////////////////////////////////////////

	QFrame *tex = addPage(i18n("MusiXTeX Export"), 0, DesktopIcon("kcmsound", KIcon::SizeMedium));

	texlygr = new QButtonGroup(i18n("MusiXTeX Layout"), tex);
	texlygr->setMinimumSize(175, 130);
	showbarnumb = new QCheckBox(i18n("Show Barnumber"), texlygr);
	showbarnumb->setGeometry(10, 35, 150, 20);
	showstr = new QCheckBox(i18n("Show Tuning"), texlygr);
	showstr->setGeometry(10, 60, 150, 20);
	showpagenumb = new QCheckBox(i18n("Show Pagenumber"), texlygr);
	showpagenumb->setGeometry(10, 85, 150, 20);

	QVBoxLayout *texvb1 = new QVBoxLayout(texlygr, 15, 10);
	texvb1->addSpacing(5);
	texvb1->addWidget(showbarnumb);
	texvb1->addWidget(showstr);
	texvb1->addWidget(showpagenumb);
	texvb1->activate();

	texsizegr = new QButtonGroup(i18n("Tab Size"), tex);
	texsizegr->setMinimumSize(175, 130);
	tabsize[0] = new QRadioButton(i18n("Smallest"), texsizegr);
	tabsize[1] = new QRadioButton(i18n("Small"), texsizegr);
	tabsize[2] = new QRadioButton(i18n("Normal"), texsizegr);
	tabsize[3] = new QRadioButton(i18n("Big"), texsizegr);

    QVBoxLayout *texvb2 = new QVBoxLayout(texsizegr, 15, 10);
    texvb2->addSpacing(5); // Cosmetic space
    for (int i = 0; i < 4; i++)
	    texvb2->addWidget(tabsize[i]);
    texvb2->activate();

	QHBoxLayout *vbtex = new QHBoxLayout(tex, 15, 10);
	vbtex->addWidget(texlygr);
	vbtex->addWidget(texsizegr);
	vbtex->activate();
}

void Options::setupAlsaTab()
{
#ifdef HAVE_LIBASOUND
    //////////////////////////////////////////////////////////////////
	// ALSA MIDI SETTINGS
    //////////////////////////////////////////////////////////////////

	QFrame *alsa = addPage(i18n("ALSA"), 0, DesktopIcon("kcmmidi", KIcon::SizeMedium));

	alsaport = new QListView(alsa);
	alsaport->setSorting(-1); // no text sorting
	alsaport->setFrameStyle(QFrame::Panel | QFrame::Sunken);
	alsaport->addColumn(i18n("Port"));
	alsaport->addColumn(i18n("Client Info"));
	alsaport->addColumn(i18n("Port Info"));
	alsaport->setMinimumSize(350, 100);

	fillAlsaBox();

	QLabel *alsaport_l = new QLabel(alsaport, i18n("MIDI &output port"), alsa);
	alsaport_l->setMinimumSize(100, 15);

	QPushButton *alsarefresh = new QPushButton(i18n("&Refresh"), alsa);
	connect(alsarefresh, SIGNAL(clicked()), SLOT(fillAlsaBox()));
	alsarefresh->setMinimumSize(75, 30);

	QVBoxLayout *alsavb = new QVBoxLayout(alsa, 10, 5);
	alsavb->addWidget(alsaport_l);
	alsavb->addWidget(alsaport, 1);
	alsavb->addWidget(alsarefresh);
	alsavb->activate();

#endif
}


void Options::fillAlsaBox()
{
#ifdef HAVE_LIBASOUND
	snd_seq_client_info_t cinfo;
	snd_seq_port_info_t pinfo;
	snd_seq_system_info_t sysinfo;
	int client;
	int port;
	int err;
	snd_seq_t *handle;

	QString tmp, tmp2;

	alsaport->clear();

	err = snd_seq_open(&handle, SND_SEQ_OPEN);
	if (err < 0) {
		perror("Could not open sequencer");
		return;
	}

	err = snd_seq_system_info(handle, &sysinfo);
	if (err < 0) {
		perror("Could not get sequencer information");
		return;
	}

	int cap = (SND_SEQ_PORT_CAP_SUBS_WRITE | SND_SEQ_PORT_CAP_WRITE);

	for (client = 0; client < sysinfo.clients; client++) {
		err = snd_seq_get_any_client_info(handle, client, &cinfo);
		if (err < 0)
			continue;

		for (port = 0; port < sysinfo.ports; port++) {
			err = snd_seq_get_any_port_info(handle, client, port, &pinfo);
			if (err < 0)
				continue;
			
			if ((pinfo.capability & cap) == cap) {
				tmp.setNum(pinfo.client);
				tmp2.setNum(pinfo.port);
				tmp = tmp + ":" + tmp2;
				(void) new QListViewItem(alsaport, tmp, cinfo.name, pinfo.name);
			}
		}
	}
#endif
}
