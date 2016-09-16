#ifndef DATAENTITYPERFORMER_H
#define DATAENTITYPERFORMER_H

#include <QObject>

#include "SBIDPerformer.h"

class SBSqlQueryModel;

class DataEntityPerformer : public QObject
{
    Q_OBJECT

public:
    DataEntityPerformer();
    ~DataEntityPerformer();

    QString addRelatedPerformerSQL(int performerID1, int performerID2) const;
    QString deleteRelatedPerformerSQL(int performerID1, int performerID2) const;
	SBIDPerformer getDetail(const SBIDBase& id);
	SBSqlQueryModel* getAllAlbums(const SBIDBase& id);
	SBSqlQueryModel* getAllCharts(const SBIDBase& id);
    SBSqlQueryModel* getAllPerformers();
	SBSqlQueryModel* getAllSongs(const SBIDBase& id);
	SBSqlQueryModel* getAllOnlineSongs(const SBIDBase& id);
	SBSqlQueryModel* getRelatedPerformers(const SBIDBase& id);
	SBSqlQueryModel* matchPerformer(const SBIDBase& id, const QString& newPerformerName);	//	DEPRECIATED: USE SBIDPerformer::match()
	bool updateExistingPerformer(const SBIDBase& orgPerformerID, SBIDPerformer& newPerformerID, const QStringList& extraSQL=QStringList(),bool commitFlag=1);

    static void updateSoundexFields();	//	CWIP: may be removed if database generation and updates are implemented

public slots:
	void updateHomePage(const SBIDPerformer& id);
	void updateMBID(const SBIDPerformer& id);
};

#endif // DATAENTITYPERFORMER_H
