#ifndef DATAENTITYPLAYLIST_H
#define DATAENTITYPLAYLIST_H

#include <QHash>
#include <QObject>

#include "SBID.h"
#include "SBTime.h"

class SBSqlQueryModel;

class DataEntityPlaylist : public QObject
{
    Q_OBJECT

public:
    DataEntityPlaylist();
    ~DataEntityPlaylist();

    void assignPlaylistItem(const SBID& assignID, const SBID& toID) const;
    SBID createNewPlaylist() const;
    void deletePlaylistItem(const SBID& assignID, const SBID& fromID) const;
    void deletePlaylist(const SBID& id) const;
    SBID getDetail(const SBID& id) const;
    SBSqlQueryModel* getAllItemsByPlaylist(const SBID& id) const;
    void getAllItemsByPlaylistRecursive(QHash<int,int>& compositesTraversed, QList<SBID>& allSongs, const SBID& id) const;
    SBSqlQueryModel* getAllPlaylists() const;
    void recalculateAllPlaylistDurations() const;
    void recalculatePlaylistDuration(const SBID& id) const;
    void renamePlaylist(const SBID& id) const;
    void reorderItem(const SBID& playlistID, const SBID& fromID, const SBID& toID) const;

private:
    void init();
    void reorderPlaylistPositions(const SBID& id,int maxPosition=INT_MAX) const;
};

#endif // DATAENTITYPLAYLIST_H

