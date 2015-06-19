#ifndef SBMODELPLAYLIST_H
#define SBMODELPLAYLIST_H

#include <QObject>

class SBModelList;

class SBModelPlaylist
{
public:
    static SBID getDetail(const SBID& id);
    static SBModelList* getAllItemsByPlaylist(const SBID& id);
    static SBModelList* getAllPlaylists();
};

#endif // SBMODELPLAYLIST_H
