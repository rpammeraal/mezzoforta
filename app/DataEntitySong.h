#ifndef DATAENTITYSONG_H
#define DATAENTITYSONG_H

#include <QSqlQueryModel>

#include "SBIDSong.h"

class SBSqlQueryModel;

class DataEntitySong
{
public:
    static SBSqlQueryModel* findSong(const SBIDBase& id);
    static SBSqlQueryModel* getAllSongs();
    static int getMaxSongID();
    static SBSqlQueryModel* getPerformedByListBySong(const SBIDBase& id);
    static SBSqlQueryModel* getOnAlbumListBySong(const SBIDBase& id);
    static SBSqlQueryModel* getOnChartListBySong(const SBIDBase& id);
    static SBSqlQueryModel* getOnlineSongs();
    static SBSqlQueryModel* getOnPlaylistListBySong(const SBIDBase& id);
    static SBSqlQueryModel* matchSong(const SBIDBase& newSongID);
    static SBSqlQueryModel* matchSongWithinPerformer(const SBIDBase& newSongID, const QString& newSongTitle);
    static bool updateExistingSong(const SBIDBase& orgSongID, SBIDSong& newSongID, const QStringList& extraSQL=QStringList(),bool commitFlag=1);
    static bool updateLastPlayDate(const SBIDBase& id);
    static void updateSoundexFields();	//	CWIP: may be removed if database generation and updates are implemented

private:
};

#endif // DATAENTITYSONG_H
