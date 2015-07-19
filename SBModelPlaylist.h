#ifndef SBMODELPLAYLIST_H
#define SBMODELPLAYLIST_H

#include <QHash>
#include <QObject>

#include "SBID.h"
#include "SBTime.h"

class SBSqlQueryModel;

class SBModelPlaylist : public QObject
{
    Q_OBJECT

public:
    SBModelPlaylist();
    ~SBModelPlaylist();

    void assignItem(const SBID& assignID, const SBID& toID);
    SBID createNewPlaylist();
    void deleteItem(const SBID& assignID, const SBID& fromID);
    void deletePlaylist(const SBID& id);
    SBID getDetail(const SBID& id);
    SBSqlQueryModel* getAllItemsByPlaylist(const SBID& id);
    void getAllItemsByPlaylistRecursive(QHash<int,int>& compositesTraversed, QList<SBID>& allSongs, const SBID& id);
    SBSqlQueryModel* getAllPlaylists();
    void recalculateAllPlaylistDurations();
    void renamePlaylist(const SBID& id);
    void reorderItem(const SBID& playlistID, const SBID& fromID, const SBID& toID);

signals:

private:
    void _init();
    void _calculatePlaylistDuration(const SBID& id);
    void _reorderPlaylistPositions(const SBID& id);
};

#endif // SBMODELPLAYLIST_H

