
#include <qlist.h>

#include "midilist.h"
#include "tabtrack.h"


void MidiData::getMidiList(TabTrack *&trk, MidiList &ml)
{
    long timer = 0;
    Q_UINT8 noteon = MIDI_NOTEON | (trk->channel - 1);
    int midilen = 0, duration;
    uchar pitch;

    for (uint x = 0; x < trk->c.size(); x++) {
        // Calculate real duration (including all the linked beats)
        midilen = dot2len(trk->c[x].l, trk->c[x].flags & FLAG_DOT);
        while ((x + 1 < trk->c.size()) && (trk->c[x + 1].flags & FLAG_ARC)) {
            x++;
            midilen += dot2len(trk->c[x].l, trk->c[x].flags & FLAG_DOT);
        }

        // Note on/off events
        for (int i = 0; i < trk->string; i++) {
            if (trk->c[x].a[i] == -1)  continue;

            if (trk->c[x].a[i] == DEAD_NOTE) {
                pitch = trk->tune[i];
                duration = 5;
            } else {
                pitch = trk->c[x].a[i] + trk->tune[i];
                duration = midilen;
            }

            if (trk->c[x].flags & FLAG_PM)
                duration = duration / 2;

            if (trk->c[x].e[i] == EFFECT_ARTHARM)
                pitch += 12;
            if (trk->c[x].e[i] == EFFECT_HARMONIC) {
                switch (trk->c[x].a[i]) {
					case 3: pitch += 28; break;
					case 4: pitch += 24; break;
					case 5: pitch += 19; break;
					case 7: pitch += 12; break;
					case 9: pitch += 19; break;
					case 16: pitch += 12; break; // GREYFIX: is it true?
                }
            }

            ml.inSort(new MidiEvent(timer, noteon, pitch, 0x60));
            ml.inSort(new MidiEvent(timer + duration, noteon, pitch, 0));
        }
        timer += midilen;
    }
}

int MidiList::compareItems(QCollection::Item e1, QCollection::Item e2)
{
    int i = ((MidiEvent *) e1)->timestamp - ((MidiEvent *) e2)->timestamp;

    // if we play one note twice, send first a noteOff event
    if (i == 0)
        return ((MidiEvent *) e1)->data2 - ((MidiEvent *) e2)->data2;
    else
        return i;
}
