#include "options.h"

#include <kapp.h>
#include <qlayout.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>

#ifdef HAVE_LIBASOUND
#include <sys/asoundlib.h>
#include <qlistbox.h>
#include <qlabel.h>
#endif

Options::Options(QWidget *parent=0, const char *name=0): QTabDialog(parent,name,TRUE)
{
    //////////////////////////////////////////////////////////////////
    // CHORD DIALOG SETTINGS TAB
    //////////////////////////////////////////////////////////////////

    QWidget *cd = new QWidget(this);

    // Dominant 7th name selection group

    maj7gr = new QButtonGroup(i18n("Dominant 7th"),cd);
    maj7gr->setGeometry(10,10,150,110);
    maj7[0] = new QRadioButton("7M",maj7gr);
    maj7[1] = new QRadioButton("maj7",maj7gr);
    maj7[2] = new QRadioButton("dom7",maj7gr);

    QVBoxLayout *vb1 = new QVBoxLayout(maj7gr,15,10);
    vb1->addSpacing(5); // Cosmetic space
    for (int i = 0; i < 3; i++)
		vb1->addWidget(maj7[i]);
    vb1->activate();

    // Chord step alterations selection group

    flatgr = new QButtonGroup(i18n("Alterations"),cd);
    flatgr->setGeometry(170,10,150,110);
    flat[0] = new QRadioButton(i18n("-/+ symbols"),flatgr);
    flat[1] = new QRadioButton(i18n("b/# symbols"),flatgr);

    QVBoxLayout *vb2 = new QVBoxLayout(flatgr,15,10);
    vb2->addSpacing(5); // Cosmetic space
    vb2->addWidget(flat[0]);
    vb2->addWidget(flat[1]);
    vb2->activate();

    addTab(cd,i18n("&Chords"));

    ///////////////////////////////////////////////////////////////////
    // MusiXTeX Settings Tab  - alinx
    ///////////////////////////////////////////////////////////////////

    QWidget *tex = new QWidget(this);

    texlygr = new QButtonGroup(i18n("MusiXTeX Layout"),tex);
    texlygr->setGeometry(10,10,175,130);
    showbarnumb = new QCheckBox(i18n("Show Barnumber"),texlygr);
    showbarnumb->setGeometry(10,35,150,20);
    showstr = new QCheckBox(i18n("Show Tuning"),texlygr);
    showstr->setGeometry(10,60,150,20);
    showpagenumb = new QCheckBox(i18n("Show Pagenumber"),texlygr);
    showpagenumb->setGeometry(10,85,150,20);

    texsizegr = new QButtonGroup(i18n("Tab Size"),tex);
    texsizegr->setGeometry(200,10,175,130);
    tabsize[0] = new QRadioButton(i18n("Smallest"),texsizegr);
    tabsize[1] = new QRadioButton(i18n("Small"),texsizegr);
    tabsize[2] = new QRadioButton(i18n("Normal"),texsizegr);
    tabsize[3] = new QRadioButton(i18n("Big"),texsizegr);

    QVBoxLayout *texvb1 = new QVBoxLayout(texsizegr,15,10);
    texvb1->addSpacing(5); // Cosmetic space
    for (int i = 0; i < 4; i++)
	    texvb1->addWidget(tabsize[i]);
    texvb1->activate();

    addTab(tex, i18n("MusiXTeX Export"));

#ifdef HAVE_LIBASOUND
    //////////////////////////////////////////////////////////////////
	// ALSA MIDI SETTINGS
    //////////////////////////////////////////////////////////////////

    QWidget *alsa = new QWidget(this);

	alsaport = new QListBox(alsa);
	fillAlsaBox();

	QLabel *alsaport_l = new QLabel(alsaport, i18n("MIDI &output port"), alsa);
	alsaport_l->setMinimumSize(100, 20);

	QVBoxLayout *alsavb = new QVBoxLayout(alsa, 10);
	alsavb->addWidget(alsaport_l);
	alsavb->addWidget(alsaport, 1);
	alsavb->activate();

	addTab(alsa, i18n("ALSA"));
#endif

    //////////////////////////////////////////////////////////////////
    // REST OF TABDIALOG SETTINS
    //////////////////////////////////////////////////////////////////

    setOkButton(i18n("OK"));
    setCancelButton(i18n("Cancel"));

    resize(400,300);
    setCaption(i18n("General options"));
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
				alsaport->insertItem(QString(cinfo.name) + QString(": ") + QString(pinfo.name));
//				printf("%3d:%-3d   %-30.30s    %s\n",
//					   pinfo.client, pinfo.port, cinfo.name, pinfo.name);
			}
		}
	}
#endif HAVE_LIBASOUND
}
