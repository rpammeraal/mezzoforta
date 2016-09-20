#ifndef DATAENTITYPLAYLIST_H
#define DATAENTITYPLAYLIST_H

#include <QHash>
#include <QObject>

#include "SBIDSong.h"
#include "SBIDPlaylist.h"

class SBSqlQueryModel;

class DataEntityPlaylist : public QObject
{
    Q_OBJECT

public:
    DataEntityPlaylist();
    ~DataEntityPlaylist();

    void assignPlaylistItem(const SBIDBase& assignID, const SBIDBase& toID) const;
    SBIDPlaylist createNewPlaylist() const;
    void deletePlaylistItem(SBIDBase::sb_type itemType,int playlistID, int playlistPosition);
    void deletePlaylist(const SBIDBase& id) const;
    SBIDBase getDetail(const SBIDBase& id) const;
    SBIDSong getDetailPlaylistItemSong(int playlistID, int playlistPosition) const;
    SBSqlQueryModel* getAllItemsByPlaylist(const SBIDBase& id) const;
    void getAllItemsByPlaylistRecursive(QList<SBIDBase>& compositesTraversed, QList<SBIDBase>& allSongs, const SBIDPtr& root) const;
    SBSqlQueryModel* getAllPlaylists() const;
    void recalculateAllPlaylistDurations() const;
    void recalculatePlaylistDuration(const SBIDPtr& ptr) const;
    void renamePlaylist(const SBIDBase& id) const;
    void reorderItem(const SBIDBase& playlistID, const SBIDBase& fromID, int row) const;
    void reorderItem(const SBIDBase& playlistID, const SBIDBase& fromID, const SBIDBase& toID) const;
    QMap<int,SBIDBase> retrievePlaylistItems(const SBIDBase& id);

private:
    void init();
    void reorderPlaylistPositions(const SBIDBase& id,int maxPosition=INT_MAX) const;
};

#endif // DATAENTITYPLAYLIST_H

