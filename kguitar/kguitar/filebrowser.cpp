#include <klocale.h>
#include <qapp.h>
#include <qtooltip.h>
#include <qdir.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <kmessagebox.h>
#include <kiconloader.h>

#include "application.h"
#include "trackview.h"
#include "tabsong.h"
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


FileBrowser::FileBrowser(ApplicationWindow *parent, const char *name) 
	: QDialog(parent, name, TRUE)
{
	p = parent;

	resize(640, 430);
	setMinimumSize(640, 430);
	setFixedSize(640, 430);
	setCaption(i18n("File browser"));
	widget_1 = new QWidget(this);
	widget_1->setGeometry(10, 10, 420, 30);
	widget_1->setMinimumSize(0, 0);
	btnclose = new QPushButton(widget_1);
	btnclose->setGeometry(0, 0, 24, 24);
	btnclose->setMinimumSize(0, 0);
	btnclose->setPixmap(BarIcon("exit.xpm"));
	connect(btnclose, SIGNAL(clicked()), SLOT(closeDlg()));
	QToolTip::add(btnclose, i18n("Close Dialog"));

	btnscan = new QPushButton(widget_1);
	btnscan->setGeometry(30, 0, 24, 24);
	btnscan->setMinimumSize(0, 0);
	btnscan->setPixmap(BarIcon("scandir.xpm"));
	connect(btnscan, SIGNAL(clicked()), SLOT(scanDir()));
	QToolTip::add(btnscan, i18n("Scan Subdirectories"));

#ifdef HAVE_MIDI
	separator = new KSeparator(widget_1);
	separator->setGeometry(55, 0, 24, 24);
	separator->setMinimumSize(0, 0);
	separator->setOrientation(KSeparator::VLine);

	btnplay = new QPushButton(widget_1);
	btnplay->setGeometry(80, 0, 24, 24);
	btnplay->setMinimumSize(0, 0);
	btnplay->setPixmap(BarIcon("play.xpm"));
	connect(btnplay, SIGNAL(clicked()), SLOT(playSong()));
	QToolTip::add(btnplay, i18n("Play score"));
	btnplay->setToggleButton(TRUE);

	jumpcombo = new QComboBox(widget_1);
	jumpcombo->setGeometry(110, 0, 150, 24);
	jumpcombo->setMinimumSize(0, 0);
	jumpcombo->insertItem(i18n("No Jump"));
	jumpcombo->insertItem(i18n("Jump after 1 bar"));
	jumpcombo->insertItem(i18n("Jump after 2 bars"));
	jumpcombo->insertItem(i18n("Jump after 5 bars"));
	jumpcombo->insertItem(i18n("Jump after 10 bars"));
	jumpcombo->insertItem(i18n("Jump after 25 bars"));
	jumpcombo->insertItem(i18n("Jump after the score"));
#endif

	dirlist = new QListView(this);
	dirlist->setGeometry(10, 50, 230, 340);
	dirlist->setMinimumSize(0, 0);
	dirlist->addColumn(i18n("Directories"));
	dirlist->setFrameStyle(QFrame::Panel | QFrame::Sunken);
	connect(dirlist, SIGNAL(selectionChanged(QListViewItem*)), 
			this, SLOT(fillFileView(QListViewItem*)));
	connect(dirlist, SIGNAL(doubleClicked(QListViewItem*)),
			this, SLOT(fillFileView(QListViewItem*)));

	fileview = new QListView(this);
	fileview->setGeometry(250, 50, 380, 250);
	fileview->setMinimumSize(0, 0);
	fileview->addColumn(i18n("Name"));
	fileview->addColumn(i18n("Size"));
	fileview->addColumn(i18n("Modified"));
	fileview->addColumn(i18n("Directory"));
	fileview->setAllColumnsShowFocus(TRUE);
	fileview->setFrameStyle(QFrame::Panel | QFrame::Sunken);
	connect(fileview, SIGNAL(currentChanged(QListViewItem*)),
			this, SLOT(loadSong(QListViewItem*)));

	widget_2 = new QWidget(this);
	widget_2->setGeometry(250, 300, 380, 90);
	widget_2->setMinimumSize(0, 0);
	titlelabel = new QLabel(widget_2);
	titlelabel->setGeometry(10, 10, 100, 20);
	titlelabel->setMinimumSize(0, 0);
	titlelabel->setText(i18n("Title:"));

	authorlabel = new QLabel(widget_2);
	authorlabel->setGeometry(10, 40, 100, 20);
	authorlabel->setMinimumSize(0, 0);
	authorlabel->setText(i18n("Author:"));

	translabel = new QLabel(widget_2);
	translabel->setGeometry(10, 70, 100, 20);
	translabel->setMinimumSize(0, 0);
	translabel->setText(i18n("Transcriber:"));

	tlabel = new QLabel(widget_2);
	tlabel->setGeometry(120, 10, 260, 20);
	tlabel->setMinimumSize(0, 0);

	alabel = new QLabel(widget_2);
	alabel->setGeometry(120, 40, 260, 20);
	alabel->setMinimumSize(0, 0);

	tslabel = new QLabel(widget_2);
	tslabel->setGeometry(120, 70, 260, 20);
	tslabel->setMinimumSize(0, 0);

	messagelabel = new QLabel(this);
	messagelabel->setGeometry(10, 400, 260, 20);
	messagelabel->setMinimumSize(0, 0);

	//FILL dirlist
	directory = new Directory(dirlist);
	directory->setOpen(TRUE);
}

FileBrowser::~FileBrowser()
{

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
	QString fname, fsize, fmodified, fdir;

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
				QListViewItem* lv = new QListViewItem(fileview, fname, fsize, 
													  fmodified, fdir);
			}
		}
		else
			if (fi->isDir()) scanSubDirs(fi->absFilePath());
		++it;
	}
}

void FileBrowser::scanDir()
{
	QListViewItem* lv;
	QString msg;

	fileview->clear();
	messagelabel->setText(i18n("Reading subdirectories..."));

	lv = dirlist->currentItem();
	if (lv == 0){
		msg = i18n("Please select a directory!");
		messagelabel->setText(msg);
		KMessageBox::information(this, msg, i18n("File browser"));
		return;
	}

#ifdef HAVE_MIDI
	btnplay->setEnabled(FALSE);
#endif
	btnclose->setEnabled(FALSE);
	btnscan->setEnabled(FALSE);
	qApp->processEvents();

	QDir dir(getFullPath(lv));
	if (dir.isReadable())
		scanSubDirs(getFullPath(lv));
	else
		KMessageBox::sorry(this, i18n("You have no permission to read this directory!"), 
						   i18n("File browser"));

#ifdef HAVE_MIDI
	btnplay->setEnabled(TRUE);
#endif
	btnclose->setEnabled(TRUE);
	btnscan->setEnabled(TRUE);

	msg.setNum(fileview->childCount());
	msg += i18n(" file(s)");
	messagelabel->setText(msg);
}

void FileBrowser::playSong()
{

}

void FileBrowser::fillFileView(QListViewItem* item)
{
	QString fname, fsize, fmodified, fdir, msg;

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
				QListViewItem* lv = new QListViewItem(fileview, fname, fsize, 
													  fmodified, fdir);
			}
		}
		++it;
	}
	msg.setNum(fileview->childCount());
	msg += i18n(" file(s)");
	messagelabel->setText(msg);
}

void FileBrowser::loadSong(QListViewItem* item)
{
	QString fname, fpath;
	fname = item->text(0);
	fpath = item->text(3);
	fname = fpath + "/" + fname;

	if (p->tv->sng()->load_from_kg(fname)) {
		p->setCaption(fname);
		p->tv->setCurt(p->tv->sng()->t.first());
		p->tv->sng()->t.first()->x = 0;
		p->tv->sng()->t.first()->y = 0;
		p->tv->sng()->filename = fname;
		p->tv->updateRows();
		p->tv->repaint();
		tlabel->setText(p->tv->sng()->title);
		alabel->setText(p->tv->sng()->author);
		tslabel->setText(p->tv->sng()->transcriber);
		p->addRecentFile(fname);
	} else
		KMessageBox::error(this,  i18n("Can't load the song!"), i18n("File browser"));
}
