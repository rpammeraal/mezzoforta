#include "SBIDPlaylistDetail.h"

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
    SBIDBasePtr p=ptr();
    return (p?p->commonPerformerID():-1);
}

QString
SBIDPlaylistDetail::commonPerformerName() const
{
    SBIDBasePtr p=ptr();
    return (p?p->commonPerformerName():QString());
}

int
SBIDPlaylistDetail::itemID() const
{
    return _playlistDetailID;
}

SBIDBase::sb_type
SBIDPlaylistDetail::itemType() const
{
    return SBIDBase::sb_type_playlist_detail;
}

QString
SBIDPlaylistDetail::genericDescription() const
{
    QString g;
    SBIDBasePtr p=ptr();
    g+=(p?p->genericDescription():QString());
    return g;
}

QString
SBIDPlaylistDetail::iconResourceLocation() const
{
    SBIDBasePtr p=ptr();
    return (p?p->iconResourceLocation():QString());
}

QMap<int,SBIDOnlinePerformancePtr>
SBIDPlaylistDetail::onlinePerformances(bool updateProgressDialogFlag) const
{
    QMap<int,SBIDOnlinePerformancePtr> list;

    SBIDBasePtr p=ptr();
    if(p)
    {
        list=p->onlinePerformances(updateProgressDialogFlag);
    }
    return list;
}

void
SBIDPlaylistDetail::sendToPlayQueue(bool enqueueFlag)
{
    SBIDBasePtr p=ptr();
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

///	Pointers
SBIDPlaylistPtr
SBIDPlaylistDetail::playlistPtr() const
{
    SBIDPlaylistMgr* pMgr=Context::instance()->getPlaylistMgr();
    return pMgr->retrieve(
                SBIDPlaylist::createKey(_playlistID),
                SBIDManagerTemplate<SBIDPlaylist,SBIDBase>::open_flag_parentonly);
}

SBIDOnlinePerformancePtr
SBIDPlaylistDetail::onlinePerformancePtr() const
{
    SBIDOnlinePerformanceMgr* pMgr=Context::instance()->getOnlinePerformanceMgr();
    return pMgr->retrieve(
                SBIDOnlinePerformance::createKey(_onlinePerformanceID),
                SBIDManagerTemplate<SBIDOnlinePerformance,SBIDBase>::open_flag_parentonly);
}

SBIDPlaylistPtr
SBIDPlaylistDetail::childPlaylistPtr() const
{
    SBIDPlaylistMgr* pMgr=Context::instance()->getPlaylistMgr();
    return pMgr->retrieve(
                SBIDPlaylist::createKey(_childPlaylistID),
                SBIDManagerTemplate<SBIDPlaylist,SBIDBase>::open_flag_parentonly);
}

SBIDChartPtr
SBIDPlaylistDetail::chartPtr() const
{
    SBIDChartMgr* pMgr=Context::instance()->getChartMgr();
    return pMgr->retrieve(
                SBIDChart::createKey(_chartID),
                SBIDManagerTemplate<SBIDChart,SBIDBase>::open_flag_parentonly);
}

SBIDAlbumPtr
SBIDPlaylistDetail::albumPtr() const
{
    SBIDAlbumMgr* pMgr=Context::instance()->getAlbumMgr();
    return pMgr->retrieve(
                SBIDAlbum::createKey(_albumID),
                SBIDManagerTemplate<SBIDAlbum,SBIDBase>::open_flag_parentonly);
}

SBIDPerformerPtr
SBIDPlaylistDetail::performerPtr() const
{
    SBIDPerformerMgr* pMgr=Context::instance()->getPerformerMgr();
    return pMgr->retrieve(
                SBIDPerformer::createKey(_performerID),
                SBIDManagerTemplate<SBIDPerformer,SBIDBase>::open_flag_parentonly);
}

//	Redirectors
int
SBIDPlaylistDetail::onlinePerformanceID() const
{
    SBIDOnlinePerformancePtr opPtr=onlinePerformancePtr();
    return (opPtr?opPtr->onlinePerformanceID():-1);
}

QString
SBIDPlaylistDetail::childKey() const
{
    SBIDPtr p=ptr();
    return (p?p->key():QString());
}

SBIDPtr
SBIDPlaylistDetail::ptr() const
{
    switch(_consistOfItemType())
    {
    case SBIDBase::sb_type_online_performance:
        return onlinePerformancePtr();

    case SBIDBase::sb_type_playlist:
        return playlistPtr();

    case SBIDBase::sb_type_chart:
        return chartPtr();

    case SBIDBase::sb_type_album:
        return albumPtr();

    case SBIDBase::sb_type_performer:
        return performerPtr();

    case SBIDBase::sb_type_album_performance:
    case SBIDBase::sb_type_chart_performance:
    case SBIDBase::sb_type_playlist_detail:
    case SBIDBase::sb_type_song:
    case SBIDBase::sb_type_song_performance:
    case SBIDBase::sb_type_invalid:
        break;
    }
    return SBIDPtr();
}

//	Methods required by SBIDManagerTemplate
QString
SBIDPlaylistDetail::createKey(int playlistDetailID)
{
    const QString key= (playlistDetailID>=0)?QString("%1:%2")
        .arg(SBIDBase::sb_type_playlist_detail)
        .arg(playlistDetailID):QString("x:x")	//	Return invalid key if one or both parameters<0
    ;
    return key;
}

QString
SBIDPlaylistDetail::key() const
{
    return createKey(_playlistDetailID);
}

void
SBIDPlaylistDetail::refreshDependents(bool showProgressDialogFlag,bool forcedFlag)
{
    Q_UNUSED(showProgressDialogFlag);
    Q_UNUSED(forcedFlag);
}

QStringList
SBIDPlaylistDetail::updateSQL() const
{
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QStringList SQL;

    qDebug() << SB_DEBUG_INFO
             << deletedFlag()
             << mergedFlag()
             << changedFlag()
    ;

    if(deletedFlag())
    {
        SQL.append(QString
        (
            "DELETE FROM ___SB_SCHEMA_NAME___playlist_detail "
            "WHERE playlist_detail_id=%1 "
        ).arg(this->_playlistDetailID));
    }
    else if(!mergedFlag() && !deletedFlag() && changedFlag())
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

    if(SQL.count()==0)
    {
        SBMessageBox::standardWarningBox("__FILE__ __LINE__ No SQL generated.");
    }
    qDebug() << SB_DEBUG_INFO << SQL;

    return SQL;

}

//	Static methods
SBIDPlaylistDetailPtr
SBIDPlaylistDetail::retrievePlaylistDetail(int playlistDetailID, bool noDependentsFlag)
{
    SBIDPlaylistDetailMgr* pdmgr=Context::instance()->getPlaylistDetailMgr();
    SBIDPlaylistDetailPtr pdPtr;
    if(playlistDetailID>=0)
    {
    pdPtr=pdmgr->retrieve(createKey(playlistDetailID),(noDependentsFlag==1?SBIDManagerTemplate<SBIDPlaylistDetail,SBIDBase>::open_flag_parentonly:SBIDManagerTemplate<SBIDPlaylistDetail,SBIDBase>::open_flag_default));
    }
    return pdPtr;
}

SBIDPlaylistDetailPtr
SBIDPlaylistDetail::createPlaylistDetail(int playlistID, int playlistPosition, SBIDPtr ptr)
{
    Common::sb_parameters p;
    p.playlistID=playlistID;
    p.playlistPosition=playlistPosition;

    switch(ptr->itemType())
    {
    case SBIDBase::sb_type_online_performance:
        p.onlinePerformanceID=ptr->itemID();
        break;

    case SBIDBase::sb_type_playlist:
        p.childPlaylistID=ptr->itemID();
        break;

    case SBIDBase::sb_type_chart:
        p.chartID=ptr->itemID();
        break;

    case SBIDBase::sb_type_album:
        p.albumID=ptr->itemID();
        break;

    case SBIDBase::sb_type_performer:
        p.performerID=ptr->itemID();
        break;

    case SBIDBase::sb_type_album_performance:
    case SBIDBase::sb_type_chart_performance:
    case SBIDBase::sb_type_playlist_detail:
    case SBIDBase::sb_type_song:
    case SBIDBase::sb_type_song_performance:
    case SBIDBase::sb_type_invalid:
        break;
    }

    SBIDPlaylistDetailPtr pdPtr=SBIDPlaylistDetail::createInDB(p);

    return pdPtr;
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

    qDebug() << SB_DEBUG_INFO << "BEFORE"
             << "pos" << pd._playlistPosition
             << "opID=" << pd._onlinePerformanceID
             << "cpID=" << pd._childPlaylistID
             << "cID=" << pd._chartID
             << "aID=" << pd._albumID
             << "pID=" << pd._performerID
	;

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
    qDebug() << SB_DEBUG_INFO << "AFTER"
             << "pos" << pd._playlistPosition
             << "opID=" << pd._onlinePerformanceID
             << "cpID=" << pd._childPlaylistID
             << "cID=" << pd._chartID
             << "aID=" << pd._albumID
             << "pID=" << pd._performerID
    ;
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
SBIDBase::sb_type
SBIDPlaylistDetail::_consistOfItemType() const
{
    if(_onlinePerformanceID!=-1)
    {
        return SBIDBase::sb_type_online_performance;
    }
    else if(_childPlaylistID!=-1)
    {
        return SBIDBase::sb_type_playlist;
    }
    else if(_chartID!=-1)
    {
        return SBIDBase::sb_type_chart;
    }
    else if(_albumID!=-1)
    {
        return SBIDBase::sb_type_album;
    }
    else if(_performerID!=-1)
    {
        return SBIDBase::sb_type_performer;
    }
    return SBIDBase::sb_type_invalid;
}

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
