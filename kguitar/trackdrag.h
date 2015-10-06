#ifndef TRACKDRAG_H
#define TRACKDRAG_H

#include <QString>
#include <QByteArray>

class TabTrack;
class QMimeData;

class TrackDrag
{
public:

	static QByteArray encode(TabTrack *trk);
	static bool canDecode(const QMimeData *e);
	static bool decode(const QMimeData *e, TabTrack *&trk);

	static QString TRACK_MIME_TYPE;
};

#endif
