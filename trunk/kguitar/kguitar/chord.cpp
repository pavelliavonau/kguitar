#include "chord.h"
#include "fingers.h"
#include "fingerlist.h"
#include "track.h"

#include <kapp.h>

#include <qpushbutton.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qcombobox.h>
#include <qlistbox.h>
#include <qlineedit.h>
#include <qstring.h>
#include <qlabel.h>

// Note names
QString note_name(int num)
{
    switch (num) {
        case 0:  return "C"; break;
        case 1:  return "C#";break;
        case 2:  return "D"; break;
        case 3:  return "D#";break;
        case 4:  return "E"; break;
        case 5:  return "F"; break;
        case 6:  return "F#";break;
        case 7:  return "G"; break;
        case 8:  return "G#";break;
        case 9:  return "A"; break;
        case 10: return "A#";break;
        case 11: return "B"; break;
    }
    return "Unknown";
}

//                     3  5  7  9  11 13
int stemplate[9][6]={ {-1,2, 0, 0, 0, 0 },
		      {-1,2, 2, 0, 0, 0 },
		      {-1,2, 3, 0, 0, 0 },
		      {-1,2, 1, 0, 0, 0 },
		      {3, 3, 0, 0, 0, 0 },
		      {2, 1, 1, 0, 0, 0 },
		      {-1,2, 2, 2, 0, 0 },
		      {-1,2, 2, 2, 2, 0 },
                      {0, 2, 0, 0, 0, 0 } };

ChordSelector::ChordSelector(TabTrack *p, QWidget *parent=0, const char *name=0)
    :QDialog(parent,name,TRUE)
{
    parm = p;

    chname = new QLineEdit(this);
    chname->setGeometry(10,10,210,20);

    // CHORD SELECTOR FOR FINDER WIDGETS

    tonic = new QListBox(this);
    for(int i=0;i<12;i++)
      tonic->insertItem(note_name(i));
    tonic->setGeometry(10,40,50,200);
    connect(tonic,SIGNAL(highlighted(int)),SLOT(findChords()));

    step3 = new QListBox(this);
    step3->insertItem("M");
    step3->insertItem("m");
    step3->insertItem("sus2");
    step3->insertItem("sus4");
    step3->setGeometry(70,40,80,70);
    connect(step3,SIGNAL(highlighted(int)),SLOT(setStep3()));

    stephigh = new QListBox(this);
    stephigh->insertItem("");
    stephigh->insertItem("7");
    stephigh->insertItem("7M");
    stephigh->insertItem("6");
    stephigh->insertItem("aug");
    stephigh->insertItem("dim");
    stephigh->insertItem("9");
    stephigh->insertItem("11");
    stephigh->insertItem("5");
    stephigh->setGeometry(160,40,60,200);
    connect(stephigh,SIGNAL(highlighted(int)),SLOT(setHighSteps()));

    // st array holds values for each step:
    // st[0] - 3'    st[1] - 5'    st[2] - 7'
    // st[3] - 9'    st[4] - 11'   st[5] - 13'

    QLabel *stlabel[7];
    QString tmp;
    for (int i=0;i<7;i++) {
	tmp.setNum(i*2+1);
	tmp=tmp+"\'";
	stlabel[i] = new QLabel(tmp,this);
	stlabel[i]->setGeometry(230+i*STEPSIZE,170,STEPSIZE,20);
	stlabel[i]->setAlignment(AlignCenter);

	cnote[i] = new QLabel(this);
	cnote[i]->setGeometry(230+i*STEPSIZE,210,STEPSIZE,20);
	cnote[i]->setAlignment(AlignCenter);
	
	if (i>0) {
	    st[i-1] = new QComboBox(FALSE,this);
	    st[i-1]->setGeometry(230+i*STEPSIZE,190,STEPSIZE,20);
	    st[i-1]->insertItem("x");
	    if ((i==2) || (i>=4)) {
		st[i-1]->insertItem("-");
		st[i-1]->insertItem("0");
		st[i-1]->insertItem("+");
	    }
	    connect(st[i-1],SIGNAL(activated(int)),SLOT(findSelection()));
	    connect(st[i-1],SIGNAL(activated(int)),SLOT(findChords()));
	}
    }

    st[0]->insertItem("2");
    st[0]->insertItem("-");
    st[0]->insertItem("+");
    st[0]->insertItem("4");

    st[2]->insertItem("6");
    st[2]->insertItem("-");
    st[2]->insertItem("+");

    complexity = new QButtonGroup(this);
    complexity->setGeometry(70,150,80,70);
    complexer[0] = new QRadioButton(i18n("Usual"),complexity);
    complexer[0]->setGeometry(5,5,70,20);
    complexer[1] = new QRadioButton(i18n("Rare"),complexity);
    complexer[1]->setGeometry(5,25,70,20);
    complexer[2] = new QRadioButton(i18n("All"),complexity);
    complexer[2]->setGeometry(5,45,70,20);
    complexity->setButton(0);
    connect(complexity,SIGNAL(clicked(int)),SLOT(findChords()));

    // CHORD ANALYZER

    fng = new Fingering(p,this);
    fng->move(230,10);
    connect(fng,SIGNAL(chordChange()),SLOT(detectChord()));

    chords = new QListBox(this);
    chords->setGeometry(400,10,100,150);

    // CHORD FINDER OUTPUT

    fnglist = new FingerList(p,this);
    fnglist->setGeometry(10,250,500,140);
    connect(fnglist,SIGNAL(chordSelected(const int *)),fng,SLOT(setFingering(const int *)));
    
    // DIALOG BUTTONS
    
    QPushButton *ok, *cancel;

    ok = new QPushButton(i18n("OK"),this);
    ok->setGeometry(520,250,75,30);
    connect(ok,SIGNAL(clicked()),SLOT(accept()));

    cancel = new QPushButton(i18n("Cancel"),this);
    cancel->setGeometry(520,290,75,30);
    connect(cancel,SIGNAL(clicked()),SLOT(reject()));

    setFixedSize(600,400);
}

int ChordSelector::app(int l)
{
    return fng->app(l);
}

// Try to detect some chord forms from a given applicature.
void ChordSelector::detectChord()
{
    bool cn[12];
    int i,j,numnotes,noteok;
    QString name;
    int s3,s5,s7,s9,s11,s13;

    for (i=0;i<12;i++)
	cn[i]=FALSE;
    numnotes=0; // number of different notes in a chord
    
    for (i=0;i<parm->string;i++) {
	j=fng->app(i);
	if (j!=-1) {
	    j=(j+parm->tune[i])%12;
	    if (!cn[j]) {
		cn[j]=TRUE;
		numnotes++;
	    }
	}
    }
    
    chords->clear();
    
    for (i=0;i<12;i++)  if (cn[i]) {

	// Initializing
	s3=-1;s5=-1;s7=-1;s9=-1;s11=-1;s13=-1;noteok=numnotes-1;

	// Detecting thirds
	if (cn[(i+4)%12]) {
	    s3=4;noteok--;               // Major
	} else if (cn[(i+3)%12]) {
	    s3=3;noteok--;               // Minor
	} else if (cn[(i+5)%12]) {
	    s3=5;noteok--;               // Sus4
	} else if (cn[(i+2)%12]) {
	    s3=2;noteok--;               // Sus2
	}

	// Detecting fifths
	if (cn[(i+7)%12]) {
	    s5=7;noteok--;               // 5
	} else if (cn[(i+6)%12]) {
	    s5=6;noteok--;               // 5-
	} else if (cn[(i+8)%12]) {
	    s5=8;noteok--;               // 5+
	}

	// Detecting sevenths
	if (cn[(i+10)%12]) {
	    s7=10;noteok--;              // 7
	} else if (cn[(i+11)%12]) {
	    s7=11;noteok--;              // 7M
	} else if (cn[(i+9)%12]) {
	    s7=9;noteok--;               // 6
	}

	// Detecting 9ths
	if ((cn[(i+2)%12]) && (s3!=2)) {
	    s9=2;noteok--;               // 9
	} else if ((cn[(i+3)%12]) && (s3!=3)) {
	    s9=3;noteok--;               // 9+
	} else if (cn[(i+1)%12]) {
	    s9=1;noteok--;               // 9-
	}

	// Detecting 11ths
	if ((cn[(i+5)%12]) && (s3!=5)) {
	    s11=5;noteok--;               // 11
	} else if ((cn[(i+4)%12]) && (s3!=4)) {
	    s11=4;noteok--;               // 11-
	} else if ((cn[(i+6)%12]) && (s5!=6)) {
	    s11=6;noteok--;               // 11+
	}

	// Detecting 13ths
	if ((cn[(i+9)%12]) && (s7!=9)) {
	    s13=9;noteok--;
	} else if ((cn[(i+8)%12]) && (s5!=8)) {
	    s13=8;noteok--;
	} else if ((cn[(i+10)%12]) && (s7!=10)) {
	    s13=10;noteok--;
	}

	if (noteok==0) {
	    name=note_name(i);

	    // Special cases
	    if ((s3==-1) && (s5==7) && (s7==-1) &&
		(s9==-1) && (s11==-1) && (s13==-1)) {
		chords->insertItem(name+"5");
		continue;
	    }
	    if ((s3==4) && (s5==8) && (s7==-1) &&
		(s9==-1) && (s11==-1) && (s13==-1)) {
		chords->insertItem(name+"aug");
		continue;
	    }

	    if ((s3==3) && (s5==6) && (s7==9)) {
		name=name+"dim";
	    } else {
		if (s3==3)
		    name=name+"m";
		
		if (s5==6)
		    name=name+"/5-";
		if (s5==8)
		    name=name+"/5+";
		if (((s5==6) || (s5==8)) && ((s7!=-1) || (s9!=-1) ||
					     (s11!=-1) || (s13!=-1)))
		    name=name+"/";
		
		if ((s7==10) && (s9==-1))
		    name=name+"7";
		if (s7==11)
		    name=name+"7M";
		if (s7==9)
		    name=name+"6";
		if (((s7==11) || (s7==9)) && ((s9!=-1) || (s11!=-1) || (s13!=-1)))
		    name=name+"/";
	    }

	    if ((s7==-1)  && (s9!=-1))
		name=name+"add";
	    if ((s9==2) && (s11==-1))
		name=name+"9";
	    if (s9==1)
		name=name+"9-";
	    if (s9==3)
		name=name+"9+";
	    if (((s9==1) || (s9==3)) && ((s11!=-1) || (s13!=-1)))
		name=name+"/";

	    if ((s9==-1) && (s11!=-1))
		name=name+"add";
	    if ((s11==5) && (s13==-1))
		name=name+"11";
	    if (s11==6)
		name=name+"11+";
	    if (s11==4)
		name=name+"11-";
	    if (((s11==4) || (s11==6)) && (s13!=-1))
		name=name+"/";

	    if ((s11==-1) && (s13!=-1))
		name=name+"add";
	    if (s13==9)
		name=name+"13";
	    if (s13==10)
		name=name+"13+";
	    if (s13==8)
		name=name+"13-";

	    if (s3==2)
		name=name+"sus2";
	    if (s3==5)
		name=name+"sus4";

	    if ((s3==-1) && (s5==-1)) {
		name=name+" (no3no5)";
	    } else {
		if (s3==-1)
		    name=name+" (no3)";
		if (s5==-1)
		    name=name+" (no5)";
	    }
	    chords->insertItem(name);
	}
    }

/*
    // 2 note chord - C5
    if (numnotes==2) {
	for (i=0;i<12;i++) {
	    if ((cn[i]) && (cn[(i+7)%12]))
		chords->insertItem(note_name(i)+"5");
	}
    }
    
    // 3 note chord - C, Cm, Csus2, Csus4, Caug
    if (numnotes==3) {
	for (i=0;i<12;i++)
	    if ((cn[i]) && (cn[(i+4)%12]) && (cn[(i+7)%12]))
		chords->insertItem(note_name(i));
	if ((cn[i]) && (cn[(i+3)%12]) && (cn[(i+7)%12]))
	    chords->insertItem(note_name(i)+"m");
	if ((cn[i]) && (cn[(i+2)%12]) && (cn[(i+7)%12]))
	    chords->insertItem(note_name(i)+"sus2");
	if ((cn[i]) && (cn[(i+5)%12]) && (cn[(i+7)%12]))
	    chords->insertItem(note_name(i)+"sus4");
	if ((cn[i]) && (cn[(i+4)%12]) && (cn[(i+8)%12]))
	    chords->insertItem(note_name(i)+"aug");
    }
    
    // 4 note chord - C7, C7M, C6, Cm7, Cm7M, C7sus2, C7sus4, Cdim
    if (numnotes==4) {
	for (i=0;i<12;i++) {
	    if ((cn[i]) && (cn[(i+4)%12]) && (cn[(i+7)%12]) && (cn[(i+9)%12]))
		chords->insertItem(note_name(i)+"6");
	    if ((cn[i]) && (cn[(i+4)%12]) && (cn[(i+7)%12]) && (cn[(i+10)%12]))
		chords->insertItem(note_name(i)+"7");
	    if ((cn[i]) && (cn[(i+4)%12]) && (cn[(i+7)%12]) && (cn[(i+11)%12]))
		chords->insertItem(note_name(i)+"7M");
	    if ((cn[i]) && (cn[(i+3)%12]) && (cn[(i+7)%12]) && (cn[(i+9)%12]))
		chords->insertItem(note_name(i)+"m6");
	    if ((cn[i]) && (cn[(i+3)%12]) && (cn[(i+7)%12]) && (cn[(i+10)%12]))
		chords->insertItem(note_name(i)+"m7");
	    if ((cn[i]) && (cn[(i+3)%12]) && (cn[(i+7)%12]) && (cn[(i+11)%12]))
		chords->insertItem(note_name(i)+"m7M");
	    if ((cn[i]) && (cn[(i+2)%12]) && (cn[(i+7)%12]) && (cn[(i+10)%12]))
		chords->insertItem(note_name(i)+"7sus2");
	    if ((cn[i]) && (cn[(i+5)%12]) && (cn[(i+7)%12]) && (cn[(i+10)%12]))
		chords->insertItem(note_name(i)+"7sus4");
	    if ((cn[i]) && (cn[(i+3)%12]) && (cn[(i+6)%12]) && (cn[(i+9)%12]))
		chords->insertItem(note_name(i)+"dim");
	}
    }
    
    // 5 note chord
    if (numnotes==5) {
	for (i=0;i<12;i++) {
	    if ((cn[i]) && (cn[(i+4)%12]) && (cn[(i+7)%12]) && (cn[(i+10)%12]) && (cn[(i+2)]%12)) {
		chords->insertItem(note_name(i)+"9");
	    }
	}
    }
*/
}

void ChordSelector::setStep3()
{
    switch (step3->currentItem()) {
    case 0: st[0]->setCurrentItem(3);break;                // Major
    case 1: st[0]->setCurrentItem(2);break;                // Minor
    case 2: st[0]->setCurrentItem(1);break;                // Sus2
    case 3: st[0]->setCurrentItem(4);break;                // Sus4
    }

    findSelection();
    findChords();
}

void ChordSelector::setHighSteps()
{
    int j = stephigh->currentItem();

    if (j==-1)
	return;

    for (int i=0;i<6;i++)
	if (stemplate[j][i]!=-1)
	    st[i]->setCurrentItem(stemplate[j][i]);

    findSelection();
    findChords();
}

void ChordSelector::findSelection()
{
    bool ok;

    switch (st[0]->currentItem()) {
    case 0: step3->clearSelection();break;                 // no3
    case 1: step3->setCurrentItem(2);break;                // Sus2
    case 2: step3->setCurrentItem(1);break;                // Minor
    case 3: step3->setCurrentItem(0);break;                // Major
    case 4: step3->setCurrentItem(3);break;                // Sus4
    }

    for (int j=0;j<stephigh->count();j++) {
	ok = TRUE;
	for (int i=0;i<6;i++) {
	    if ((stemplate[j][i]!=-1) && (stemplate[j][i]!=st[i]->currentItem())) {
		ok = FALSE;
		break;
	    }
	}
	if (ok) {
	    stephigh->setCurrentItem(j);
	    break;
	}
    }
    if (!ok)
	stephigh->clearSelection();
}

void ChordSelector::findChords()
{
    int i,j,k,min,max,bass,muted;
    int app[MAX_STRINGS];
    int toneshift[6]={0,7,10,2,5,9};

    int fb[MAX_STRINGS][parm->frets];  // array with an either -1 or number of note from a chord

    // CALCULATION OF REQUIRED NOTES FOR A CHORD FROM USER STEP INPUT

    int need[6],got[6];

    int t = tonic->currentItem();

    int notenum=1;
    need[0]=t;
    cnote[0]->setText(note_name(t));

    switch (st[0]->currentItem()) {
    case 1: need[1]=(t+2)%12;notenum++;break;     // 2
    case 2: need[1]=(t+3)%12;notenum++;break;     // 3-
    case 3: need[1]=(t+4)%12;notenum++;break;     // 3+
    case 4: need[1]=(t+5)%12;notenum++;break;     // 4
    }

    if (st[0]->currentItem()!=0) {
	cnote[1]->setText(note_name(need[1]));
    } else {
	cnote[1]->clear();
    }

    for (i=1;i<6;i++) {
	j=st[i]->currentItem();
	if (j) {
	    need[notenum]=(t+toneshift[i]+(j-2))%12;
	    cnote[i+1]->setText(note_name(need[notenum]));
	    notenum++;
	} else {
	    cnote[i+1]->clear();
	}
    }

    int span=3; // maximal fingerspan
    
    if (complexer[1]->isChecked())
	span=4;
    if (complexer[2]->isChecked())
	span=5;    
    
    // PREPARING FOR FINGERING CALCULATION
    
    for (i=0;i<parm->string;i++) {
	for (j=0;j<=parm->frets;j++)
	    fb[i][j]=-1;
	for (k=0;k<notenum;k++) {
	    j=(need[k]-parm->tune[i]%12+12)%12;
	    while (j<=parm->frets) {
		fb[i][j]=k;
		j+=12;
	    }
	}
    }
    
    for (i=0;i<parm->string;i++)
	 app[i]=-1;
    
    fnglist->switchAuto(FALSE);
    fnglist->clear();
    
    // MAIN FINGERING CALCULATION LOOP

    i=0;
    do {
	if (app[i]<=parm->frets) {
	    min=parm->frets+1;max=0;
	    for (k=0;k<notenum;k++)
		got[k]=0;
	    k=0;bass=255;muted=0;
	    for (j=0;j<parm->string;j++) {
		if (app[j]>0) {
		    if (app[j]<min)  min=app[j];
		    if (app[j]>max)  max=app[j];
		}
		if (max-min>=span)
		    break;
		if (app[j]>=0) {
		    if (parm->tune[j]+app[j]<bass)
			bass=parm->tune[j]+app[j];
		    if (!got[fb[j][app[j]]]) {
			got[fb[j][app[j]]]=1;
			k++;
		    }
		} else {
		    muted++;
		}
	    }
	    
	    if ((k==notenum) && (max-min<span) && (bass % 12 == need[0])) {
		if (complexer[0]->isChecked()) {
		    if ((muted==0) ||                                       // No muted strings
			((muted==1) && (app[0]==-1)) ||                     // Last string muted
			((muted==2) && (app[0]==-1) && (app[1]==-1))) {     // Last and pre-last muted
			fnglist->addFingering(app,TRUE);
		    }
		} else {
		    fnglist->addFingering(app,TRUE);
		}
	    }
	    
	    i=0;
	} else {
	    app[i]=-1;i++;
	    if (i>=parm->string)
		break;
	}
	app[i]++;
	while ((fb[i][app[i]]==-1) && (app[i]<=parm->frets))
	    app[i]++;
    } while (TRUE);
    
    fnglist->switchAuto(TRUE);
    fnglist->repaint();
}



