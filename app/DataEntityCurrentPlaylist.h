#ifndef DATAENTITYCURRENTPLAYLIST_H
#define DATAENTITYCURRENTPLAYLIST_H

#include <QSqlQueryModel>

#include "SBID.h"

class SBSqlQueryModel;

class DataEntityCurrentPlaylist
{
public:
    static SBSqlQueryModel* getAllOnlineSongs();
    static SBSqlQueryModel* getAllSongs();
    static SBID getFirstUnplayedSong();
    static void resetPlaylist();
    static void setSongAttributes(int playID,bool activeFlag=0, bool hasPlayedFlag=0);

private:
};

#endif // DATAENTITYCURRENTPLAYLIST_H