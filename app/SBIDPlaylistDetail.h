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
    virtual int itemID() const;
    virtual SBIDBase::sb_type itemType() const;
    virtual QString genericDescription() const;
    virtual QString iconResourceLocation() const;
    virtual QMap<int,SBIDOnlinePerformancePtr> onlinePerformances(bool updateProgressDialogFlag=0) const;
    virtual void sendToPlayQueue(bool enqueueFlag=0);
    virtual QString text() const;
    virtual QString type() const;

    //	SBIDPlaylistDetail specific methods
    virtual SBIDBase::sb_type consistOfItemType() const;
    int playlistPosition() const { return _playlistPosition; }
    void setPlaylistPosition(int i)
    {
        if (i!=_playlistPosition)
        {
            _playlistPosition=i;
            setChangedFlag();
        }
    }

    //	Pointers
    SBIDPlaylistPtr playlistPtr() const;
    SBIDOnlinePerformancePtr onlinePerformancePtr() const;
    SBIDPlaylistPtr childPlaylistPtr() const;
    SBIDChartPtr chartPtr() const;
    SBIDAlbumPtr albumPtr() const;
    SBIDPerformerPtr performerPtr() const;

    //	Redirectors
    int onlinePerformanceID() const;
    QString childKey() const;
    SBIDPtr ptr() const;

    //	Methods required by SBIDBase
    static QString createKey(int playlistDetailID);
    virtual QString key() const;
    virtual void refreshDependents(bool showProgressDialogFlag=0,bool forcedFlag=0);

    //	Static methods
    static SBSqlQueryModel* playlistDetailsByAlbum(int albumID);
    static SBSqlQueryModel* playlistDetailsByPerformer(int performerID);
    static SBIDPlaylistDetailPtr retrievePlaylistDetail(int playlistDetailID,bool noDependentsFlag=1);
    static SBIDPlaylistDetailPtr createPlaylistDetail(int playlistID, int playlistPosition, SBIDPtr ptr);

protected:
    template <class T, class parentT> friend class CacheTemplate;
    friend class Preloader;

    SBIDPlaylistDetail();

    //	Methods used by SBIDManager
    static SBIDPlaylistDetailPtr createInDB(Common::sb_parameters& p);
    static SBIDPlaylistDetailPtr instantiate(const QSqlRecord& r);
    static void openKey(const QString& key, int& playlistDetailID);
    void postInstantiate(SBIDPlaylistDetailPtr& ptr);
    static SBSqlQueryModel* retrieveSQL(const QString& key);
    virtual void setPrimaryKey(int PK) { _playlistDetailID=PK;  }
    QStringList updateSQL(const Common::db_change db_change) const;

    friend class SBIDAlbum;
    void setAlbumID(int albumID) { if(_albumID!=albumID) { _albumID=albumID; setChangedFlag(); } }

    friend class SBIDPerformer;
    void setPerformerID(int performerID) { if(_performerID!=performerID) { _performerID=performerID; setChangedFlag(); } }

private:
    int     _playlistDetailID;
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
