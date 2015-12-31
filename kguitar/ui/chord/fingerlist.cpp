#include "fingerlist.h"
#include "global.h"
#include "data/tabtrack.h"

#include <qpainter.h>
#include <qcolor.h>
#include <qstyle.h>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QPalette>
#include <QStyleOptionFocusRect>
#include <QStyledItemDelegate>
#include <QHeaderView>

#define ICONCHORD                50
#define FRET_NUMBER_FONT_FACTOR  0.7

typedef struct {
	int f[MAX_STRINGS];
} fingering;

Q_DECLARE_METATYPE(fingering)

// ============== MODEL ====================
class FingerListModel : public QAbstractTableModel
{
public:
	explicit FingerListModel(QObject *parent = 0)
		: QAbstractTableModel(parent)
		, perRow(0)
		, numRows(0)
		, numCols(0)
	{}

	enum FingerListModelRoles {
		FingeringRole = Qt::UserRole
	};

	void beginSession();
	void endSession();
	void addFingering(const int a[MAX_STRINGS]);
	void clear();
	int count();
	void resetNumRows();
	void resetNumCols();

	int perRow, num;

	// QAbstractItemModel interface
public:
	int rowCount(const QModelIndex &parent = QModelIndex()) const override;
	int columnCount(const QModelIndex &parent = QModelIndex()) const override;
	QVariant data(const QModelIndex &index, int role) const override;

private:
	int numRows, numCols;
	QVector<fingering> appl;
};

void FingerListModel::beginSession()
{
	clear();
}

void FingerListModel::endSession()
{
	// num is overral number of chord fingerings. If it's 0 - then there are no
	// fingerings. In the appl array, indexes should be ranged from 0 to (num-1)
	resetNumRows();
	resetNumCols();
}

void FingerListModel::addFingering(const int a[])
{
	appl.resize((num + 1) * MAX_STRINGS);

	for (int i = 0; i < MAX_STRINGS; i++)
		appl[num].f[i] = a[i];

	num++;
}

void FingerListModel::clear()
{
	beginResetModel();

	appl.resize(0);
	num = 0;
	numRows = 0;
	numCols = 0;

	endResetModel();
}

int FingerListModel::count()
{
	return appl.count();
}

void FingerListModel::resetNumRows()
{
	int numRows_ = (num - 1) / (perRow - 1) + 1;

	if( numRows == numRows_ || appl.isEmpty() )
		return;

	if( numRows < numRows_) {
		beginInsertRows(QModelIndex(), numRows, numRows_ - 1);

		numRows = numRows_;

		endInsertRows();
	} else {
		beginRemoveRows(QModelIndex(), 0, numRows - numRows_ - 1);

		numRows = numRows_;

		endRemoveRows();
	}
}

void FingerListModel::resetNumCols()
{
	int numCols_ = perRow - 1;
	if( num < numCols_ )
		numCols_ = num;

	if( numCols == numCols_ || appl.isEmpty() )
		return;

	if( numCols < numCols_ ) {
		beginInsertColumns(QModelIndex(), numCols, numCols_ - 1);

		numCols = numCols_;

		endInsertColumns();
	} else {
		beginRemoveColumns(QModelIndex(), 0, numCols - numCols_ - 1);

		numCols = numCols_;

		endRemoveColumns();
	}
}

int FingerListModel::rowCount(const QModelIndex &) const
{
	return numRows;
}

int FingerListModel::columnCount(const QModelIndex &) const
{
	return numCols;
}

QVariant FingerListModel::data(const QModelIndex &index, int role) const
{
	int fingeringIndex = index.column() + index.row() * (perRow - 1);
	if(fingeringIndex >= num)
		return QVariant();

	if(role == FingeringRole)
		return QVariant::fromValue(appl[fingeringIndex]);

	return QVariant();
}

namespace {
	// ============== DELEGATE ====================
	class FingerListDelegate : public QStyledItemDelegate
	{
	public:
		explicit FingerListDelegate(TabTrack *parm_, QObject *parent = 0);
		~FingerListDelegate();
		enum {
			SCALE=6,
			CIRCLE=5,
			CIRCBORD=1,
			BORDER=1,
			SPACER=1,
			FRETTEXT=9
		};
		// QAbstractItemDelegate interface
	public:
		void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

	private:
		QFont *fretNumberFont;
		TabTrack *parm;
	};

	void FingerListDelegate::paint(QPainter *p, const QStyleOptionViewItem &option, const QModelIndex &index) const
	{
		QVariant variantData = index.data(FingerListModel::FingeringRole);
		if(!variantData.isValid())
			return;

		fingering f = variantData.value<fingering>();


		QStyledItemDelegate::paint(p, option, index);

		QColor back = option.palette.color(QPalette::Base);
		QColor fore = option.palette.color(QPalette::Text);

		p->setPen(fore);

		// Horizontal lines

		for (int i = 0; i <= NUMFRETS; i++)
			p->drawLine((SCALE/2+BORDER+FRETTEXT) + option.rect.left(),
			            BORDER+SCALE+2*SPACER+i*SCALE + option.rect.top(),
			            SCALE/2+BORDER+parm->string*SCALE-SCALE+FRETTEXT + option.rect.left(),
			            BORDER+SCALE+2*SPACER+i*SCALE + option.rect.top());

		// Beginning fret number

		int firstFret = parm->frets;
		bool noff = TRUE;

		for (int i = 0; i < parm->string; i++) {
			if ((f.f[i] < firstFret) && (f.f[i] > 0))
				firstFret = f.f[i];
			if (f.f[i] > 5)
				noff = FALSE;
		}

		if (noff)
			firstFret = 1;

		if (firstFret > 1) {
			QString fs;
			fs.setNum(firstFret);
			p->setFont(*fretNumberFont);
			p->drawText(BORDER + option.rect.left(), BORDER + SCALE + 2 * SPACER + option.rect.top(), 50, 50,
			            Qt::AlignLeft | Qt::AlignTop, fs);
		}

		// Vertical lines and fingering

		for (int i = 0; i < parm->string; i++) {
			p->drawLine(i * SCALE + BORDER + SCALE / 2 + FRETTEXT + option.rect.left(),
			            BORDER + SCALE + 2 * SPACER + option.rect.top(),
			            i * SCALE + BORDER + SCALE / 2 + FRETTEXT + option.rect.left(),
			            BORDER + SCALE + 2 * SPACER + NUMFRETS * SCALE + option.rect.top());
			if (f.f[i] == -1) {
				p->drawLine(i*SCALE+BORDER+CIRCBORD+FRETTEXT + option.rect.left(),
				            BORDER+CIRCBORD + option.rect.top(),
				            i*SCALE+BORDER+SCALE-CIRCBORD+FRETTEXT + option.rect.left(),
				            BORDER+SCALE-CIRCBORD + option.rect.top());
				p->drawLine(i*SCALE+BORDER+SCALE-CIRCBORD+FRETTEXT + option.rect.left(),
				            BORDER+CIRCBORD + option.rect.top(),
				            i*SCALE+BORDER+CIRCBORD+FRETTEXT + option.rect.left(),
				            BORDER+SCALE-CIRCBORD + option.rect.top());
			} else if (f.f[i]==0) {
				p->setBrush(back);
				p->drawEllipse(i*SCALE+BORDER+CIRCBORD+FRETTEXT + option.rect.left(),
				               BORDER+CIRCBORD + option.rect.top(),
				               CIRCLE,
				               CIRCLE);
			} else {
				p->setBrush(fore);
				p->drawEllipse(i*SCALE+BORDER+CIRCBORD+FRETTEXT + option.rect.left(),
				               BORDER+SCALE+2*SPACER+(f.f[i]-firstFret)*SCALE+CIRCBORD + option.rect.top(),
				               CIRCLE,
				               CIRCLE);
			}
		}

		// Analyze & draw barre

		p->setBrush(fore);

		int barre, eff;

		for (int i = 0; i < NUMFRETS; i++) {
			barre = 0;
			while ((f.f[parm->string - barre - 1] >= (i + firstFret)) ||
				   (f.f[parm->string - barre - 1] == -1)) {
				barre++;
				if (barre > parm->string - 1)
					break;
			}

			while ((f.f[parm->string-barre]!=(i+firstFret)) && (barre>1))
				barre--;

			eff = 0;
			for (int j = parm->string-barre; j < parm->string; j++) {
				if (f.f[j] != -1)
					eff++;
			}

			if (eff > 2) {
				p->drawRect((parm->string-barre) * SCALE + SCALE / 2 +
				            BORDER + FRETTEXT + option.rect.left(),
				            BORDER + SCALE + 2 * SPACER + i * SCALE + CIRCBORD + option.rect.top(),
				            (barre - 1) * SCALE, CIRCLE);
			}
		}

		p->setBrush(Qt::NoBrush);
		p->setPen(Qt::SolidLine);
	}

	FingerListDelegate::FingerListDelegate(TabTrack *parm_, QObject *parent)
		: QStyledItemDelegate(parent)
		, parm(parm_)
	{
		fretNumberFont = new QFont(/*font()*/);
		if (fretNumberFont->pointSize() == -1) {
			fretNumberFont->setPixelSize((int) ((double) fretNumberFont->pixelSize() * FRET_NUMBER_FONT_FACTOR));
		} else {
			fretNumberFont->setPointSizeF(fretNumberFont->pointSizeF() * FRET_NUMBER_FONT_FACTOR);
		}
	}

	FingerListDelegate::~FingerListDelegate()
	{
		delete fretNumberFont;
	}
}

FingerList::FingerList(TabTrack *p, QWidget *parent)
	: QTableView(parent)
	, flmodel(nullptr)
{
	//setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	setFrameStyle(Panel | Sunken);
	setBackgroundRole(QPalette::Base);
	setFocusPolicy(Qt::StrongFocus);
	setShowGrid(false);
	horizontalHeader()->setResizeMode(QHeaderView::Fixed);
	verticalHeader()->setResizeMode(QHeaderView::Fixed);
	horizontalHeader()->setDefaultSectionSize(ICONCHORD);
	verticalHeader()->setDefaultSectionSize(ICONCHORD);
	horizontalHeader()->hide();
	verticalHeader()->hide();
	setSelectionMode(SingleSelection);

	setItemDelegate(new FingerListDelegate(p, this));
	flmodel = new FingerListModel(this);
	setModel(flmodel);

	setMinimumSize(ICONCHORD + 2, ICONCHORD + 2);
	resize(width(), 3 * ICONCHORD + 2);

	connect(selectionModel(),SIGNAL(currentChanged(const QModelIndex &, const QModelIndex &)),SLOT(currentChangedSlot(const QModelIndex &, const QModelIndex &)));

	repaint();
}

// Begins new "session" for fingering list, i.e. clears it and
// prepares for adding new chords.
void FingerList::beginSession()
{
	flmodel->beginSession();
}

// Ends adding "session" for the list, setting proper number of
// columns / rows, updating it, etc.
void FingerList::endSession()
{
	// num is overral number of chord fingerings. If it's 0 - then there are no
	// fingerings. In the appl array, indexes should be ranged from 0 to (num-1)
	flmodel->endSession();
	viewport()->update();
}

int FingerList::count() { return flmodel->count(); }

void FingerList::addFingering(const int a[MAX_STRINGS])
{
	flmodel->addFingering(a);
}
// TODO: known bug : wrong behaviour of selection during resizing
void FingerList::resizeEvent(QResizeEvent *e)
{
	QTableView::resizeEvent(e);
	flmodel->perRow = viewport()->width() / ICONCHORD + 1;
	flmodel->endSession();
}

void FingerList::selectFirst()
{
	fingering f = model()->data(model()->index(0, 0), FingerListModel::FingeringRole).value<fingering>();
	emit chordSelected(f.f);
}

void FingerList::currentChangedSlot(const QModelIndex &current, const QModelIndex &)
{
	fingering f = current.data( FingerListModel::FingeringRole ).value<fingering>();
	emit chordSelected(f.f);
}
