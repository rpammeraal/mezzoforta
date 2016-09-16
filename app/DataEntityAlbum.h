#ifndef DATAENTITYALBUM_H
#define DATAENTITYALBUM_H

#include "SBIDAlbum.h"
#include "SBIDSong.h"

class SBSqlQueryModel;

class DataEntityAlbum
{
public:
    static QStringList addSongToAlbum(const SBIDSong& song);
    static SBIDAlbum getDetail(const SBIDBase& id);
    static SBSqlQueryModel* getAllSongs(const SBIDBase& id);
    static SBSqlQueryModel* matchAlbum(const SBIDBase& newAlbum);
    static QStringList mergeAlbum(const SBIDBase& from, const SBIDBase& to);
    static QStringList mergeSongInAlbum(const SBIDBase& album, int newPosition, const SBIDBase& song);
    static QStringList removeAlbum(const SBIDBase& album);
    static QStringList removeSongFromAlbum(const SBIDBase& album, int position);
    static QStringList repositionSongOnAlbum(int albumID, int fromPosition, int toPosition);
    static bool updateExistingAlbum(const SBIDBase& orgAlbum, const SBIDBase& newAlbum, const QStringList& SQL,bool commitFlag=1);
    static QStringList updateSongOnAlbum(int albumID, const SBIDSong& song);
};

#endif // DATAENTITYALBUM_H
