#ifndef DATAENTITYPERFORMER_H
#define DATAENTITYPERFORMER_H

#include <QObject>

#include "SBID.h"

class SBSqlQueryModel;

class DataEntityPerformer : public QObject
{
    Q_OBJECT

public:
    DataEntityPerformer();
    ~DataEntityPerformer();

    QString addRelatedPerformerSQL(int performerID1, int performerID2) const;
    QString deleteRelatedPerformerSQL(int performerID1, int performerID2) const;
    SBID getDetail(const SBID& id);
    SBSqlQueryModel* getAllAlbums(const SBID& id);
    SBSqlQueryModel* getAllCharts(const SBID& id);
    SBSqlQueryModel* getAllPerformers();
    SBSqlQueryModel* getAllSongs(const SBID& id);
    SBSqlQueryModel* getAllOnlineSongs(const SBID& id);
    SBSqlQueryModel* getRelatedPerformers(const SBID& id);
	SBSqlQueryModel* matchPerformer(const SBID& id, const QString& newPerformerName);	//	DEPRECIATED: USE SBIDPerformer::match()
    bool updateExistingPerformer(const SBID& orgPerformerID, SBID& newPerformerID, const QStringList& extraSQL=QStringList(),bool commitFlag=1);

    static void updateSoundexFields();	//	CWIP: may be removed if database generation and updates are implemented

public slots:
    void updateHomePage(const SBID& id);
    void updateMBID(const SBID& id);
};

#endif // DATAENTITYPERFORMER_H
