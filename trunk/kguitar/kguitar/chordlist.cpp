#include "chordlist.h"

ChordList::ChordList(QWidget *parent=0, const char *name=0):
    QListBox(parent, name)
{
}

ChordListItem* ChordList::currentItemPointer()
{
    return (ChordListItem*) item(currentItem());
}
