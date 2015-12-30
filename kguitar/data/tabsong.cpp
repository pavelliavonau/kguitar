#include "global.h"
#include "tabsong.h"
#include "settings.h"

#include <qxml.h>
#include <qfile.h>
#include <qdatastream.h>
#include <kconfig.h>
#include <klocalizedstring.h>

#ifdef WITH_TSE3
#include <tse3/Track.h>
#include <tse3/Part.h>
#include <tse3/TempoTrack.h>
#include <string>
#endif

TabSong::TabSong(QString _title, int _tempo)
{
	tempo = _tempo;
	info["TITLE"] = _title;
}

int TabSong::freeChannel()
{
	bool fc[17];
	for (int i = 1; i <= 16; i++)
		fc[i] = TRUE;

	for (int i = 0; i < t.size(); i++)
		fc[t[i]->channel] = false;

	int res;
	for (res = 1; res <= 16; res++)
		if (fc[res])
			break;

	if (res > 16)
		res = 1;

	return res;
}

uint TabSong::maxLen() const
{
	uint res = 0;

	for (int i = 0; i < t.size(); i++)
		if( t.at(i) )
			res = t.at(i)->b.size() > res ? t.at(i)->b.size() : res;
		else
			res = 1 > res ? 1 : res;

	return res;
}

void TabSong::arrangeBars()
{
	// For every track
	foreach(TabTrack *trk, t)
		trk->arrangeBars();
}

#ifdef WITH_TSE3
// Assembles the whole TSE song from various tracks, generated with
// corresponding midiTrack() calls.
TSE3::Song *TabSong::midiSong(bool tracking)
{
	TSE3::Song *song = new TSE3::Song(0);

	// Create tempo track
	TSE3::Event<TSE3::Tempo> tempoEvent(tempo, TSE3::Clock(0));
	song->tempoTrack()->insert(tempoEvent);

	// Create data tracks
	int tn = 0;
	foreach (TabTrack *ttrk, t) {
		TSE3::PhraseEdit *trackData = ttrk->midiTrack(tracking, tn);
		TSE3::Phrase *phrase = trackData->createPhrase(song->phraseList());
		TSE3::Part *part = new TSE3::Part(0, trackData->lastClock() + 1); // GREYFIX: this may be why last event got clipped?
		part->setPhrase(phrase);
		TSE3::Track *trk = new TSE3::Track();
		trk->insert(part);
		song->insert(trk);
		delete trackData;
		tn++;
	}

	return song;
}
#endif

void TabSong::addEmptyTrack()
{
	TabTrack *trk = new TabTrack(TabTrack::FretTab, i18n("Guitar"), 1, 0, 25, 6, 24);
	t.append(trk);
}


int TabSong::rowCount(const QModelIndex &parent) const
{
	if(parent.isValid())
		return 0;

	return t.size();
}

int TabSong::columnCount(const QModelIndex &parent) const
{
	if(parent.isValid())
		return 0;

	return maxLen();
}

QVariant TabSong::data(const QModelIndex &index, int role) const
{
	if(!index.isValid())
		return QVariant();

	int row = index.row();   // track number
	int col = index.column();// bar number

	if( role == BarRole ) {
		return QVariant::fromValue( t.at( row )->b.at( col ) );
	}

	if( role == TrackPtrRole ) {
		return QVariant::fromValue( t.at( row ) );
	}

	if( role == Qt::ToolTipRole)
		return QVariant("srow = " + QString::number(index.row()) + " scol = " + QString::number(index.column()));

	return QVariant();
}


bool TabSong::insertColumns(int column, int count, const QModelIndex &parent)
{
	beginInsertColumns(parent, column, column + count - 1);
	for(int row = 0; row < rowCount(); ++row)
		for(int col= 0; col < count; ++col)
			t.at(row)->b.insert(column, TabBar());    // TODO: get current bar settings
	endInsertColumns();
	return true;
}

bool TabSong::insertRows(int row, int count, const QModelIndex &parent)
{
	beginInsertRows(parent, row, row + count - 1);
	for(int i = 0; i < count; ++i)
		t.insert(row, nullptr);
	endInsertRows();
	return true;
}

bool TabSong::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if(!index.isValid())
		return false;

	switch(role) {
		case TrackPtrRole: {
			TabTrack* track = value.value<TabTrack*>();
			normalizeBarCount(track);
			t.replace(index.row(), track);
			break;
		}
		case BarRole:  {
			TabBar newBar = value.value<TabBar>();

			bool isAdditionalBar = !t.at(index.row())->b.at(index.column()).isValid();

			if( !isAdditionalBar )
				t.at(index.row())->b.replace(index.column(), newBar);
			else {
				// UGLY HACK. needs change of architecture
				for( int i = 0; i< t.size(); ++i ) {
					if(index.row() != i && index.column() > 0) {
						TabBar otherTrackBar(newBar);
						otherTrackBar.start = t.at(i)->c.size();
						t.at(i)->b.replace(index.column(), otherTrackBar);
						TabColumn column;
						column.l = t.at(i)->barDuration(index.column() - 1);
						t.at(i)->c.append(column);
					}
					else
						t.at(i)->b.replace(index.column(), newBar);
				}
			}
			arrangeBars();  //may be expensive
			break;
		}
	}

	emit dataChanged(index, index);
	return true;
}

bool TabSong::removeRows(int row, int count, const QModelIndex &parent)
{
	beginRemoveRows(parent, row, row + count - 1);
	for (int i = 0; i < count; ++i)
		delete t.takeAt(row);
	endRemoveRows();
	return true;
}

bool TabSong::removeColumns(int column, int count, const QModelIndex &parent)
{
	beginRemoveColumns(parent, column, column + count - 1);
	for(int row = 0; row < rowCount(); ++row)
		for(int col= 0; col < count; ++col)
			t.at(row)->b.remove(column);
	endRemoveColumns();

	return true;
}

void TabSong::normalizeBarCount(TabTrack *track_) const
{
	if(!t.at(0))
		return;

	// this code far from ideal and creates new bugs. Must be rewrited in future.

	// for new track every bar contain only 1 column
	track_->c.resize(t.at(0)->b.size());

	for( int i = 0; i < t.at(0)->b.size(); ++i ) {

		const TabBar& songBar = t.at(0)->b.at(i);
		TabBar trackBar(i, songBar.time1, songBar.time2);

		if(i >= track_->bars().size())
			track_->b.append(trackBar);
		//Tab
		int songbardur = t.at(0)->barDuration(i);
		int newbardur = track_->barDuration(i);
		if(newbardur < songbardur)
			track_->c[i].l = songbardur;
	}

	track_->arrangeBars();
}
