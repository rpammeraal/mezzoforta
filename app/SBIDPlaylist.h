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
    virtual int commonPerformerID() const;
    virtual QString commonPerformerName() const;
//    virtual SBSqlQueryModel* findMatches(const QString& name) const;
    virtual QString genericDescription() const;
    virtual QString iconResourceLocation() const;
    virtual int itemID() const;
    virtual sb_type itemType() const;
    virtual void sendToPlayQueue(bool enqueueFlag=0);
    virtual QString text() const;
    virtual QString type() const;

    //	Methods specific to SBIDPlaylist
    void assignPlaylistItem(SBIDPtr ptr) ;	//	CWIP:pmgr rewrite
    void deletePlaylistItem(SBIDBase::sb_type itemType, int playlistPosition) const;	//	CWIP:pmgr rewrite
    inline Duration duration() const { return _duration; }
    SBSqlQueryModel* getAllItemsByPlaylist() const;
    SBIDSongPtr getDetailPlaylistItemSong(int playlistPosition) const;
    inline int numItems() const { return _num_items; }
    inline int playlistID() const { return _sb_playlist_id; }
    inline QString playlistName() const { return _playlistName; }
    void recalculatePlaylistDuration();
    static void recalculateAllPlaylistDurations();
    void reorderItem(const SBIDPtr& fromPtr, int row) const;	//	CWIP:pmgr rewrite
    void reorderItem(const SBIDPtr fromPtr, const SBIDPtr toID) const;	//	CWIP:pmgr rewrite
    //void setDuration(const Duration& duration) { _duration=duration;  }
    void setPlaylistID(int playlistID) { _sb_playlist_id=playlistID; }
    void setPlaylistName(const QString& playlistName) { _playlistName=playlistName; setChangedFlag(); }

    //	Operators
    virtual operator QString() const;
    SBIDPlaylist& operator=(const SBIDPlaylist& t);	//	CWIP: to be moved to protected

    //	Methods required by SBIDManagerTemplate
    QString key() const;

    //	Static methods
    static QString createKey(int playlistID,int unused=-1);
    static SBIDPlaylistPtr retrievePlaylist(int playlistID,bool noDependentsFlag=0);

protected:
    SBIDPlaylist();
    SBIDPlaylist(int itemID);

    template <class T> friend class SBIDManagerTemplate;

    //	Methods used by SBIDManager (these should all become pure virtual if not static)
    static SBIDPlaylistPtr createInDB();
    static SBIDPlaylistPtr instantiate(const QSqlRecord& r,bool noDependentsFlag=0);
    static void openKey(const QString& key, int& albumID);
    void postInstantiate(SBIDPlaylistPtr& ptr);
    static SBSqlQueryModel* retrieveSQL(const QString& key="");
    QStringList updateSQL() const;

private:
    Duration _duration;
    int      _sb_playlist_id;
    QString  _playlistName;

    //	Not instantiated
    int      _num_items;

    //	Methods
    static void _getAllItemsByPlaylistRecursive(QList<SBIDPtr>& compositesTraversed, QList<SBIDPerformancePtr>& allPerformances, SBIDPtr root);
    void _init();
    void _reorderPlaylistPositions(int maxPosition=INT_MAX) const;
    static QMap<int,SBIDPerformancePtr> _retrievePlaylistItems(int playlistID);
};

#endif // SBIDPLAYLIST_H
