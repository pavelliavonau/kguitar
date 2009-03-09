#ifndef TRACKDRAG_H
#define TRACKDRAG_H

#include <q3dragobject.h>

class TabTrack;

class TrackDrag: public Q3StoredDrag {
	Q_OBJECT
public:
	TrackDrag(TabTrack *trk, QWidget *dragSource = 0, const char *name = 0);
	TrackDrag(QWidget *dragSource = 0, const char *name = 0);
	~TrackDrag();

	virtual void setTrack(TabTrack *trk);

	static bool canDecode(const QMimeSource *e);
	static bool decode(const QMimeSource *e, TabTrack *&trk);
};

#endif
