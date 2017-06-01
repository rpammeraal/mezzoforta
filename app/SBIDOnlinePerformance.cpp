#include "SBIDOnlinePerformance.h"

#include "Common.h"
#include "Context.h"
#include "SBSqlQueryModel.h"

///	Ctors, dtors
SBIDOnlinePerformance::SBIDOnlinePerformance(const SBIDOnlinePerformance& p):SBIDBase(p)
{
    _onlinePerformanceID=p._onlinePerformanceID;
    _albumPerformanceID =p._albumPerformanceID;
    _path               =p._path;

    _apPtr              =p._apPtr;

    _playPosition       =p._playPosition;
}

SBIDOnlinePerformance::~SBIDOnlinePerformance()
{
}

//	Inherited methods
int
SBIDOnlinePerformance::commonPerformerID() const
{
    SBIDAlbumPerformancePtr apPtr=albumPerformancePtr();
    if(apPtr)
    {
        return apPtr->commonPerformerID();
    }
    return -1;
}

QString
SBIDOnlinePerformance::commonPerformerName() const
{
    SBIDAlbumPerformancePtr apPtr=albumPerformancePtr();
    if(apPtr)
    {
        return apPtr->commonPerformerName();
    }
    return QString();
}

QString
SBIDOnlinePerformance::genericDescription() const
{
    return QString("Online Song - %1 by %2 on '%3'")
        .arg(this->songTitle())
        .arg(this->songPerformerName())
        .arg(this->albumTitle())
    ;
}

QString
SBIDOnlinePerformance::iconResourceLocation() const
{
    return QString("n/a");
}

int
SBIDOnlinePerformance::itemID() const
{
    return _onlinePerformanceID;
}

SBIDBase::sb_type
SBIDOnlinePerformance::itemType() const
{
    return SBIDBase::sb_type_online_performance;
}

void
SBIDOnlinePerformance::sendToPlayQueue(bool enqueueFlag)
{
    QMap<int,SBIDOnlinePerformancePtr> list;
    const SBIDOnlinePerformancePtr opPtr=SBIDOnlinePerformance::retrieveOnlinePerformance(this->_onlinePerformanceID);
    list[0]=opPtr;
    SBModelQueuedSongs* mqs=Context::instance()->getSBModelQueuedSongs();
    mqs->populate(list,enqueueFlag);
}

QString
SBIDOnlinePerformance::text() const
{
    return this->songTitle();
}

QString
SBIDOnlinePerformance::type() const
{
    return QString("online performance");
}

//	SBIDOnlinePerformance specific methods
bool
SBIDOnlinePerformance::updateLastPlayDate()
{
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());

    QString q=QString
    (
        "UPDATE "
            "___SB_SCHEMA_NAME___online_performance "
        "SET "
            "last_play_date=%1 "
        "WHERE "
            "song_id=%2 AND "
            "artist_id=%3 AND "
            "record_id=%4 AND "
            "record_position=%5 "
    )
        .arg(dal->getGetDateTime())
        .arg(this->songID())
        .arg(this->songPerformerID())
        .arg(this->albumID())
        .arg(this->albumPosition())
    ;
    dal->customize(q);
    qDebug() << SB_DEBUG_INFO << q;

    QSqlQuery query(q,db);
    query.exec();

    return 1;	//	CWIP: need proper error handling
}

//	Pointers
SBIDAlbumPerformancePtr
SBIDOnlinePerformance::albumPerformancePtr() const
{
    if(!_apPtr && _albumPerformanceID>=0)
    {
        const_cast<SBIDOnlinePerformance *>(this)->_loadAlbumPerformancePtr();
    }
    return _apPtr;
}

SBIDSongPtr
SBIDOnlinePerformance::songPtr() const
{
    SBIDAlbumPerformancePtr apPtr=albumPerformancePtr();
    SBIDSongPtr sPtr;
    if(apPtr)
    {
        sPtr=apPtr->songPtr();
    }
    return sPtr;
}


///	Redirectors
int
SBIDOnlinePerformance::albumID() const
{
    SBIDAlbumPerformancePtr apPtr=albumPerformancePtr();
    return (apPtr?apPtr->albumID():-1);
}

int
SBIDOnlinePerformance::albumPerformerID() const
{
    SBIDAlbumPerformancePtr apPtr=albumPerformancePtr();
    return (apPtr?apPtr->albumPerformerID():-1);
}

QString
SBIDOnlinePerformance::albumPerformerName() const
{
    SBIDAlbumPerformancePtr apPtr=albumPerformancePtr();
    return (apPtr?apPtr->albumPerformerName():QString("n/a"));
}

int
SBIDOnlinePerformance::albumPosition() const
{
    SBIDAlbumPerformancePtr apPtr=albumPerformancePtr();
    return (apPtr?apPtr->albumPosition():-1);
}

QString
SBIDOnlinePerformance::albumTitle() const
{
    SBIDAlbumPerformancePtr apPtr=albumPerformancePtr();
    return (apPtr?apPtr->albumTitle():QString("n/a"));
}

Duration
SBIDOnlinePerformance::duration() const
{
    //	CWIP: base duration directly of file length --or-- store duration when file is imported.
    SBIDAlbumPerformancePtr apPtr=albumPerformancePtr();
    return (apPtr?apPtr->duration():QString("n/a"));
}

int
SBIDOnlinePerformance::songID() const
{
    SBIDAlbumPerformancePtr apPtr=albumPerformancePtr();
    return (apPtr?apPtr->songID():-1);
}

int
SBIDOnlinePerformance::songPerformerID() const
{
    SBIDAlbumPerformancePtr apPtr=albumPerformancePtr();
    return (apPtr?apPtr->songPerformerID():-1);
}

QString
SBIDOnlinePerformance::songPerformerName() const
{
    SBIDAlbumPerformancePtr apPtr=albumPerformancePtr();
    return (apPtr?apPtr->songPerformerName():QString("n/a"));
}

QString
SBIDOnlinePerformance::songTitle() const
{
    SBIDAlbumPerformancePtr apPtr=albumPerformancePtr();
    return (apPtr?apPtr->songTitle():QString("n/a"));
}

///	Operators
SBIDOnlinePerformance::operator QString()
{
    //	Do not cause retrievals to be done, in case this method is being called during a retrieval.
    QString str=_apPtr?_apPtr->operator QString():QString("not retrieved yet");

    return QString("SBIdOnlinePerformance:%1")
        .arg(str)
    ;
}



//	Static methods
QString
SBIDOnlinePerformance::createKey(int onlinePerformanceID)
{
    const QString key= (onlinePerformanceID>=0)?QString("%1:%2")
        .arg(SBIDBase::sb_type_online_performance)
        .arg(onlinePerformanceID):QString("x:x")	//	Return invalid key if one or both parameters<0
    ;
    return key;

}

SBIDOnlinePerformancePtr
SBIDOnlinePerformance::retrieveOnlinePerformance(int onlinePerformanceID, bool noDependentsFlag)
{
    SBIDOnlinePerformanceMgr* opMgr=Context::instance()->getOnlinePerformanceMgr();
    SBIDOnlinePerformancePtr opPtr;

    if(onlinePerformanceID>=0)
    {
        opPtr=opMgr->retrieve(createKey(onlinePerformanceID),  (noDependentsFlag==1?SBIDManagerTemplate<SBIDOnlinePerformance,SBIDBase>::open_flag_parentonly:SBIDManagerTemplate<SBIDOnlinePerformance,SBIDBase>::open_flag_default));
    }
    return opPtr;
}


SBSqlQueryModel*
SBIDOnlinePerformance::retrieveAllOnlinePerformances(int limit)
{
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();

    QString limitClause;

    if(limit)
    {
        limitClause=QString("LIMIT %1").arg(limit);
    }

    //	Main query
    QString q=QString
    (
        "SELECT DISTINCT "
            "SB_SONG_ID, "
            "songTitle AS \"song title\", "
            "SB_PERFORMER_ID, "
            "artistName AS \"performer\", "
            "SB_ALBUM_ID, "
            "recordTitle AS \"album title\", "
            "SB_POSITION_ID, "
            "SB_PATH, "
            "duration, "
            "SB_PLAY_ORDER, "
            "SB_ONLINE_PERFORMANCE_ID "
        "FROM "
            "( "
                "SELECT "
                    "s.song_id AS SB_SONG_ID, "
                    "s.title AS songTitle, "
                    "a.artist_id AS SB_PERFORMER_ID, "
                    "a.name AS artistName, "
                    "r.record_id AS SB_ALBUM_ID, "
                    "r.title AS recordTitle, "
                    "rp.record_position AS SB_POSITION_ID, "
                    "op.path AS SB_PATH, "
                    "rp.duration, "
                    "%1(op.last_play_date,'1/1/1900') AS SB_PLAY_ORDER, "
                    "op.online_performance_id AS SB_ONLINE_PERFORMANCE_ID "
                "FROM "
                    "___SB_SCHEMA_NAME___online_performance op "
                        "JOIN ___SB_SCHEMA_NAME___record_performance rp ON "
                            "op.record_performance_id=rp.record_performance_id "
                        "JOIN ___SB_SCHEMA_NAME___record r ON  "
                            "rp.record_id=r.record_id "
                        "JOIN ___SB_SCHEMA_NAME___performance p ON "
                            "rp.performance_id=p.performance_id "
                        "JOIN ___SB_SCHEMA_NAME___artist a ON  "
                            "p.artist_id=a.artist_id "
                        "JOIN ___SB_SCHEMA_NAME___song s ON  "
                            "p.song_id=s.song_id "
            ") a "
        "ORDER BY "
            "SB_PLAY_ORDER "
        "%2 "
    )
            .arg(dal->getIsNull())
            .arg(limitClause)
    ;

    return new SBSqlQueryModel(q);
}

SBSqlQueryModel*
SBIDOnlinePerformance::performancesBySong(int songID)
{
    QString q=QString
    (
        "SELECT DISTINCT "
            "rp.record_performance_id, "
            "s.song_id, "
            "rp.record_id, "
            "rp.record_position, "
            "p.artist_id, "
            "rp.duration, "
            "p.year, "
            "rp.notes, "
            "op.path "
        "FROM "
            "___SB_SCHEMA_NAME___song s "
                "JOIN ___SB_SCHEMA_NAME___performance p ON " //	Removed LEFT. Want to get existing album performances.
                    "s.song_id=p.song_id "
                "JOIN ___SB_SCHEMA_NAME___record_performance rp ON " //	Removed LEFT. See above.
                    "p.performance_id=rp.performance_id "
                "LEFT JOIN ___SB_SCHEMA_NAME___online_performance op ON "
                    "rp.record_performance_id=op.record_performance_id "
        "WHERE s.song_id=%1 "
    )
        .arg(songID)
    ;

    qDebug() << SB_DEBUG_INFO << q;
    return new SBSqlQueryModel(q);
}

QString
SBIDOnlinePerformance::key() const
{
    return createKey(_onlinePerformanceID);
}

void
SBIDOnlinePerformance::refreshDependents(bool showProgressDialogFlag,bool forcedFlag)
{
    Q_UNUSED(showProgressDialogFlag);
    Q_UNUSED(forcedFlag);

    _loadAlbumPerformancePtr();
}

///	Protected methods
SBIDOnlinePerformance::SBIDOnlinePerformance()
{
    _init();
}

SBIDOnlinePerformancePtr
SBIDOnlinePerformance::instantiate(const QSqlRecord& r)
{
    SBIDOnlinePerformance op;
    int i=0;

    op._onlinePerformanceID=Common::parseIntFieldDB(&r,i++);
    op._albumPerformanceID =Common::parseIntFieldDB(&r,i++);

    op._path               =r.value(i++).toString();

    return std::make_shared<SBIDOnlinePerformance>(op);
}

void
SBIDOnlinePerformance::openKey(const QString &key, int& onlinePerformanceID)
{
    QStringList l=key.split(":");
    onlinePerformanceID=l.count()==2?l[1].toInt():-1;
}

void
SBIDOnlinePerformance::postInstantiate(SBIDOnlinePerformancePtr& ptr)
{
    Q_UNUSED(ptr);
}

SBSqlQueryModel*
SBIDOnlinePerformance::retrieveSQL(const QString &key)
{
    int opID=-1;
    openKey(key,opID);

    QString q=QString
    (
        "SELECT DISTINCT "
            "op.online_performance_id, "
            "op.record_performance_id, "
            "rp.duration, "
            "op.path "
        "FROM "
            "___SB_SCHEMA_NAME___online_performance op "
                "JOIN ___SB_SCHEMA_NAME___record_performance rp USING (record_performance_id) "
        "%1  "
    )
        .arg(key.length()==0?"":QString("WHERE op.online_performance_id=%1").arg(opID))
    ;

    return new SBSqlQueryModel(q);
}

QStringList
SBIDOnlinePerformance::updateSQL() const
{
    return QStringList();
}

///	Private methods
void
SBIDOnlinePerformance::_init()
{
    _onlinePerformanceID=-1;
    _albumPerformanceID=-1;
    _playPosition=-1;
}

void
SBIDOnlinePerformance::_loadAlbumPerformancePtr()
{
    _apPtr=SBIDAlbumPerformance::retrieveAlbumPerformance(_albumPerformanceID,1);
}
