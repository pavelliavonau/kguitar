#include "chord.h"
#include "fingers.h"
#include "fingerlist.h"

#include <qpushbutton.h>
#include <qlistbox.h>
#include <qlineedit.h>
#include <qscrollbar.h>
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

    fng = new Fingering(6,this);
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
    int i,j,k,cn,min,max;
    int app[MAX_STRINGS];

    int maxfret=9,notenum=3;
    bool finished=FALSE,ok;

    // CALCULATION OF REQUIRED NOTES FOR A CHORD FROM USER INPUT

    int need[3]={0,4,7},got[3];

    for (i=0;i<3;i++)
      need[i]=(need[i]+tonic->currentItem())%12;

    // PREPARING FOR FINGERING CALCULATION

    for (i=0;i<MAX_STRINGS;i++)
      app[i]=0;
    fnglist->clear();

    // MAIN FINGERING CALCULATION LOOP

    do {
        i=0;
        do {
            app[i]++;
            if (app[i]<=maxfret)  {
                ok = FALSE;
                cn = (app[i]+tune[i]) % 12;
                for (k=0;k<notenum;k++) {
                    if (cn==need[k]) { ok = TRUE; break; }
                }
                if (ok) { break; }
            } else {
                app[i]=0;i++;
                if (i>5) {
                    finished=TRUE;
                    break;
                }
            }
        } while (TRUE);
        if (!finished) {
            min=maxfret+1;max=0;
            for (k=0;k<notenum;k++) { got[k]=0; }
            for (j=0;j<=5;j++) {
                if (app[j]!=0) {
                    if (app[j]<min)  min = app[j];
                    if (app[j]>max)  max = app[j];
                }
                cn = (app[j]+tune[j]) % 12;
                for (k=0;k<notenum;k++) {
                    if (cn==need[k])  { got[k]++; break; }
                }
            };
            ok=TRUE;
            for (k=0;k<notenum;k++) {
                if (got[k]==0) { ok = FALSE; break; }
            };
            if ((ok) && (max-min<3)) {
	      fnglist->addFingering(app,TRUE);
	      printf("%d%d%d%d%d%d\n",app[5],app[4],app[3],app[2],app[1],app[0]);
            }
        }
    } while (!finished);

    fnglist->repaint();
}



