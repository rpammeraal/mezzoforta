#ifndef SBMODELALBUM_H
#define SBMODELALBUM_H

#include "SBID.h"

class SBModelAlbum
{
public:
    static SBID getDetail(const SBID& id);
    static SBModelSonglist* getAllSongs(const SBID& id);
};

#endif // SBMODELALBUM_H
