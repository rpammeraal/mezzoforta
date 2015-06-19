#ifndef SBMODELALBUM_H
#define SBMODELALBUM_H

#include "SBID.h"

class SBModelList;

class SBModelAlbum
{
public:
    static SBID getDetail(const SBID& id);
    static SBModelList* getAllSongs(const SBID& id);
};

#endif // SBMODELALBUM_H
