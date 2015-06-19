#ifndef SBMODELSONG_H
#define SBMODELSONG_H

#include <QSqlQueryModel>

#include "SBID.h"

class SBModelList;

class SBModelSong
{
public:
    static SBID getDetail(const SBID& id);
    static SBModelList* getAllSongs();
    static SBModelList* getPerformedByListBySong(const SBID& id);
    static SBModelList* getOnAlbumListBySong(const SBID& id);
    static SBModelList* getOnChartListBySong(const SBID& id);
    static SBModelList* getOnPlaylistListBySong(const SBID& id);

private:
};

#endif // SBMODELSONG_H
