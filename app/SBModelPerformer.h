#ifndef SBMODELPERFORMER_H
#define SBMODELPERFORMER_H

#include <QObject>

#include "SBID.h"

class SBSqlQueryModel;

class SBModelPerformer : public QObject
{
    Q_OBJECT

public:
    SBModelPerformer();
    ~SBModelPerformer();

    QString addRelatedPerformer(int performerID1, int performerID2) const;
    QString deleteRelatedPerformer(int performerID1, int performerID2) const;
    SBID getDetail(const SBID& id);
    SBSqlQueryModel* getAllAlbums(const SBID& id);
    SBSqlQueryModel* getAllCharts(const SBID& id);
    SBSqlQueryModel* getAllPerformers();
    SBSqlQueryModel* getAllSongs(const SBID& id);
    SBSqlQueryModel* getRelatedPerformers(const SBID& id);
    SBSqlQueryModel* matchPerformer(const SBID& id, const QString& newPerformerName);
    bool saveNewPerformer(SBID& newPerformerID);
    bool updateExistingPerformer(const SBID& orgPerformerID, SBID& newPerformerID, const QStringList& extraSQL=QStringList());

    static void updateSoundexFields();	//	CWIP: may be removed if database generation and updates are implemented

public slots:
    void updateHomePage(const SBID& id);
    void updateMBID(const SBID& id);
};

#endif // SBMODELPERFORMER_H
