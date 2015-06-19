#ifndef SBMODELPERFORMER_H
#define SBMODELPERFORMER_H

#include <QObject>

#include "SBID.h"

class SBModelList;

class SBModelPerformer : public QObject
{
    Q_OBJECT

public:
    SBModelPerformer();
    ~SBModelPerformer();

    SBID getDetail(const SBID& id);
    SBModelList* getAllAlbums(const SBID& id);
    SBModelList* getAllCharts(const SBID& id);
    SBModelList* getAllSongs(const SBID& id);
    SBModelList* getRelatedPerformers(const SBID& id);

public slots:
    void updateHomePage(const SBID& id);
    void updateMBID(const SBID& id);
};

#endif // SBMODELPERFORMER_H
