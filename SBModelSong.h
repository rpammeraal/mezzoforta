#ifndef SBMODELSONG_H
#define SBMODELSONG_H

#include <QSqlQueryModel>

#include "SBID.h"

class SBModelSonglist;

class SBModelSong
{
public:
    static SBID getDetail(const SBID& id);
    static SBModelSonglist* getAllSongs();
    static SBModelSonglist* getPerformedByListBySong(const SBID& id);
    static SBModelSonglist* getOnAlbumListBySong(const SBID& id);
    static SBModelSonglist* getOnChartListBySong(const SBID& id);
    static SBModelSonglist* getOnPlaylistListBySong(const SBID& id);

private:
};

#endif // SBMODELSONG_H
