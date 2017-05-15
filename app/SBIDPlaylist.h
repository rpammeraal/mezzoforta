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
    inline Duration duration() const { return _duration; }
    //SBIDSongPtr getDetailPlaylistItemSong(int playlistPosition) const;
    SBTableModel* items() const;
    int numItems() const;
    inline int playlistID() const { return _sb_playlist_id; }
    inline QString playlistName() const { return _playlistName; }
    void recalculatePlaylistDuration();
    bool removePlaylistItem(int playlistPosition);
    bool moveItem(const SBIDPtr& fromPtr, int toRow);
    //void reorderItem(const SBIDPtr fromPtr, const SBIDPtr toID) const;	//	CWIP:pmgr rewrite
    void setPlaylistID(int playlistID) { _sb_playlist_id=playlistID; }
    void setPlaylistName(const QString& playlistName) { _playlistName=playlistName; setChangedFlag(); }

    //	Operators
    virtual operator QString() const;
    SBIDPlaylist& operator=(const SBIDPlaylist& t);	//	CWIP: to be moved to protected

    //	Methods required by SBIDManagerTemplate
    virtual QString key() const;
    virtual void refreshDependents(bool showProgressDialogFlag=0,bool forcedFlag=0);

    //	Static methods
    static QString createKey(int playlistID,int unused=-1);
    static SBIDPlaylistPtr retrievePlaylist(int playlistID,bool noDependentsFlag=0,bool showProgressDialogFlag=0);

protected:
    SBIDPlaylist();
    SBIDPlaylist(int itemID);

    template <class T, class parentT> friend class SBIDManagerTemplate;
    friend class Preloader;

    //	Methods used by SBIDManager (these should all become pure virtual if not static)
    bool addDependent(SBIDPtr tobeAddedPtr);
    static SBIDPlaylistPtr createInDB();
    static SBIDPlaylistPtr instantiate(const QSqlRecord& r);
    static void openKey(const QString& key, int& albumID);
    void postInstantiate(SBIDPlaylistPtr& ptr);
    bool moveDependent(int fromPosition, int toPosition);
    bool removeDependent(int position);
    static SBSqlQueryModel* retrieveSQL(const QString& key="");
    QStringList updateSQL() const;

private:
    Duration          _duration;
    int               _sb_playlist_id;
    QString           _playlistName;

    //	Not instantiated
    int               _num_items;	//	may only be used until _items has been loaded

    QMap<int,SBIDPtr> _items;

    //	Methods
    static void _getAllItemsByPlaylistRecursive(QList<SBIDPtr>& compositesTraversed, QList<SBIDOnlinePerformancePtr>& allPerformances, SBIDPtr root, QProgressDialog* progressDialog=NULL);
    void _init();
    void _reorderPlaylistPositions(int maxPosition=INT_MAX) const;
    static QMap<int,SBIDOnlinePerformancePtr> _retrievePlaylistItems(int playlistID,bool showProgressDialogFlag=0);

    QStringList _generateSQLdeleteItem(int playlistPosition) const;
    QStringList _generateSQLinsertItem(const SBIDPtr itemPtr, int playlistPositionDB) const;
    QStringList _generateSQLmoveItem(int fromPlaylistPositionDB, int toPlaylistPosition) const;

};

#endif // SBIDPLAYLIST_H
