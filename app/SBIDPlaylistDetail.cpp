#include "SBIDPlaylistDetail.h"

#include "CacheManager.h"
#include "Context.h"
#include "DataAccessLayer.h"
#include "SqlQuery.h"

///	Public
SBIDPlaylistDetail::SBIDPlaylistDetail(const SBIDPlaylistDetail &p):SBIDBase(p)
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
    SBIDPtr p=childPtr();
    return (p?p->commonPerformerID():-1);
}

QString
SBIDPlaylistDetail::commonPerformerName() const
{
    SBIDPtr p=childPtr();
    return (p?p->commonPerformerName():QString());
}

QString
SBIDPlaylistDetail::defaultIconResourceLocation() const
{
    SBIDPtr p=childPtr();
    return (p?p->defaultIconResourceLocation():QString());
}

QString
SBIDPlaylistDetail::genericDescription() const
{
    QString g;
    SBIDPtr p=childPtr();
    g+=(p?p->genericDescription():QString());
    return g;
}

QMap<int,SBIDOnlinePerformancePtr>
SBIDPlaylistDetail::onlinePerformances(bool updateProgressDialogFlag) const
{
    QMap<int,SBIDOnlinePerformancePtr> list;

    SBIDPtr p=childPtr();
    if(p)
    {
        list=p->onlinePerformances(updateProgressDialogFlag);
    }
    return list;
}

SBIDPtr
SBIDPlaylistDetail::retrieveItem(const SBKey& itemKey) const
{
    return this->retrievePlaylistDetail(itemKey);
}

void
SBIDPlaylistDetail::sendToPlayQueue(bool enqueueFlag)
{
    SBIDPtr p=childPtr();
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
SBKey::ItemType
SBIDPlaylistDetail::consistOfItemType() const
{
    if(_onlinePerformanceID!=-1)
    {
        return SBKey::OnlinePerformance;
    }
    else if(_childPlaylistID!=-1)
    {
        return SBKey::Playlist;
    }
    else if(_chartID!=-1)
    {
        return SBKey::Chart;
    }
    else if(_albumID!=-1)
    {
        return SBKey::Album;
    }
    else if(_performerID!=-1)
    {
        return SBKey::Performer;
    }
    return SBKey::Invalid;
}


///	Pointers
SBIDPlaylistPtr
SBIDPlaylistDetail::playlistPtr() const
{
    CacheManager* cm=Context::instance()->cacheManager();
    CachePlaylistMgr* pMgr=cm->playlistMgr();
    return pMgr->retrieve(SBIDPlaylist::createKey(_playlistID));
}

SBIDOnlinePerformancePtr
SBIDPlaylistDetail::onlinePerformancePtr() const
{
    CacheManager* cm=Context::instance()->cacheManager();
    CacheOnlinePerformanceMgr* pMgr=cm->onlinePerformanceMgr();
    return pMgr->retrieve(SBIDOnlinePerformance::createKey(_onlinePerformanceID));
}

SBIDPlaylistPtr
SBIDPlaylistDetail::childPlaylistPtr() const
{
    CacheManager* cm=Context::instance()->cacheManager();
    CachePlaylistMgr* pMgr=cm->playlistMgr();
    return pMgr->retrieve(SBIDPlaylist::createKey(_childPlaylistID));
}

SBIDChartPtr
SBIDPlaylistDetail::chartPtr() const
{
    CacheManager* cm=Context::instance()->cacheManager();
    CacheChartMgr* pMgr=cm->chartMgr();
    return pMgr->retrieve(SBIDChart::createKey(_chartID));
}

SBIDAlbumPtr
SBIDPlaylistDetail::albumPtr() const
{
    CacheManager* cm=Context::instance()->cacheManager();
    CacheAlbumMgr* pMgr=cm->albumMgr();
    return pMgr->retrieve(SBIDAlbum::createKey(_albumID));
}

SBIDPerformerPtr
SBIDPlaylistDetail::performerPtr() const
{
    CacheManager* cm=Context::instance()->cacheManager();
    CachePerformerMgr* pMgr=cm->performerMgr();
    return pMgr->retrieve(SBIDPerformer::createKey(_performerID));
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
    SBIDPtr pPtr=childPtr();
    SB_RETURN_IF_NULL(pPtr,SBKey());
    return pPtr->key();
}

//	CWIP:SBKey
SBIDPtr
SBIDPlaylistDetail::childPtr() const
{
    switch(consistOfItemType())
    {
    case SBKey::OnlinePerformance:
        return onlinePerformancePtr();

    case SBKey::Playlist:
        return childPlaylistPtr();

    case SBKey::Chart:
        return chartPtr();

    case SBKey::Album:
        return albumPtr();

    case SBKey::Performer:
        return performerPtr();

    case SBKey::AlbumPerformance:
    case SBKey::ChartPerformance:
    case SBKey::PlaylistDetail:
    case SBKey::Song:
    case SBKey::SongPerformance:
    case SBKey::Invalid:
        break;
    }
    return SBIDPtr();
}

//	Methods required by SBIDManagerTemplate
SBKey
SBIDPlaylistDetail::createKey(int playlistDetailID)
{
    return SBKey(SBKey::PlaylistDetail,playlistDetailID);
}

void
SBIDPlaylistDetail::refreshDependents(bool forcedFlag)
{
    Q_UNUSED(forcedFlag);
}

void
SBIDPlaylistDetail::setDeletedFlag()
{
    SBIDBase::setDeletedFlag();
    SBIDPtr ptr=childPtr();
    SB_RETURN_VOID_IF_NULL(ptr);
    ptr->setToReloadFlag();
}

QStringList
SBIDPlaylistDetail::updateSQL(const Common::db_change db_change) const
{
    DataAccessLayer* dal=Context::instance()->dataAccessLayer();
    QStringList SQL;

    if(deletedFlag() && db_change==Common::db_delete)
    {
        SQL.append(QString
        (
            "DELETE FROM ___SB_SCHEMA_NAME___playlist_detail "
            "WHERE playlist_detail_id=%1 "
        ).arg(this->itemID()));
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
            .arg(this->itemID())
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
SBIDPlaylistDetail::retrievePlaylistDetail(SBKey key)
{
    CacheManager* cm=Context::instance()->cacheManager();
    CachePlaylistDetailMgr* pdmgr=cm->playlistDetailMgr();
    return pdmgr->retrieve(key);
}

SBIDPlaylistDetailPtr
SBIDPlaylistDetail::retrievePlaylistDetail(int playlistDetailID)
{
    return retrievePlaylistDetail(createKey(playlistDetailID));
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
    case SBKey::OnlinePerformance:
        p.onlinePerformanceID=ptr->itemID();
        break;

    case SBKey::Playlist:
        p.childPlaylistID=ptr->itemID();
        break;

    case SBKey::Chart:
        p.chartID=ptr->itemID();
        break;

    case SBKey::Album:
        p.albumID=ptr->itemID();
        break;

    case SBKey::Performer:
        p.performerID=ptr->itemID();
        break;

    case SBKey::SongPerformance:
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

    case SBKey::AlbumPerformance:
    {
        const SBIDAlbumPerformancePtr apPtr=SBIDAlbumPerformance::retrieveAlbumPerformance(ptr->itemID());
        if(apPtr)
        {
            const SBIDOnlinePerformancePtr opPtr=apPtr->preferredOnlinePerformancePtr();
            if(opPtr)
            {
                p.onlinePerformanceID=opPtr->onlinePerformanceID();
            }
        }
        break;
    }

    case SBKey::ChartPerformance:
    {
        const SBIDChartPerformancePtr cpPtr=SBIDChartPerformance::retrieveChartPerformance(ptr->itemID());
        const SBIDSongPerformancePtr spPtr=SBIDSongPerformance::retrieveSongPerformanceByPerformerID(cpPtr->songID(),cpPtr->songPerformerID());
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
SBIDPlaylistDetail::SBIDPlaylistDetail():SBIDBase(SBKey::PlaylistDetail,-1)
{
    _init();
}

SBIDPlaylistDetail::SBIDPlaylistDetail(int playlistDetailID):SBIDBase(SBKey::PlaylistDetail,playlistDetailID)
{
    _init();
}

SBIDPlaylistDetailPtr
SBIDPlaylistDetail::createInDB(Common::sb_parameters& p)
{
    DataAccessLayer* dal=Context::instance()->dataAccessLayer();
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
    SqlQuery insert(q,db);
    Q_UNUSED(insert);

    //	Instantiate
    SBIDPlaylistDetail pd(dal->retrieveLastInsertedKey());
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
    int i=0;

    SBIDPlaylistDetail pd(Common::parseIntFieldDB(&r,i++));
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

SBSqlQueryModel*
SBIDPlaylistDetail::retrieveSQL(SBKey key)
{
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
        .arg(key.validFlag()?QString("WHERE pd.playlist_detail_id=%1").arg(key.itemID()):QString())
    ;

    return new SBSqlQueryModel(q);
}

///	Private methods
void
SBIDPlaylistDetail::_copy(const SBIDPlaylistDetail &c)
{
    SBIDBase::_copy(c);

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
    this->_playlistID         =-1;
    this->_playlistPosition   =-1;
    this->_onlinePerformanceID=-1;
    this->_childPlaylistID    =-1;
    this->_chartID            =-1;
    this->_albumID            =-1;
    this->_performerID        =-1;
    this->_notes              =QString();
}
