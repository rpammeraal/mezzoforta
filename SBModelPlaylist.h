#ifndef SBMODELPLAYLIST_H
#define SBMODELPLAYLIST_H

#include <QObject>

class SBSqlQueryModel;

class SBModelPlaylist
{
public:
    static SBID createNewPlaylist();
    static void assignItem(const SBID& assignID, const SBID& toID);
    static void deleteItem(const SBID& assignID, const SBID& fromID);
    static void deletePlaylist(const SBID& id);
    static SBID getDetail(const SBID& id);
    static SBSqlQueryModel* getAllItemsByPlaylist(const SBID& id);
    static SBSqlQueryModel* getAllPlaylists();
    static void renamePlaylist(const SBID& id);

private:
    static void reorderPlaylistPositions(const SBID& id);
};

#endif // SBMODELPLAYLIST_H
