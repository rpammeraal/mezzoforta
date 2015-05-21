#ifndef SBMODELPLAYLIST_H
#define SBMODELPLAYLIST_H

#include <QObject>

class SBModelSonglist;

class SBModelPlaylist //: public SBModel
{
public:
    static SBID getDetail(const SBID& id);
    static SBModelSonglist* getAllItemsByPlaylist(const SBID& id);
    static SBModelSonglist* getAllPlaylists();
};

#endif // SBMODELPLAYLIST_H
