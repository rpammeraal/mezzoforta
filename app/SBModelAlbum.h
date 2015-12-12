#ifndef SBMODELALBUM_H
#define SBMODELALBUM_H

#include "SBID.h"

class SBSqlQueryModel;

class SBModelAlbum
{
public:
    static QStringList addSongToAlbum(const SBID& album, const SBID& song);
    static SBID getDetail(const SBID& id);
    static SBSqlQueryModel* getAllSongs(const SBID& id);
    static SBSqlQueryModel* matchAlbum(const SBID& newAlbum);
    static QStringList mergeAlbum(const SBID& from, const SBID& to);
    static QStringList mergeSongInAlbum(const SBID& album, int newPosition, const SBID& song);
    static QStringList removeAlbum(const SBID& album);
    static QStringList removeSongFromAlbum(const SBID& album, int position);
    static QStringList repositionSongOnAlbum(int albumID, int fromPosition, int toPosition);
    static bool updateExistingAlbum(const SBID& orgAlbum, const SBID& newAlbum, const QStringList& SQL,bool commitFlag=1);

};

#endif // SBMODELALBUM_H
