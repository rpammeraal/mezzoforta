#ifndef SBIDPLAYLISTDETAIL_H
#define SBIDPLAYLISTDETAIL_H

#include "SBIDBase.h"

class SBIDPlaylistDetail;
typedef std::shared_ptr<SBIDPlaylistDetail> SBIDPlaylistDetailPtr;

#include "SBIDPlaylist.h"


class SBIDPlaylistDetail : public SBIDBase
{
public:
    SBIDPlaylistDetail(const SBIDPlaylistDetail& p);
    ~SBIDPlaylistDetail();

    //	Inherited methods
    virtual int commonPerformerID() const;
    virtual QString commonPerformerName() const;
    virtual QString genericDescription() const;
    virtual QString iconResourceLocation() const;
    virtual QMap<int,SBIDOnlinePerformancePtr> onlinePerformances(bool updateProgressDialogFlag=0) const;
    virtual void sendToPlayQueue(bool enqueueFlag=0);
    virtual QString text() const;
    virtual QString type() const;

    //	SBIDPlaylistDetail specific methods
    virtual SBKey::ItemType consistOfItemType() const;
    int playlistPosition() const { return _playlistPosition; }
    void setPlaylistPosition(int i) { if (i!=_playlistPosition) { _playlistPosition=i; setChangedFlag(); } }

    //	Pointers
    SBIDPlaylistPtr playlistPtr() const;
    SBIDOnlinePerformancePtr onlinePerformancePtr() const;
    SBIDPlaylistPtr childPlaylistPtr() const;
    SBIDChartPtr chartPtr() const;
    SBIDAlbumPtr albumPtr() const;
    SBIDPerformerPtr performerPtr() const;

    //	Redirectors
    int onlinePerformanceID() const;
    SBKey childKey() const;
    SBIDPtr childPtr() const;

    //	Methods required by SBIDBase
    static SBKey createKey(int playlistDetailID);
    virtual void refreshDependents(bool forcedFlag=0);

    //	Static methods
    static SBSqlQueryModel* playlistDetailsByAlbum(int albumID);
    static SBSqlQueryModel* playlistDetailsByPerformer(int performerID);
    static SBIDPlaylistDetailPtr retrievePlaylistDetail(int playlistDetailID);
    static SBIDPlaylistDetailPtr retrievePlaylistDetail(SBKey key);
    static SBIDPlaylistDetailPtr createPlaylistDetail(int playlistID, int playlistPosition, SBIDPtr childPtr);

    //	Helper methods for CacheTemplate
    //static ItemType classType() { return PlaylistDetail; }

protected:
    template <class T, class parentT> friend class CacheTemplate;
    friend class Preloader;
    friend class SBIDPlaylist;

    SBIDPlaylistDetail();
    SBIDPlaylistDetail(int playlistDetailID);

    //	Methods used by SBIDManager
    static SBIDPlaylistDetailPtr createInDB(Common::sb_parameters& p);
    static SBIDPlaylistDetailPtr instantiate(const QSqlRecord& r);
    static SBSqlQueryModel* retrieveSQL(SBKey key=SBKey());
    virtual void setDeletedFlag();
    QStringList updateSQL(const Common::db_change db_change) const;

    friend class SBIDAlbum;
    void setAlbumID(int albumID) { if(_albumID!=albumID) { _albumID=albumID; setChangedFlag(); } }

    friend class SBIDPerformer;
    void setPerformerID(int performerID) { if(_performerID!=performerID) { _performerID=performerID; setChangedFlag(); } }

private:
    int     _playlistID;
    int     _playlistPosition;
    int     _onlinePerformanceID;
    int     _childPlaylistID;
    int     _chartID;
    int     _albumID;
    int     _performerID;
    QString _notes;

    void _copy(const SBIDPlaylistDetail& c);
    void _init();
};

#endif // SBIDPLAYLISTDETAIL_H
