#ifndef DATAENTITYALBUM_H
#define DATAENTITYALBUM_H

#include "SBIDAlbum.h"
#include "SBIDSong.h"

class SBSqlQueryModel;

class DataEntityAlbum
{
public:
    static QStringList addSongToAlbum(const SBID& song);
    static SBIDAlbum getDetail(const SBID& id);
    static SBSqlQueryModel* getAllSongs(const SBID& id);
    static SBSqlQueryModel* matchAlbum(const SBID& newAlbum);
    static QStringList mergeAlbum(const SBID& from, const SBID& to);
    static QStringList mergeSongInAlbum(const SBID& album, int newPosition, const SBID& song);
    static QStringList removeAlbum(const SBID& album);
    static QStringList removeSongFromAlbum(const SBID& album, int position);
    static QStringList repositionSongOnAlbum(int albumID, int fromPosition, int toPosition);
    static bool updateExistingAlbum(const SBID& orgAlbum, const SBID& newAlbum, const QStringList& SQL,bool commitFlag=1);
    static QStringList updateSongOnAlbum(int albumID, const SBIDSong& song);
};

#endif // DATAENTITYALBUM_H
