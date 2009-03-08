#ifndef TRACKDRAG_H
#define TRACKDRAG_H

#include <qdragobject.h>

class TabTrack;

class TrackDrag: public QStoredDrag {
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
