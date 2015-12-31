#ifndef FINGERLIST_H
#define FINGERLIST_H

#include <QTableView>
#include "global.h"

class TabTrack;
class FingerListModel;

class FingerList: public QTableView {
	Q_OBJECT
public:
	FingerList(TabTrack *p, QWidget *parent = 0);

	void addFingering(const int a[MAX_STRINGS]);
	void beginSession();
	void endSession();
	int count();
	void selectFirst();

signals:
	void chordSelected(const int *);

private slots:
	void currentChangedSlot(const QModelIndex &current, const QModelIndex &previous);

protected:
	virtual void resizeEvent(QResizeEvent *) override;

private:
	FingerListModel* flmodel;
};

#endif
