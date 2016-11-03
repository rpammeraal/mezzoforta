#ifndef SBIDPLAYLIST_H
#define SBIDPLAYLIST_H

#include <QSqlRecord>

#include "SBIDBase.h"

class SBIDPlaylist;
typedef std::shared_ptr<SBIDPlaylist> SBIDPlaylistPtr;

#include "SBIDSong.h"

class SBIDPlaylist : public SBIDBase
{
public:
    //	Ctors, dtors
    SBIDPlaylist(const SBIDPlaylist& c);
    ~SBIDPlaylist();

    //	Public methods
    virtual SBSqlQueryModel* findMatches(const QString& name) const;
    virtual QString genericDescription() const;
    virtual QString hash() const;
    static QString iconResourceLocation();
    virtual int itemID() const;
    virtual sb_type itemType() const;
    virtual void sendToPlayQueue(bool enqueueFlag=0);
    virtual void setText(const QString &text);
    virtual QString text() const;
    virtual QString type() const;

    //	Methods specific to SBIDPlaylist
    void assignPlaylistItem(const SBIDPtr& ptr) const;
    static SBIDPlaylist createNewPlaylistDB();
    void deletePlaylistItem(SBIDBase::sb_type itemType, int playlistPosition) const;
    SBSqlQueryModel* getAllItemsByPlaylist() const;
    SBIDSongPtr getDetailPlaylistItemSong(int playlistPosition) const;
    static void recalculatePlaylistDuration(const SBIDPtr& ptr);
    static void recalculateAllPlaylistDurations();
    //void renamePlaylist();
    void reorderItem(const SBIDPtr& fromPtr, int row) const;
    void reorderItem(const SBIDBase& fromID, const SBIDBase& toID) const;
    void setCount1(const int count1) { _count1=count1;  }
    void setDuration(const Duration& duration) { _duration=duration;  }
    void setPlaylistID(int playlistID) { _sb_playlist_id=playlistID; }
    void setPlaylistName(const QString& playlistName) { _playlistName=playlistName; setChangedFlag(); }

    //	Operators
    virtual bool operator==(const SBIDBase& i) const;
    virtual operator QString() const;
    SBIDPlaylist& operator=(const SBIDPlaylist& t);	//	CWIP: to be moved to protected

protected:
    SBIDPlaylist();
    SBIDPlaylist(int itemID);

    template <class T> friend class SBIDManagerTemplate;

    //	Methods used by SBIDManager (these should all become pure virtual if not static)
    static SBIDPlaylistPtr createInDB();
    static SBIDPlaylistPtr instantiate(const QSqlRecord& r,bool noDependentsFlag=0);
    void postInstantiate(SBIDPlaylistPtr& ptr);
    static SBSqlQueryModel* retrieveSQL(int itemID=-1);
    QStringList updateSQL() const;

private:
    static void _getAllItemsByPlaylistRecursive(QList<SBIDBase>& compositesTraversed, QList<SBIDPerformancePtr>& allPerformances, const SBIDPtr& root);
    void _init();
    void _reorderPlaylistPositions(int maxPosition=INT_MAX) const;
    QMap<int,SBIDPerformancePtr> _retrievePlaylistItems();
};

#endif // SBIDPLAYLIST_H
