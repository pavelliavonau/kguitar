#include "chordlist.h"

ChordList::ChordList(QWidget *parent, const char *name)
	: QListBox(parent, name)
{
}

void ChordList::inSort(ChordListItem *it)
{
    uint l = ((QString) it->text()).length();
    uint best = 0;

    for (uint i = 0; i < count(); i++) {
		if (((QString) item(i)->text()).length() < l)
			best++;
		else
			break;
    }

    insertItem(it, best);
}

ChordListItem* ChordList::currentItemPointer()
{
    return (ChordListItem*) item(currentItem());
}
