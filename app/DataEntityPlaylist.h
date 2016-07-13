#ifndef DATAENTITYPLAYLIST_H
#define DATAENTITYPLAYLIST_H

#include <QHash>
#include <QObject>

#include "SBIDSong.h"

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
    SBIDSong getDetailPlaylistItemSong(const SBID& id) const;
    SBSqlQueryModel* getAllItemsByPlaylist(const SBID& id) const;
    void getAllItemsByPlaylistRecursive(QList<SBID>& compositesTraversed, QList<SBID>& allSongs, const SBID& id) const;
    SBSqlQueryModel* getAllPlaylists() const;
    void recalculateAllPlaylistDurations() const;
    void recalculatePlaylistDuration(const SBID& id) const;
    void renamePlaylist(const SBID& id) const;
    void reorderItem(const SBID& playlistID, const SBID& fromID, int row) const;
    void reorderItem(const SBID& playlistID, const SBID& fromID, const SBID& toID) const;
    QMap<int,SBID> retrievePlaylistItems(const SBID &id);

private:
    void init();
    void reorderPlaylistPositions(const SBID& id,int maxPosition=INT_MAX) const;
};

#endif // DATAENTITYPLAYLIST_H

