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

    SBID getDetail(const SBID& id);
    SBSqlQueryModel* getAllAlbums(const SBID& id);
    SBSqlQueryModel* getAllCharts(const SBID& id);
    SBSqlQueryModel* getAllSongs(const SBID& id);
    SBSqlQueryModel* getRelatedPerformers(const SBID& id);
    SBSqlQueryModel* matchPerformer(const SBID& id, const QString& newPerformerName);
    int savePerformer(SBID& id);

    static void updateSoundexFields();	//	CWIP: may be removed if database generation and updates are implemented

public slots:
    void updateHomePage(const SBID& id);
    void updateMBID(const SBID& id);
};

#endif // SBMODELPERFORMER_H
