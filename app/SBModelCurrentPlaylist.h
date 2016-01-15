#ifndef SBMODELCURRENTPLAYLIST_H
#define SBMODELCURRENTPLAYLIST_H

#include <QSqlQueryModel>

#include "SBID.h"

class SBSqlQueryModel;

class SBModelCurrentPlaylist
{
public:
    static SBSqlQueryModel* getAllOnlineSongs();
    static SBSqlQueryModel* getAllSongs();
    static SBID getFirstUnplayedSong();
    static void resetPlaylist();
    static void setSongAttributes(int playID,bool activeFlag=0, bool hasPlayedFlag=0);

private:
};

#endif // SBMODELCURRENTPLAYLIST_H
