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
    switch(_consistOfItemType())
    {
    case SBIDBase::sb_type_online_performance:
        return onlinePerformancePtr()->commonPerformerID();

    case SBIDBase::sb_type_playlist:
        return playlistPtr()->commonPerformerID();

    case SBIDBase::sb_type_chart:
        return chartPtr()->commonPerformerID();

    case SBIDBase::sb_type_album:
        return albumPtr()->commonPerformerID();

    }
    return -1;
}

QString
SBIDPlaylistDetail::commonPerformerName() const
{
    switch(_consistOfItemType())
    {
    case SBIDBase::sb_type_online_performance:
        return onlinePerformancePtr()->commonPerformerName();

    case SBIDBase::sb_type_playlist:
        return playlistPtr()->commonPerformerName();

    case SBIDBase::sb_type_chart:
        return chartPtr()->commonPerformerName();

    case SBIDBase::sb_type_album:
        return albumPtr()->commonPerformerName();

    }
    return QString("n/a");
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
    switch(_consistOfItemType())
    {
    case SBIDBase::sb_type_online_performance:
        g+=onlinePerformancePtr()->genericDescription();

    case SBIDBase::sb_type_playlist:
        g+=playlistPtr()->genericDescription();

    case SBIDBase::sb_type_chart:
        g+=chartPtr()->genericDescription();

    case SBIDBase::sb_type_album:
        g+=albumPtr()->genericDescription();

    }
    return g;
}

QString
SBIDPlaylistDetail::iconResourceLocation() const
{
    switch(_consistOfItemType())
    {
    case SBIDBase::sb_type_online_performance:
        return onlinePerformancePtr()->iconResourceLocation();

    case SBIDBase::sb_type_playlist:
        return playlistPtr()->iconResourceLocation();

    case SBIDBase::sb_type_chart:
        return chartPtr()->iconResourceLocation();

    case SBIDBase::sb_type_album:
        return albumPtr()->iconResourceLocation();

    }
    return QString();
}

void
SBIDPlaylistDetail::sendToPlayQueue(bool enqueueFlag)
{
    switch(_consistOfItemType())
    {
    case SBIDBase::sb_type_online_performance:
        return onlinePerformancePtr()->sendToPlayQueue(enqueueFlag);

    case SBIDBase::sb_type_playlist:
        return playlistPtr()->sendToPlayQueue(enqueueFlag);

    case SBIDBase::sb_type_chart:
        return chartPtr()->sendToPlayQueue(enqueueFlag);

    case SBIDBase::sb_type_album:
        return albumPtr()->sendToPlayQueue(enqueueFlag);
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

//	Methods required by SBIDManagerTemplate
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
QString
SBIDPlaylistDetail::createKey(int playlistDetailID)
{
    const QString key= (playlistDetailID>=0)?QString("%1:%2")
        .arg(SBIDBase::sb_type_playlist_detail)
        .arg(playlistDetailID):QString("x:x")	//	Return invalid key if one or both parameters<0
    ;
    return key;
}

///	Protected
SBIDPlaylistDetail::SBIDPlaylistDetail()
{
    _init();
}

SBIDPlaylistDetailPtr
SBIDPlaylistDetail::instantiate(const QSqlRecord &r)
{
    SBIDPlaylistDetail pd;
    int i=0;

    pd._playlistDetailID   =r.value(i++).toInt();
    pd._playlistID         =r.value(i++).toInt();
    pd._playlistPosition   =r.value(i++).toInt();
    pd._onlinePerformanceID=r.value(i++).toInt();
    pd._childPlaylistID    =r.value(i++).toInt();
    pd._chartID            =r.value(i++).toInt();
    pd._albumID            =r.value(i++).toInt();
    pd._performerID        =r.value(i++).toInt();
    pd._notes              =r.value(i++).toString();

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
        .arg(key.length()==0?"":QString("WHERE r.playlist_detail_id=%1").arg(playlistDetailID))
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
