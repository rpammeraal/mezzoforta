#ifndef SBMODELSONG_H
#define SBMODELSONG_H

#include <QSqlQueryModel>

#include "SBID.h"

class SBSqlQueryModel;

class SBModelSong
{
public:
    static SBID getDetail(const SBID& id);
    static SBSqlQueryModel* getAllSongs();
    static SBSqlQueryModel* getPerformedByListBySong(const SBID& id);
    static SBSqlQueryModel* getOnAlbumListBySong(const SBID& id);
    static SBSqlQueryModel* getOnChartListBySong(const SBID& id);
    static SBSqlQueryModel* getOnPlaylistListBySong(const SBID& id);

private:
};

#endif // SBMODELSONG_H
