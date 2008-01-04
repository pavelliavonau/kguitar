#ifndef CONVERT_XML_H
#define CONVERT_XML_H

#include "convertbase.h"
#include <qxml.h>
#include <qptrvector.h>
#include "accidentals.h"

class TabSong;
class QTextStream;

/**
 * Converter to/from MusicXML tabulature format.
 *
 * Manages conversions of tabulatures between internal KGuitar
 * structures and MusicXML tabulature formats.
 */
class ConvertXml: public ConvertBase, QXmlDefaultHandler {
public:
	/**
	 * Prepares converter to work, reads all the options, etc.
	 */
	ConvertXml(TabSong *);

	/**
	 * Called to save current data from TabSong into XML tabulature
	 * format file named fileName.
	 */
	virtual bool save(QString fileName);

	/**
	 * Called to load data from XML tabulature format file named
	 * fileName into TabSong.
	 */
	virtual bool load(QString fileName);

	void write(QTextStream&);

	// ---------------------------------------------------------------

	bool startDocument();
	bool startElement(const QString&, const QString&, const QString&,
	                  const QXmlAttributes&);
	bool endElement(const QString&, const QString&, const QString&);
	bool characters(const QString& ch);
	void reportError(const QString& err);
	void setDocumentLocator(QXmlLocator * locator);

private:


	void calcDivisions();
	QString strAccid(Accidentals::Accid);
	void writeBeams(QTextStream&, TabTrack *, int, int);
	int writeCol(QTextStream&, TabTrack *, int, int, bool);
	void writePitch(QTextStream&, int, QString, QString);
	void writeStaffDetails(QTextStream&, TabTrack *);
	void writeTime(QTextStream&, int, int);
	int divisions;

	// following variables are used by writeCol only
	int tEndPrev;				// end time of previous note
	int trpCnt;					// triplet count (0=none, 1=1st...3=3rd)
	int tStartCur;				// start time current note

	/**
	 * I/O stream, used by converter.
	 */
	QTextStream *stream;

	/**
	 * Accidentals state
	 */
	Accidentals accSt;

	bool addNote();
	bool addTrack();
	void initStNote();
	void initStScorePart();
	void initStStaffTuning();
	void reportAll(const QString& lvl, const QString& err);
	void reportWarning(const QString& err);

	TabTrack * trk;				// pointer to current track
	QPtrVector<QString> partIds;	// part (track) id's
	int x;						// current column
	int bar;					// current bar
	int iDiv;					// divisions
	int tEndCur;				// end time current note
	QXmlLocator * lctr;

	// state variables for parsing
	// general -- initialized in startDocument()
	QString stCha;				// characters collected
	// identification -- initialized in startDocument()
	QString stCrt;				// creator
	QString stEnc;				// encoder
	QString stTtl;				// title
	// measure -- initialized in startDocument()
	QString stBts;				// beats
	QString stBtt;				// beat-type
	QString stDiv;				// divisions
	QString stFif;				// fifths
	// note (including forward/backup) -- initialized in initStNote()
	QString stAlt;				// alter
	QString	stAno;				// actual notes
	bool    stCho;				// chord with previous note
	int     stDts;				// dots (count)
	QString stDur;				// duration
	QString stFrt;				// fret
	bool    stGls;				// glissando
	bool    stHmr;				// hammer-on
	QString	stNno;				// normal notes
	QString stOct;				// octave
	bool    stPlo;				// pull-off
	bool    stRst;				// rest
	QString stStp;				// step
	QString stStr;				// string
	bool    stTie;				// tie stop
	QString stTyp;				// type
	// part (== track) -- initialized in initStScorePart()
	QString stPid;				// ID
	QString stPmb;				// MIDI bank
	QString stPmc;				// MIDI channel
	QString stPmp;				// MIDI program
	QString stPnm;				// name
	// tuning -- initialized in initStStaffTuning()
	QString stPtl;				// tuning line
	QString stPtn;				// tuning number of lines
	QString stPto;				// tuning octave
	QString stPts;				// tuning step
};

#endif
