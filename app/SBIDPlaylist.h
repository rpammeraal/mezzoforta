#ifndef SBIDPLAYLIST_H
#define SBIDPLAYLIST_H

#include "SBIDBase.h"
#include "SBIDSong.h"

class SBIDPlaylist : public SBIDBase
{
public:
    //	Ctors, dtors
    SBIDPlaylist();
    SBIDPlaylist(const SBIDBase& c);
    SBIDPlaylist(const SBIDPlaylist& c);
    SBIDPlaylist(int itemID);
    ~SBIDPlaylist();

    //	Public methods
    virtual SBSqlQueryModel* findMatches(const QString& name) const;
    virtual QString genericDescription() const;
    virtual int getDetail(bool createIfNotExistFlag=0);	//	CWIP: pure virtual -- possible rename to retrieveDetail()
    virtual QString hash() const;
    virtual QString iconResourceLocation() const;
    virtual bool save();
    virtual int itemID() const;
    virtual sb_type itemType() const;
    virtual void sendToPlayQueue(bool enqueueFlag=0);
    virtual void setText(const QString &text);
    virtual QString text() const;
    virtual QString type() const;

    //	Methods specific to SBIDPlaylist
    void assignPlaylistItem(const SBIDPtr& ptr) const;
    static SBIDPlaylist createNewPlaylistDB();
    void deletePlaylist();
    void deletePlaylistItem(SBIDBase::sb_type itemType, int playlistPosition) const;
    SBSqlQueryModel* getAllItemsByPlaylist() const;
    static SBSqlQueryModel* getAllPlaylists();
    SBIDSong getDetailPlaylistItemSong(int playlistPosition) const;
    static void recalculatePlaylistDuration(const SBIDPtr& ptr);
    static void recalculateAllPlaylistDurations();
    void renamePlaylist();
    void reorderItem(const SBIDBase& fromID, int row) const;
    void reorderItem(const SBIDBase& fromID, const SBIDBase& toID) const;
    void setCount1(const int count1) { _count1=count1; }
    void setDuration(const Duration& duration) { _duration=duration; }
    void setPlaylistID(int playlistID) { _sb_playlist_id=playlistID; }
    void setPlaylistName(const QString& playlistName) { _playlistName=playlistName; }

    //	Operators
    virtual bool operator==(const SBIDBase& i) const;
    virtual operator QString() const;

private:
    static void _getAllItemsByPlaylistRecursive(QList<SBIDBase>& compositesTraversed, QList<SBIDBase>& allSongs, const SBIDPtr& root);
    void _init();
    void _reorderPlaylistPositions(int maxPosition=INT_MAX) const;
    QMap<int,SBIDBase> _retrievePlaylistItems();
};

#endif // SBIDPLAYLIST_H
