#ifndef SBIDPLAYLIST_H
#define SBIDPLAYLIST_H

#include <QSqlRecord>

#include "SBIDBase.h"

class SBIDPlaylist;
typedef std::shared_ptr<SBIDPlaylist> SBIDPlaylistPtr;

#include "SBIDSong.h"

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
    virtual int itemID() const;
    virtual sb_type itemType() const;
    virtual void sendToPlayQueue(bool enqueueFlag=0);
    virtual QString text() const;
    virtual QString type() const;

    //	Methods specific to SBIDPlaylist
    bool addPlaylistItem(SBIDPtr ptr);
    inline SBDuration duration() const { return _duration; }
    SBTableModel* items() const;
    int numItems() const;
    inline int playlistID() const { return _playlistID; }
    inline QString playlistName() const { return _playlistName; }
    void recalculatePlaylistDuration();
    bool removePlaylistItem(int playlistPosition);
    bool moveItem(const SBIDPtr& fromPtr, int toRow);
    //void reorderItem(const SBIDPtr fromPtr, const SBIDPtr toID) const;	//	CWIP:pmgr rewrite
    void setPlaylistID(int playlistID) { _playlistID=playlistID; }
    void setPlaylistName(const QString& playlistName) { _playlistName=playlistName; setChangedFlag(); }

    //	Operators
    virtual operator QString() const;

    //	Methods required by SBIDManagerTemplate
    virtual QString key() const;
    virtual void refreshDependents(bool showProgressDialogFlag=0,bool forcedFlag=0);

    //	Static methods
    static QString createKey(int playlistID,int unused=-1);
    static SBIDPlaylistPtr retrievePlaylist(int playlistID,bool noDependentsFlag=1);

protected:
    template <class T, class parentT> friend class SBIDManagerTemplate;
    friend class Preloader;

    SBIDPlaylist();
    SBIDPlaylist(int itemID);

    //	Operators
    SBIDPlaylist& operator=(const SBIDPlaylist& t);

    //	Methods used by SBIDManager (these should all become pure virtual if not static)
    bool addDependent(SBIDPtr tobeAddedPtr);
    static SBIDPlaylistPtr createInDB();
    static SBIDPlaylistPtr instantiate(const QSqlRecord& r);
    static void openKey(const QString& key, int& albumID);
    void postInstantiate(SBIDPlaylistPtr& ptr);
    bool moveDependent(int fromPosition, int toPosition);
    bool removeDependent(int position);
    static SBSqlQueryModel* retrieveSQL(const QString& key="");
    virtual void setPrimaryKey(int PK) { _playlistID=PK;  }
    QStringList updateSQL() const;

private:
    int               _playlistID;
    QString           _playlistName;
    SBDuration          _duration;

    //	Not instantiated
    int               _numItems;	//	may only be used until _items has been loaded

    QMap<int,SBIDPtr> _items;

    //	Methods
    static void _getAllItemsByPlaylistRecursive(QList<SBIDPtr>& compositesTraversed, QList<SBIDOnlinePerformancePtr>& allPerformances, SBIDPtr root);
    void _copy(const SBIDPlaylist& c);
    void _init();
    void _loadPlaylistItems();
    QMap<int,SBIDPtr> _loadPlaylistItemsFromDB() const;
    void _reorderPlaylistPositions(int maxPosition=INT_MAX) const;
    static QMap<int,SBIDOnlinePerformancePtr> _retrievePlaylistItems(int playlistID);

    QStringList _generateSQLdeleteItem(int playlistPosition) const;
    QStringList _generateSQLinsertItem(const SBIDPtr itemPtr, int playlistPositionDB) const;
    QStringList _generateSQLmoveItem(int fromPlaylistPositionDB, int toPlaylistPosition) const;

};

#endif // SBIDPLAYLIST_H
