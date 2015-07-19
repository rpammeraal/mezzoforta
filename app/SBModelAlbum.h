#ifndef SBMODELALBUM_H
#define SBMODELALBUM_H

#include "SBID.h"

class SBSqlQueryModel;

class SBModelAlbum
{
public:
    static SBID getDetail(const SBID& id);
    static SBSqlQueryModel* getAllSongs(const SBID& id);
};

#endif // SBMODELALBUM_H
