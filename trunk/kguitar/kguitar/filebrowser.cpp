#include <klocale.h>
#include <qapp.h>
#include <qtooltip.h>
#include <qdir.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qlayout.h>

#include <kmessagebox.h>
#include <kiconloader.h>

#include "application.h"
#include "trackview.h"
#include "tabsong.h"
#include "globaloptions.h"
#include "filebrowser.h"

//---------------------------------------------
Directory::Directory(Directory *parent, const char *filename)
	: QListViewItem(parent), f(filename)
{
	p = parent;
	readable = TRUE;
}


Directory::Directory(QListView *parent)
	: QListViewItem(parent), f("/")
{
	p = 0;
	readable = TRUE;
}


void Directory::setOpen(bool o)
{
	if (o && !childCount()) {
		QString s(fullName());
		QDir thisDir(s);
		if (!thisDir.isReadable()) {
			readable = FALSE;
			return;
		}

		const QFileInfoList *files = thisDir.entryInfoList();
		if (files) {
			QFileInfoListIterator it(*files);
			QFileInfo * f;
			while((f = it.current()) != 0) {
				++it;
				if (f->fileName() == "." || f->fileName() == "..")
					; // nothing
				else
					if (f->isDir())
						new Directory(this, f->fileName());
			}
		}
	}
	QListViewItem::setOpen(o);
}

void Directory::setup()
{
    setExpandable(TRUE);
    QListViewItem::setup();
}

QString Directory::fullName()
{
	QString s;
	if (p) {
		s = p->fullName();
		s.append( f.name() );
		s.append( "/" );
	}
	else {
		s = "/";
	}
	return s;
}

QString Directory::text(int column) const
{
	if (column == 0)
		return f.name();
	else if (readable)
		return "Directory";
	else
		return "Unreadable Directory";
}

//---------------------------------------------


FileBrowser::FileBrowser(QWidget *parent, const char *name)
	: KDialog(parent, name, TRUE)
{
    m_haveMidi = globalHaveMidi;  // ALINXFIX: Will be removed
    globalHaveMidi = FALSE;       // when Midi is implemented !!

	p = parent;

	setCaption(i18n("File browser"));

	btnclose = new QPushButton(i18n("&Close"), this);
	btnclose->setMinimumSize(75, 24);
	connect(btnclose, SIGNAL(clicked()), SLOT(closeDlg()));

	btnscan = new QPushButton(i18n("&Scan"), this);
	btnscan->setMinimumSize(75, 24);
	connect(btnscan, SIGNAL(clicked()), SLOT(scanDir()));
	QToolTip::add(btnscan, i18n("Scan Subdirectories"));

    if (globalHaveMidi) {
        btnplay = new QPushButton(i18n("&Play"), this);
        btnplay->setMinimumSize(75, 24);
        connect(btnplay, SIGNAL(clicked()), SLOT(playSong()));
        QToolTip::add(btnplay, i18n("Play score"));
        btnplay->setToggleButton(TRUE);

        jumpcombo = new QComboBox(this);
        jumpcombo->setMinimumSize(150, 24);
        jumpcombo->setMaximumSize(150, 24);
        jumpcombo->insertItem(i18n("No Jump"));
        jumpcombo->insertItem(i18n("Jump after 1 bar"));
        jumpcombo->insertItem(i18n("Jump after 2 bars"));
        jumpcombo->insertItem(i18n("Jump after 5 bars"));
        jumpcombo->insertItem(i18n("Jump after 10 bars"));
        jumpcombo->insertItem(i18n("Jump after 25 bars"));
        jumpcombo->insertItem(i18n("Jump after the score"));
    }

	dirlist = new QListView(this);
	dirlist->setMinimumSize(200, 340);
	dirlist->addColumn(i18n("Directories"));
	dirlist->setFrameStyle(QFrame::Panel | QFrame::Sunken);
	connect(dirlist, SIGNAL(currentChanged(QListViewItem*)),
			this, SLOT(fillFileView(QListViewItem*)));
	connect(dirlist, SIGNAL(doubleClicked(QListViewItem*)),
			this, SLOT(fillFileView(QListViewItem*)));

	fileview = new QListView(this);
	fileview->setMinimumSize(380, 250);
	fileview->addColumn(i18n("Name"));
	fileview->addColumn(i18n("Size"));
	fileview->addColumn(i18n("Modified"));
	fileview->addColumn(i18n("Directory"));
	fileview->setAllColumnsShowFocus(TRUE);
	fileview->setFrameStyle(QFrame::Panel | QFrame::Sunken);
	connect(fileview, SIGNAL(currentChanged(QListViewItem*)),
			this, SLOT(loadSong(QListViewItem*)));

	titlelabel = new QLabel(this);
	titlelabel->setMinimumSize(100, 20);
	titlelabel->setMaximumSize(100, 20);
	titlelabel->setText(i18n("Title:"));

	authorlabel = new QLabel(this);
	authorlabel->setMinimumSize(100, 20);
	authorlabel->setMaximumSize(100, 20);
	authorlabel->setText(i18n("Author:"));

	translabel = new QLabel(this);
	translabel->setMinimumSize(100, 20);
	translabel->setMaximumSize(100, 20);
	translabel->setText(i18n("Transcriber:"));

	tlabel = new QLabel(this);
	tlabel->setMinimumSize(260, 20);

	alabel = new QLabel(this);
	alabel->setMinimumSize(260, 20);

	tslabel = new QLabel(this);
	tslabel->setMinimumSize(260, 20);

	//Dialog Layout
	QBoxLayout *l = new QHBoxLayout(this, 3, 3);

	QGridLayout *lwg = new QGridLayout(this, 3, 2, 3, 0);
	lwg->addWidget(titlelabel, 0, 0);
	lwg->addWidget(authorlabel, 1, 0);
	lwg->addWidget(translabel, 2, 0);
	lwg->addWidget(tlabel, 0, 1);
	lwg->addWidget(alabel, 1, 1);
	lwg->addWidget(tslabel, 2, 1);
	lwg->setColStretch(0, 0);
	lwg->setColStretch(1, 2);
	lwg->activate();

	QBoxLayout *lright = new QVBoxLayout(this);
	lright->addWidget(fileview);
	lright->addLayout(lwg);
	lright->activate();

	QBoxLayout *lbtn = new QHBoxLayout(this, 5, 5);
    if (globalHaveMidi) {
        lbtn->addWidget(jumpcombo);
        lbtn->addWidget(btnplay);
    }
	lbtn->addWidget(btnscan);
	lbtn->addWidget(btnclose);
	lbtn->activate();

	lright->addLayout(lbtn);

	l->addWidget(dirlist, 1);
	l->addLayout(lright, 3);
	l->activate();

	//FILL dirlist
	directory = new Directory(dirlist);
	directory->setOpen(TRUE);
}

FileBrowser::~FileBrowser()
{
    globalHaveMidi = m_haveMidi;  // ALINXFIX: Will be removed when Midi is implemented !!
}

void FileBrowser::closeDlg()
{
	close();
}

QString FileBrowser::getFullPath(QListViewItem* item)
{
	QListViewItem *p;
	QString tmp, s;

	tmp = item->text(0);
	p = item->parent();
	while (p != 0){
		s = p->text(0);
		p = p->parent();
		if (p == 0)
			tmp = s + tmp;
		else
			tmp = s + "/" + tmp;
	}
	return tmp;
}

void FileBrowser::scanSubDirs(QString path)
{
	QListViewItem* lv;
	QString fname, fsize, fmodified, fdir;

	qApp->processEvents();
	QDir dir(path);
	if (!dir.isReadable())
		return;

	dir.setFilter(QDir::Dirs | QDir::Files | QDir::NoSymLinks);
	const QFileInfoList* fileinfolist = dir.entryInfoList();
	QFileInfoListIterator it(*fileinfolist);
	QFileInfo* fi;

	while ((fi = it.current()) != 0) {
		if (fi->fileName() == "." || fi->fileName() == ".."){
			++it;
			continue;
		}
		if (fi->isFile() && fi->isReadable()){
			QString ext = fi->extension();
			ext = ext.upper();
			if (ext == "KG") {
				fname = fi->fileName();
				fsize.setNum(fi->size());
				fmodified = fi->lastModified().toString().latin1();
				fdir = fi->dirPath().latin1();
				lv = new QListViewItem(fileview, fname, fsize, fmodified, fdir);
			}
			qApp->processEvents();
		} else
			if (fi->isDir()) scanSubDirs(fi->absFilePath());
		++it;
	}
}

void FileBrowser::scanDir()
{
	QListViewItem* lv;

	fileview->clear();
	lv = dirlist->currentItem();
	if (lv == 0){
		KMessageBox::information(this, i18n("Please select a directory!"),
								 i18n("File browser"));
		return;
	}

    if (globalHaveMidi) {
        btnplay->setEnabled(FALSE);
        jumpcombo->setEnabled(FALSE);
    }
	btnclose->setEnabled(FALSE);
	btnscan->setEnabled(FALSE);
	qApp->processEvents();

	QDir dir(getFullPath(lv));
	if (dir.isReadable())
		scanSubDirs(getFullPath(lv));
	else
		KMessageBox::sorry(this, i18n("You have no permission to read this directory!"),
						   i18n("File browser"));

    if (globalHaveMidi) {
        btnplay->setEnabled(TRUE);
        jumpcombo->setEnabled(TRUE);
    }
	btnclose->setEnabled(TRUE);
	btnscan->setEnabled(TRUE);
	qApp->processEvents();

	if (fileview->childCount() == 0)
		KMessageBox::sorry(this, i18n("There are no files."), i18n("File browser"));
}

void FileBrowser::playSong()
{
}

void FileBrowser::fillFileView(QListViewItem* item)
{
	QListViewItem* lv;
	QString fname, fsize, fmodified, fdir;

	fileview->clear();
	QDir dir(getFullPath(item));
	if (!dir.isReadable()){
		KMessageBox::sorry(this, i18n("You have no permission to read this directory!"),
						   i18n("File browser"));
		return;
	}

	dir.setFilter(QDir::Dirs | QDir::Files | QDir::NoSymLinks);
	const QFileInfoList* fileinfolist = dir.entryInfoList();
	QFileInfoListIterator it(*fileinfolist);
	QFileInfo* fi;

	while ((fi = it.current()) != 0){
		if (fi->fileName() == "." || fi->fileName() == ".."){
			++it;
			continue;
		}
		if (fi->isFile() && fi->isReadable()){
			QString ext = fi->extension();
			ext = ext.upper();
			if (ext == "KG"){
				fname = fi->fileName();
				fsize.setNum(fi->size());
				fmodified = fi->lastModified().toString().latin1();
				fdir = fi->dirPath().latin1();
				lv = new QListViewItem(fileview, fname, fsize, fmodified, fdir);
			}
		}
		++it;
	}
	if (fileview->childCount() == 0)
		lv = new QListViewItem(fileview, i18n("No files"));
		//KMessageBox::sorry(this, i18n("There are no files."), i18n("File browser"));
}

void FileBrowser::loadSong(QListViewItem* item)
{
	QString fname, fpath;

	fname = item->text(0);

	if (fname == i18n("No files"))
		return;

	fpath = item->text(3);
	fname = fpath + "/" + fname;

	emit loadFile(KURL(fname));
}
