#include "SBIDPlaylistDetail.h"

#include "CacheManager.h"
#include "Common.h"
#include "Context.h"
#include "SBIDOnlinePerformance.h"
#include "SBMessageBox.h"
#include "SBSqlQueryModel.h"

///	Public
SBIDPlaylistDetail::SBIDPlaylistDetail(const SBIDPlaylistDetail &p)
{
    _copy(p);
}

SBIDPlaylistDetail::~SBIDPlaylistDetail()
{
}

///	Inherited methods
int
SBIDPlaylistDetail::commonPerformerID() const
{
    SBIDPtr p=ptr();
    return (p?p->commonPerformerID():-1);
}

QString
SBIDPlaylistDetail::commonPerformerName() const
{
    SBIDPtr p=ptr();
    return (p?p->commonPerformerName():QString());
}

int
SBIDPlaylistDetail::itemID() const
{
    return _playlistDetailID;
}

Common::sb_type
SBIDPlaylistDetail::itemType() const
{
    return Common::sb_type_playlist_detail;
}

QString
SBIDPlaylistDetail::genericDescription() const
{
    QString g;
    SBIDPtr p=ptr();
    g+=(p?p->genericDescription():QString());
    return g;
}

QString
SBIDPlaylistDetail::iconResourceLocation() const
{
    SBIDPtr p=ptr();
    return (p?p->iconResourceLocation():QString());
}

QMap<int,SBIDOnlinePerformancePtr>
SBIDPlaylistDetail::onlinePerformances(bool updateProgressDialogFlag) const
{
    QMap<int,SBIDOnlinePerformancePtr> list;

    SBIDPtr p=ptr();
    if(p)
    {
        list=p->onlinePerformances(updateProgressDialogFlag);
    }
    return list;
}

void
SBIDPlaylistDetail::sendToPlayQueue(bool enqueueFlag)
{
    SBIDPtr p=ptr();
    if(p)
    {
        p->sendToPlayQueue(enqueueFlag);
    }
}

QString
SBIDPlaylistDetail::text() const
{
    return this->genericDescription();
}

QString
SBIDPlaylistDetail::type() const
{
    return QString("playlistdetail");
}

///	SBIDPlaylistDetail specific methods
Common::sb_type
SBIDPlaylistDetail::consistOfItemType() const
{
    if(_onlinePerformanceID!=-1)
    {
        return Common::sb_type_online_performance;
    }
    else if(_childPlaylistID!=-1)
    {
        return Common::sb_type_playlist;
    }
    else if(_chartID!=-1)
    {
        return Common::sb_type_chart;
    }
    else if(_albumID!=-1)
    {
        return Common::sb_type_album;
    }
    else if(_performerID!=-1)
    {
        return Common::sb_type_performer;
    }
    return Common::sb_type_invalid;
}


///	Pointers
SBIDPlaylistPtr
SBIDPlaylistDetail::playlistPtr() const
{
    CacheManager* cm=Context::instance()->cacheManager();
    CachePlaylistMgr* pMgr=cm->playlistMgr();
    return pMgr->retrieve(
                SBIDPlaylist::createKey(_playlistID),
                Cache::open_flag_parentonly);
}

SBIDOnlinePerformancePtr
SBIDPlaylistDetail::onlinePerformancePtr() const
{
    CacheManager* cm=Context::instance()->cacheManager();
    CacheOnlinePerformanceMgr* pMgr=cm->onlinePerformanceMgr();
    return pMgr->retrieve(
                SBIDOnlinePerformance::createKey(_onlinePerformanceID),
                Cache::open_flag_parentonly);
}

SBIDPlaylistPtr
SBIDPlaylistDetail::childPlaylistPtr() const
{
    CacheManager* cm=Context::instance()->cacheManager();
    CachePlaylistMgr* pMgr=cm->playlistMgr();
    return pMgr->retrieve(
                SBIDPlaylist::createKey(_childPlaylistID),
                Cache::open_flag_parentonly);
}

SBIDChartPtr
SBIDPlaylistDetail::chartPtr() const
{
    CacheManager* cm=Context::instance()->cacheManager();
    CacheChartMgr* pMgr=cm->chartMgr();
    return pMgr->retrieve(
                SBIDChart::createKey(_chartID),
                Cache::open_flag_parentonly);
}

SBIDAlbumPtr
SBIDPlaylistDetail::albumPtr() const
{
    CacheManager* cm=Context::instance()->cacheManager();
    CacheAlbumMgr* pMgr=cm->albumMgr();
    return pMgr->retrieve(
                SBIDAlbum::createKey(_albumID),
                Cache::open_flag_parentonly);
}

SBIDPerformerPtr
SBIDPlaylistDetail::performerPtr() const
{
    CacheManager* cm=Context::instance()->cacheManager();
    CachePerformerMgr* pMgr=cm->performerMgr();
    return pMgr->retrieve(
                SBIDPerformer::createKey(_performerID),
                Cache::open_flag_parentonly);
}

//	Redirectors
int
SBIDPlaylistDetail::onlinePerformanceID() const
{
    SBIDOnlinePerformancePtr opPtr=onlinePerformancePtr();
    return (opPtr?opPtr->onlinePerformanceID():-1);
}

SBKey
SBIDPlaylistDetail::childKey() const
{
    SBIDPtr pPtr=ptr();
    SB_RETURN_IF_NULL(pPtr,SBKey());
    return pPtr->key();
}

//	CWIP:SBKey
SBIDPtr
SBIDPlaylistDetail::ptr() const
{
    switch(consistOfItemType())
    {
    case Common::sb_type_online_performance:
        return onlinePerformancePtr();

    case Common::sb_type_playlist:
        return childPlaylistPtr();

    case Common::sb_type_chart:
        return chartPtr();

    case Common::sb_type_album:
        return albumPtr();

    case Common::sb_type_performer:
        return performerPtr();

    case Common::sb_type_album_performance:
    case Common::sb_type_chart_performance:
    case Common::sb_type_playlist_detail:
    case Common::sb_type_song:
    case Common::sb_type_song_performance:
    case Common::sb_type_invalid:
        break;
    }
    return SBIDPtr();
}

//	Methods required by SBIDManagerTemplate
SBKey
SBIDPlaylistDetail::createKey(int playlistDetailID)
{
    return SBKey(Common::sb_type_playlist_detail,playlistDetailID);
}

void
SBIDPlaylistDetail::refreshDependents(bool showProgressDialogFlag,bool forcedFlag)
{
    Q_UNUSED(showProgressDialogFlag);
    Q_UNUSED(forcedFlag);
}

QStringList
SBIDPlaylistDetail::updateSQL(const Common::db_change db_change) const
{
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QStringList SQL;

    if(deletedFlag() && db_change==Common::db_delete)
    {
        SQL.append(QString
        (
            "DELETE FROM ___SB_SCHEMA_NAME___playlist_detail "
            "WHERE playlist_detail_id=%1 "
        ).arg(this->_playlistDetailID));
    }
    else if(changedFlag() && db_change==Common::db_update)
    {
        SQL.append(QString(
            "UPDATE ___SB_SCHEMA_NAME___playlist_detail "
            "SET "
                "playlist_id=%1, "
                "playlist_position=%2, "
                "online_performance_id=CASE WHEN %11(%3,-1)=-1 THEN NULL ELSE %3 END, "
                "child_playlist_id=CASE WHEN %11(%4,-1)=-1 THEN NULL ELSE %4 END, "
                "chart_id=CASE WHEN %11(%5,-1)=-1 THEN NULL ELSE %5 END, "
                "record_id=CASE WHEN %11(%6,-1)=-1 THEN NULL ELSE %6 END, "
                "artist_id=CASE WHEN %11(%7,-1)=-1 THEN NULL ELSE %7 END, "
                "notes='%8' "
            "WHERE "
                "playlist_detail_id=%10 "
        )
            .arg(this->_playlistID)
            .arg(this->_playlistPosition)
            .arg(this->_onlinePerformanceID)
            .arg(this->_childPlaylistID)
            .arg(this->_chartID)
            .arg(this->_albumID)
            .arg(this->_performerID)
            .arg(Common::escapeSingleQuotes(this->_notes))
            .arg(this->_playlistDetailID)
            .arg(dal->getIsNull())
        );
    }
    return SQL;

}

//	Static methods
SBSqlQueryModel*
SBIDPlaylistDetail::playlistDetailsByAlbum(int albumID)
{
    QString q=QString
    (
        "SELECT DISTINCT "
            "op.playlist_detail_id "
        "FROM "
            "___SB_SCHEMA_NAME___playlist_detail op "
        "WHERE "
            "record_id=%1 "
    )
        .arg(albumID)
    ;

    return new SBSqlQueryModel(q);
}

SBSqlQueryModel*
SBIDPlaylistDetail::playlistDetailsByPerformer(int performerID)
{
    QString q=QString
    (
        "SELECT DISTINCT "
            "op.playlist_detail_id "
        "FROM "
            "___SB_SCHEMA_NAME___playlist_detail op "
        "WHERE "
            "artist_id=%1 "
    )
        .arg(performerID)
    ;

    return new SBSqlQueryModel(q);
}

SBIDPlaylistDetailPtr
SBIDPlaylistDetail::retrievePlaylistDetail(const SBKey& key, bool noDependentsFlag)
{
    CacheManager* cm=Context::instance()->cacheManager();
    CachePlaylistDetailMgr* pdmgr=cm->playlistDetailMgr();
    return pdmgr->retrieve(key,(noDependentsFlag==1?Cache::open_flag_parentonly:Cache::open_flag_default));
}

SBIDPlaylistDetailPtr
SBIDPlaylistDetail::retrievePlaylistDetail(int playlistDetailID, bool noDependentsFlag)
{
    return retrievePlaylistDetail(createKey(playlistDetailID),noDependentsFlag);
}

SBIDPlaylistDetailPtr
SBIDPlaylistDetail::createPlaylistDetail(int playlistID, int playlistPosition, SBIDPtr ptr)
{
    SB_RETURN_IF_NULL(ptr,SBIDPlaylistDetailPtr());
    Common::sb_parameters p;
    p.playlistID=playlistID;
    p.playlistPosition=playlistPosition;

    switch(ptr->itemType())
    {
    case Common::sb_type_online_performance:
        p.onlinePerformanceID=ptr->itemID();
        break;

    case Common::sb_type_playlist:
        p.childPlaylistID=ptr->itemID();
        break;

    case Common::sb_type_chart:
        p.chartID=ptr->itemID();
        break;

    case Common::sb_type_album:
        p.albumID=ptr->itemID();
        break;

    case Common::sb_type_performer:
        p.performerID=ptr->itemID();
        break;

    case Common::sb_type_song_performance:
    {
        const SBIDSongPerformancePtr spPtr=SBIDSongPerformance::retrieveSongPerformance(ptr->itemID());
        if(spPtr)
        {
            const SBIDOnlinePerformancePtr opPtr=spPtr->preferredOnlinePerformancePtr();
            if(opPtr)
            {
                p.onlinePerformanceID=opPtr->onlinePerformanceID();
            }
        }
        break;
    }

    default:
        qDebug() << SB_DEBUG_ERROR << "Case not handled:" << ptr->itemType();
        return SBIDPlaylistDetailPtr();
        break;
    }

    CacheManager* cm=Context::instance()->cacheManager();
    CachePlaylistDetailMgr* pdMgr=cm->playlistDetailMgr();
    return pdMgr->createInDB(p);
}

///	Protected
SBIDPlaylistDetail::SBIDPlaylistDetail()
{
    _init();
}

SBIDPlaylistDetailPtr
SBIDPlaylistDetail::createInDB(Common::sb_parameters& p)
{
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
    QString q;

    //	Insert
    q=QString
    (
        "INSERT INTO ___SB_SCHEMA_NAME___playlist_detail "
        "( "
            "playlist_id, "
            "playlist_position, "
            "online_performance_id, "
            "child_playlist_id, "
            "chart_id, "
            "record_id, "
            "artist_id, "
            "timestamp, "
            "notes "
        ") "
        "VALUES "
        "( "
            "%1, "
            "%2, "
            "NULLIF(%3,-1), "
            "NULLIF(%4,-1), "
            "NULLIF(%5,-1), "
            "NULLIF(%6,-1), "
            "NULLIF(%7,-1), "
            "%8, "
            "'%9' "
        ") "
    )
        .arg(p.playlistID)
        .arg(p.playlistPosition)
        .arg(p.onlinePerformanceID)
        .arg(p.childPlaylistID)
        .arg(p.chartID)
        .arg(p.albumID)
        .arg(p.performerID)
        .arg(dal->getGetDateTime())
        .arg(p.notes)
    ;
    dal->customize(q);
    qDebug() << SB_DEBUG_INFO << q;
    QSqlQuery insert(q,db);
    Q_UNUSED(insert);

    //	Instantiate
    SBIDPlaylistDetail pd;
    pd._playlistDetailID   =dal->retrieveLastInsertedKey();
    pd._playlistID         =p.playlistID;
    pd._playlistPosition   =p.playlistPosition;
    pd._onlinePerformanceID=p.onlinePerformanceID;
    pd._childPlaylistID    =p.childPlaylistID;
    pd._chartID            =p.chartID;
    pd._albumID            =p.albumID;
    pd._performerID        =p.performerID;
    pd._notes              =p.notes;

    //	Done
    return std::make_shared<SBIDPlaylistDetail>(pd);
}

SBIDPlaylistDetailPtr
SBIDPlaylistDetail::instantiate(const QSqlRecord &r)
{
    SBIDPlaylistDetail pd;
    int i=0;

    pd._playlistDetailID   =Common::parseIntFieldDB(&r,i++);
    pd._playlistID         =Common::parseIntFieldDB(&r,i++);
    pd._playlistPosition   =Common::parseIntFieldDB(&r,i++);
    pd._onlinePerformanceID=Common::parseIntFieldDB(&r,i++);
    pd._childPlaylistID    =Common::parseIntFieldDB(&r,i++);
    pd._chartID            =Common::parseIntFieldDB(&r,i++);
    pd._albumID            =Common::parseIntFieldDB(&r,i++);
    pd._performerID        =Common::parseIntFieldDB(&r,i++);
    pd._notes              =r.value(i++).toString();

    if(pd._onlinePerformanceID!=-1)
    {
        pd._childPlaylistID=-1;
        pd._chartID=-1;
        pd._albumID=-1;
        pd._performerID=-1;
    }
    if(pd._childPlaylistID!=-1)
    {
        pd._onlinePerformanceID=-1;
        pd._chartID=-1;
        pd._albumID=-1;
        pd._performerID=-1;
    }
    if(pd._chartID!=-1)
    {
        pd._onlinePerformanceID=-1;
        pd._childPlaylistID=-1;
        pd._albumID=-1;
        pd._performerID=-1;
    }
    if(pd._albumID!=-1)
    {
        pd._onlinePerformanceID=-1;
        pd._childPlaylistID=-1;
        pd._chartID=-1;
        pd._performerID=-1;
    }
    if(pd._performerID!=-1)
    {
        pd._onlinePerformanceID=-1;
        pd._childPlaylistID=-1;
        pd._chartID=-1;
        pd._albumID=-1;
    }
    return std::make_shared<SBIDPlaylistDetail>(pd);
}

void
SBIDPlaylistDetail::openKey(const QString& key, int& playlistDetailID)
{
    QStringList l=key.split(":");
    playlistDetailID=l.count()==2?l[1].toInt():-1;
}

void
SBIDPlaylistDetail::postInstantiate(SBIDPlaylistDetailPtr &ptr)
{
    Q_UNUSED(ptr);
}

SBSqlQueryModel*
SBIDPlaylistDetail::retrieveSQL(const QString &key)
{
    int playlistDetailID=-1;
    openKey(key,playlistDetailID);
    QString q=QString
    (
        "SELECT DISTINCT "
            "pd.playlist_detail_id, "
            "pd.playlist_id, "
            "pd.playlist_position, "
            "pd.online_performance_id, "
            "pd.child_playlist_id, "
            "pd.chart_id, "
            "pd.record_id, "
            "pd.artist_id, "
            "pd.notes "
        "FROM "
            "___SB_SCHEMA_NAME___playlist_detail pd "
        "%1 "
        "LIMIT 1 "
    )
        .arg(key.length()==0?"":QString("WHERE pd.playlist_detail_id=%1").arg(playlistDetailID))
    ;

    qDebug() << SB_DEBUG_INFO << q;
    return new SBSqlQueryModel(q);
}

///	Private methods
void
SBIDPlaylistDetail::_copy(const SBIDPlaylistDetail &c)
{
    this->_playlistDetailID   =c._playlistDetailID;
    this->_playlistID         =c._playlistID;
    this->_playlistPosition   =c._playlistPosition;
    this->_onlinePerformanceID=c._onlinePerformanceID;
    this->_childPlaylistID    =c._childPlaylistID;
    this->_chartID            =c._chartID;
    this->_albumID            =c._albumID;
    this->_performerID        =c._performerID;
    this->_notes              =c._notes;
}

void
SBIDPlaylistDetail::_init()
{
    this->_playlistDetailID   =-1;
    this->_playlistID         =-1;
    this->_playlistPosition   =-1;
    this->_onlinePerformanceID=-1;
    this->_childPlaylistID    =-1;
    this->_chartID            =-1;
    this->_albumID            =-1;
    this->_performerID        =-1;
    this->_notes              =-1;
}
