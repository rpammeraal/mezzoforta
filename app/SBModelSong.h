#ifndef SBMODELSONG_H
#define SBMODELSONG_H

#include <QSqlQueryModel>

#include "SBID.h"

class SBSqlQueryModel;

class SBModelSong
{
public:
    static SBID getDetail(const SBID& id);
    static SBSqlQueryModel* findSong(const SBID& id);
    static SBSqlQueryModel* getAllSongs();
    static int getMaxSongID();
    static SBSqlQueryModel* getPerformedByListBySong(const SBID& id);
    static SBSqlQueryModel* getOnAlbumListBySong(const SBID& id);
    static SBSqlQueryModel* getOnChartListBySong(const SBID& id);
    static SBSqlQueryModel* getOnPlaylistListBySong(const SBID& id);
    static SBSqlQueryModel* matchSong(const SBID& newSongID);
    static SBSqlQueryModel* matchSongWithinPerformer(const SBID& newSongID, const QString& newSongTitle);
    static bool updateExistingSong(const SBID& orgSongID, SBID& newSongID, const QStringList& extraSQL=QStringList(),bool commitFlag=1);
    static bool saveNewSong(SBID& newSongID);
    static void updateSoundexFields();	//	CWIP: may be removed if database generation and updates are implemented

private:
};

#endif // SBMODELSONG_H
