#ifndef DATAENTITYCURRENTPLAYLIST_H
#define DATAENTITYCURRENTPLAYLIST_H

#include <QSqlQueryModel>

#include "SBID.h"

class SBSqlQueryModel;

class DataEntityCurrentPlaylist
{
public:
    static SBSqlQueryModel* getAllOnlineSongs();

private:
};

#endif // DATAENTITYCURRENTPLAYLIST_H
