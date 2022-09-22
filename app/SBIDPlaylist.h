#ifndef SBIDPLAYLIST_H
#define SBIDPLAYLIST_H

#include <QSqlRecord>

#include "SBIDBase.h"

class SBIDPlaylist;
typedef std::shared_ptr<SBIDPlaylist> SBIDPlaylistPtr;

#include "SBIDPlaylistDetail.h"

class SBTableModel;
class QProgressDialog;

class SBIDPlaylist : public SBIDBase
{
public:
    //	Ctors, dtors
    SBIDPlaylist(const SBIDPlaylist& c);
    ~SBIDPlaylist();

    //	Public methods
    virtual int commonPerformerID() const;
    virtual QString commonPerformerName() const;
    virtual QString genericDescription() const;
    virtual QString iconResourceLocation() const;
    virtual SBKey::ItemType itemType() const;
    virtual QMap<int,SBIDOnlinePerformancePtr> onlinePerformances(bool updateProgressDialogFlag=0) const;
    virtual void sendToPlayQueue(bool enqueueFlag=0);
    virtual QString text() const;
    virtual QString type() const;

    //	Methods specific to SBIDPlaylist
    bool addPlaylistItem(SBIDPtr ptr);
    inline SBDuration duration() const { return _duration; }
    QMap<int,SBIDPlaylistDetailPtr> items() const;
    int numItems() const;
    inline int playlistID() const { return itemID(); }
    inline QString playlistName() const { return _playlistName; }
    void recalculatePlaylistDuration();
    bool removePlaylistItem(int playlistPosition);
    bool moveItem(SBKey from, int toRow);
    void setPlaylistName(const QString& playlistName) { _playlistName=playlistName; setChangedFlag(); }
    SBTableModel* tableModelItems() const;

    //	Operators
    virtual operator QString() const;

    //	Methods required by CacheTemplate
    virtual void refreshDependents(bool forcedFlag=0);

    //	Static methods
    static SBKey createKey(int playlistID);
    static SBIDPlaylistPtr retrievePlaylist(int playlistID);
    static SBIDPlaylistPtr retrievePlaylist(SBKey key);
    static void removePlaylistItemFromAllPlaylistsByKey(SBKey key);

    //	Helper methods for CacheTemplate
    //static ItemType classType() { return Playlist; }

protected:
    template <class T, class parentT> friend class CacheTemplate;
    friend class Preloader;

    SBIDPlaylist();
    SBIDPlaylist(int playlistID);

    //	Operators
    SBIDPlaylist& operator=(const SBIDPlaylist& t);

    //	Methods used by SBIDManager (these should all become pure virtual if not static)
    static SBIDPlaylistPtr createInDB(Common::sb_parameters& p);
    static SBIDPlaylistPtr instantiate(const QSqlRecord& r);
    bool moveDependent(int fromPosition, int toPosition);
    static SBSqlQueryModel* retrieveSQL(SBKey key=SBKey());
    virtual void setDeletedFlag();
    QStringList updateSQL(const Common::db_change db_change) const;

private:
    QString           _playlistName;
    SBDuration        _duration;

    //	Not instantiated
    int               _numItems;	//	may only be used until _items has been loaded

    QMap<int,SBIDPlaylistDetailPtr> _items;

    //	Methods
    static void _getOnlineItemsByPlaylist(QList<SBIDPtr>& compositesTraversed, QList<SBIDOnlinePerformancePtr>& allPerformances, const SBIDPlaylistPtr& rootPlPtr, const QString& progressDialogTitle=QString());
    void _copy(const SBIDPlaylist& c);
    SBIDPlaylistDetailPtr _findItemByKey(SBKey key);
    void _init();
    void _loadPlaylistItems();
    QMap<int,SBIDPlaylistDetailPtr> _loadPlaylistItemsFromDB() const;
    void _reorderPlaylistPositions(int maxPosition=INT_MAX) const;

};

#endif // SBIDPLAYLIST_H
