#include "chord.h"
#include "fingers.h"
#include "fingerlist.h"

#include <qpushbutton.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qlistbox.h>
#include <qlineedit.h>
#include <qstring.h>

// GREYFIX
#include <stdio.h>

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

ChordSelector::ChordSelector(QWidget *parent=0, const char *name=0)
    :QDialog(parent,name,TRUE)
{
    QPushButton *ok, *cancel;

    ok = new QPushButton("Ok",this);
    ok->setGeometry(10,250,75,30);
    connect(ok,SIGNAL(clicked()),SLOT(accept()));

    cancel = new QPushButton("Cancel",this);
    cancel->setGeometry(100,250,75,30);
    connect(cancel,SIGNAL(clicked()),SLOT(reject()));

    chords = new QListBox(this);
    chords->setGeometry(350,10,80,150);

    chname = new QLineEdit(this);
    chname->setGeometry(10,10,130,20);

    fng = new Fingering(6,this); // GREYFIX hack 6 strings
    fng->setGeometry(150,10,100,100);
    connect(fng,SIGNAL(chordChange()),SLOT(detectChord()));

    fnglist = new FingerList(this);
    fnglist->setGeometry(10,240,420,100);
    connect(fnglist,SIGNAL(chordSelected(const int *)),fng,SLOT(setFingering(const int *)));

    tonic = new QListBox(this);
    tonic->setAutoUpdate(FALSE);
    for(int i=0;i<12;i++)
      tonic->insertItem(note_name(i));
    tonic->setAutoUpdate(TRUE);
    tonic->setGeometry(10,40,40,12*tonic->itemHeight()+6);  // GREYFIX
    connect(tonic,SIGNAL(highlighted(int)),SLOT(findChords()));

    step3 = new QListBox(this);
    step3->insertItem("M");
    step3->insertItem("m");
    step3->insertItem("sus2");
    step3->insertItem("sus4");
    step3->setGeometry(60,40,50,100);

    complexity = new QButtonGroup(this);
    complexity->setGeometry(60,150,90,70);
    complexer[0] = new QRadioButton("Usual",complexity);
    complexer[0]->setGeometry(5,5,80,20);
    complexer[1] = new QRadioButton("Rare",complexity);
    complexer[1]->setGeometry(5,25,80,20);
    complexer[2] = new QRadioButton("All",complexity);
    complexer[2]->setGeometry(5,45,80,20);
    complexity->setButton(0);

    setFixedSize(450,350);
}

// Standard tuning
//int tune[6] = {64,59,55,50,45,40};
int tune[6]={40,45,50,55,59,64};

// Try to detect some chord forms from a given applicature.
void ChordSelector::detectChord()
{
  bool cn[12];
  int i,j,numnotes;

  for (i=0;i<12;i++)
    cn[i]=FALSE;
  numnotes=0; // number of different notes in a chord

  for (i=0;i<fng->numstrings();i++) {
    j=fng->app(i);
    if (j!=-1) {
      j=(j+tune[i])%12;
      if (!cn[j]) {
	cn[j]=TRUE;
	numnotes++;
      }
    }
  }

  chords->clear();

  // 2 note chord - C5
  if (numnotes==2) {
    for (i=0;i<12;i++) {
      if ((cn[i]) && (cn[(i+7)%12]))
	chords->insertItem(note_name(i)+"5");
    }
  }

  // 3 note chord - C, Cm, Csus2, Csus4, Caug
  if (numnotes==3) {
    for (i=0;i<12;i++) {
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
}

void ChordSelector::findChords()
{
    int i,j,k,min,max;
    int app[MAX_STRINGS];

    int maxfret=24,notenum=3,numstr=6; // GREYFIX!!

    int fb[MAX_STRINGS][maxfret];  // array with an either -1 or number of note from a chord

    // CALCULATION OF REQUIRED NOTES FOR A CHORD FROM USER INPUT

    int need[3]={0,4,7},got[3];

    for (i=0;i<notenum;i++)
	 need[i]=(need[i]+tonic->currentItem())%12;
    
    int span=3; // maximal fingerspan
    
    if (complexer[1]->isChecked())
	 span=4;
    if (complexer[2]->isChecked())
	 span=5;    
    
    // PREPARING FOR FINGERING CALCULATION
    
    for (i=0;i<numstr;i++) {
	 for (j=0;j<=maxfret;j++) {
	      fb[i][j]=-1;
	 }
	 for (k=0;k<notenum;k++) {
	      j=(need[k]-tune[i]%12+12)%12;
	      while (j<=maxfret) {
		   fb[i][j]=k;
		   j+=12;
	      }
	 }
    }
    
    for (i=0;i<numstr;i++)
	 app[i]=-1;
    
    fnglist->switchAuto(FALSE);
    fnglist->clear();
    
    // MAIN FINGERING CALCULATION LOOP

    i=0;
    do {
	 if (app[i]<=maxfret) {
	      min=maxfret+1;max=0;
	      for (k=0;k<notenum;k++)
		   got[k]=0;
	      k=0;
	      for (j=0;j<numstr;j++) {
		   if (app[j]>0) {
			if (app[j]<min)  min=app[j];
			if (app[j]>max)  max=app[j];
		   }
		   if (max-min>=span)
			break;
		   if (app[j]>=0) {
			if (!got[fb[j][app[j]]]) {
			     got[fb[j][app[j]]]=1;
			     k++;
			}
		   }
	      }
	      
	      if ((k==notenum) && (max-min<span)) {
		   k=255; // finding lowest note in a chord
		   for (j=0;j<numstr;j++) {
			if ((app[j]!=-1) && (tune[j]+app[j]<k))
			     k=tune[j]+app[j];
		   }
		   if (k % 12 == need[0])
			fnglist->addFingering(app,TRUE);
	      }
	      
	      i=0;
	 } else {
	      app[i]=-1;i++;
	      if (i>=numstr)
		   break;
	 }
	 app[i]++;
	 while ((fb[i][app[i]]==-1) && (app[i]<=maxfret))
	      app[i]++;
    } while (TRUE);
    
    fnglist->switchAuto(TRUE);
    fnglist->repaint();
}



