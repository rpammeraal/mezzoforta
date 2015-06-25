#ifndef SBMODELPLAYLIST_H
#define SBMODELPLAYLIST_H

#include <QObject>

class SBModelList;

class SBModelPlaylist
{
public:
    static SBID createNewPlaylist();
    static void deletePlaylist(const SBID& id);
    static SBID getDetail(const SBID& id);
    static SBModelList* getAllItemsByPlaylist(const SBID& id);
    static SBModelList* getAllPlaylists();
    static void renamePlaylist(const SBID& id);
};

#endif // SBMODELPLAYLIST_H
