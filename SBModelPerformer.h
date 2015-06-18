#ifndef SBMODELPERFORMER_H
#define SBMODELPERFORMER_H

#include <QObject>

#include "SBID.h"

class SBModelSonglist;

class SBModelPerformer : public QObject
{
    Q_OBJECT

public:
    SBModelPerformer();
    ~SBModelPerformer();

    SBID getDetail(const SBID& id);
    SBModelSonglist* getAllAlbums(const SBID& id);
    SBModelSonglist* getAllCharts(const SBID& id);
    SBModelSonglist* getAllSongs(const SBID& id);
    SBModelSonglist* getRelatedPerformers(const SBID& id);

public slots:
    void updateHomePage(const SBID& id);
    void updateMBID(const SBID& id);
};

#endif // SBMODELPERFORMER_H
