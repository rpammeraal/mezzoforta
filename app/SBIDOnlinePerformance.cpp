#include "SBIDOnlinePerformance.h"

#include "CacheManager.h"
#include "Context.h"
#include "DataAccessLayer.h"
#include "SBModelQueuedSongs.h"

///	Ctors, dtors
SBIDOnlinePerformance::SBIDOnlinePerformance(const SBIDOnlinePerformance& p):SBIDBase(p)
{
    _copy(p);
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

SBKey::ItemType
SBIDOnlinePerformance::itemType() const
{
    return SBKey::OnlinePerformance;
}

QMap<int,SBIDOnlinePerformancePtr>
SBIDOnlinePerformance::onlinePerformances(bool updateProgressDialogFlag) const
{
    Q_UNUSED(updateProgressDialogFlag);
    QMap<int,SBIDOnlinePerformancePtr> list;
    const SBIDOnlinePerformancePtr opPtr=SBIDOnlinePerformance::retrieveOnlinePerformance(this->itemID());
    if(opPtr)
    {
        list[0]=opPtr;
    }
    return list;
}

void
SBIDOnlinePerformance::sendToPlayQueue(bool enqueueFlag)
{
    QMap<int,SBIDOnlinePerformancePtr> list=onlinePerformances();
    SBModelQueuedSongs* mqs=Context::instance()->sbModelQueuedSongs();
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
    DataAccessLayer* dal=Context::instance()->dataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());

    QString q=QString
    (
        "UPDATE "
            "___SB_SCHEMA_NAME___online_performance "
        "SET "
            "last_play_date=%1 "
        "WHERE "
            "online_performance_id=%2 "
    )
        .arg(dal->getGetDateTime())
        .arg(this->onlinePerformanceID())
    ;
    dal->customize(q);
    qDebug() << SB_DEBUG_INFO << q;

    QSqlQuery query(q,db);
    query.exec();

    return 1;	//	CWIP: need proper error handling
}

//	Pointers
SBIDAlbumPtr
SBIDOnlinePerformance::albumPtr() const
{
    SBIDAlbumPerformancePtr apPtr=albumPerformancePtr();
    return (apPtr?apPtr->albumPtr():SBIDAlbumPtr());
}

SBIDAlbumPerformancePtr
SBIDOnlinePerformance::albumPerformancePtr() const
{
    CacheManager* cm=Context::instance()->cacheManager();
    CacheAlbumPerformanceMgr* apMgr=cm->albumPerformanceMgr();
    return apMgr->retrieve(SBIDAlbumPerformance::createKey(_albumPerformanceID));
}

SBIDSongPtr
SBIDOnlinePerformance::songPtr() const
{
    SBIDAlbumPerformancePtr apPtr=albumPerformancePtr();
    return (apPtr?apPtr->songPtr():SBIDSongPtr());
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

SBKey
SBIDOnlinePerformance::albumKey() const
{
    SBIDAlbumPerformancePtr apPtr=albumPerformancePtr();
    SB_RETURN_IF_NULL(apPtr,SBKey());
    return apPtr->albumKey();
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

SBDuration
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
SBIDOnlinePerformance::songPerformanceID() const
{
    SBIDAlbumPerformancePtr apPtr=albumPerformancePtr();
    return (apPtr?apPtr->songPerformanceID():-1);
}

SBKey
SBIDOnlinePerformance::songPerformerKey() const
{
    SBIDAlbumPerformancePtr apPtr=albumPerformancePtr();
    SB_RETURN_IF_NULL(apPtr,SBKey());
    return apPtr->songPerformerKey();
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
    return QString("SBIDOnlinePerformance:opID=%1:apID=%2")
        .arg(itemID())
        .arg(_albumPerformanceID)
    ;
}

//	Static methods
SBKey
SBIDOnlinePerformance::createKey(int onlinePerformanceID)
{
    return SBKey(SBKey::OnlinePerformance,onlinePerformanceID);
}

SBIDOnlinePerformancePtr
SBIDOnlinePerformance::findByFK(const Common::sb_parameters &p)
{
    SBIDOnlinePerformancePtr spPtr;
    CacheManager* cm=Context::instance()->cacheManager();
    CacheOnlinePerformanceMgr* spMgr=cm->onlinePerformanceMgr();
    QMap<int,QList<SBIDOnlinePerformancePtr>> matches;
    int count=spMgr->find(p,SBIDOnlinePerformancePtr(),matches,1);

    if(count)
    {
        if(matches[0].count()==1)
        {
            spPtr=matches[0][0];
        }
    }
    return spPtr;
}

SBIDOnlinePerformancePtr
SBIDOnlinePerformance::retrieveOnlinePerformance(SBKey key)
{
    CacheManager* cm=Context::instance()->cacheManager();
    CacheOnlinePerformanceMgr* opMgr=cm->onlinePerformanceMgr();
    return opMgr->retrieve(key);
}

SBIDOnlinePerformancePtr
SBIDOnlinePerformance::retrieveOnlinePerformance(int onlinePerformanceID)
{
    return retrieveOnlinePerformance(createKey(onlinePerformanceID));
}

SBSqlQueryModel*
SBIDOnlinePerformance::retrieveOnlinePerformancesByAlbumPerformance(int albumPerformanceID)
{
    QString q=QString
    (
        "SELECT DISTINCT "
            "op.online_performance_id "
        "FROM "
            "___SB_SCHEMA_NAME___online_performance op "
        "WHERE "
            "album_performance_id=%1 "
    )
        .arg(albumPerformanceID)
    ;

    return new SBSqlQueryModel(q);
}

SBSqlQueryModel*
SBIDOnlinePerformance::retrieveAllOnlinePerformances(int limit)
{
    DataAccessLayer* dal=Context::instance()->dataAccessLayer();

    QString limitClause;

    if(limit)
    {
        limitClause=QString("LIMIT %1").arg(limit);
    }

    //	Main query
    QString q=QString
    (
        "SELECT DISTINCT "
            "op.online_performance_id, "
            "%1(op.last_play_date,'1/1/1900') AS SB_PLAY_ORDER "
        "FROM "
            "___SB_SCHEMA_NAME___online_performance op "
        "ORDER BY "
            "2 "
        "%2 "
    )
            .arg(dal->getIsNull())
            .arg(limitClause)
    ;

    return new SBSqlQueryModel(q);
}

SBSqlQueryModel*
SBIDOnlinePerformance::retrieveAllOnlinePerformancesExtended(int limit)
{
    QString limitClause;

    if(limit)
    {
        limitClause=QString("LIMIT %1").arg(limit);
    }

    //	Main query
    QString q=QString
    (
        "SELECT DISTINCT "
            "p.song_id, "
            "p.artist_id, "
            "p.performance_id, "
            "rp.record_id, "
            "r.artist_id, "
            "rp.record_position, "
            "rp.record_performance_id, "
            "op.online_performance_id, "
            "op.path "
        "FROM "
            "___SB_SCHEMA_NAME___online_performance op "
                "JOIN ___SB_SCHEMA_NAME___record_performance rp ON "
                    "op.record_performance_id=rp.record_performance_id "
                "JOIN ___SB_SCHEMA_NAME___performance p ON "
                    "rp.performance_id=p.performance_id "
                "JOIN ___SB_SCHEMA_NAME___record r ON "
                    "rp.record_id=r.record_id "
        "ORDER BY "
            "2 "
        "%2 "
    )
            .arg(limitClause)
    ;

    return new SBSqlQueryModel(q);
}

int
SBIDOnlinePerformance::totalNumberOnlinePerformances()
{
    DataAccessLayer* dal=Context::instance()->dataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
    QString q;
    int numSongs=0;

    //	Insert
    q=QString
    (
        "SELECT COUNT(*) FROM  ___SB_SCHEMA_NAME___online_performance "
    )
    ;
    dal->customize(q);
    qDebug() << SB_DEBUG_INFO << q;

    QSqlQuery query(q,db);

    if(query.next())
    {
        numSongs=query.value(0).toInt();
    }
    return numSongs;
}

QString
SBIDOnlinePerformance::onlinePerformancesBySong_Preloader(int songID)
{
    return QString
    (
        "SELECT DISTINCT "
            "s.song_id, "                         //	0
            "s.title, "
            "s.notes, "
            "p.artist_id, "
            "p.year, "

            "p.notes, "                           //	5
            "r.record_id, "
            "r.title, "
            "r.artist_id, "
            "r.year, "

            "r.genre, "                           //	10
            "r.notes, "
            "a.artist_id, "
            "a.name, "
            "a.www, "

            "a.notes, "                           //	15
            "a.mbid, "
            "rp.record_position, "
            "rp.duration, "
            "rp.notes, "

            "op.path, "                           //	20
            "rp.record_performance_id, "
            "l.lyrics, "
            "op.online_performance_id, "
            "rp.preferred_online_performance_id, "

            "p.performance_id, "                   //	25
            "rp.notes, "
            "s.original_performance_id, "
            "p.preferred_record_performance_id, "
            "NULL AS sole_id "
        "FROM "
            "___SB_SCHEMA_NAME___song s "
                "JOIN ___SB_SCHEMA_NAME___performance p ON "
                    "s.song_id=p.song_id "
                "JOIN ___SB_SCHEMA_NAME___artist a ON "
                    "p.artist_id=a.artist_id "
                "JOIN ___SB_SCHEMA_NAME___record_performance rp ON "
                    "p.performance_id=rp.performance_id "
                "LEFT JOIN ___SB_SCHEMA_NAME___record r ON "
                    "rp.record_id=r.record_id "
                "JOIN ___SB_SCHEMA_NAME___online_performance op ON "
                    "rp.record_performance_id=op.record_performance_id "
                "LEFT JOIN ___SB_SCHEMA_NAME___lyrics l ON "
                    "s.song_id=l.song_id "
        "WHERE "
            "s.song_id=%1 "
    )
        .arg(songID)
    ;
}

void
SBIDOnlinePerformance::refreshDependents(bool forcedFlag)
{
    Q_UNUSED(forcedFlag);
}

void
SBIDOnlinePerformance::setToReloadFlag()
{
    SBIDBase::setToReloadFlag();
    SBIDSongPtr sPtr=this->songPtr();
    SB_RETURN_VOID_IF_NULL(sPtr);
    sPtr->setToReloadFlag();
}

void
SBIDOnlinePerformance::setReloadFlag()
{
    SBIDBase::setReloadFlag();
    SBIDSongPtr sPtr=this->songPtr();
    SB_RETURN_VOID_IF_NULL(sPtr);
    sPtr->setReloadFlag();
}

///	Protected methods
SBIDOnlinePerformance::SBIDOnlinePerformance():SBIDBase(SBKey::OnlinePerformance,-1)
{
    _init();
}

SBIDOnlinePerformance::SBIDOnlinePerformance(int onlinePerformanceID):SBIDBase(SBKey::OnlinePerformance,onlinePerformanceID)
{
    _init();
}

SBIDOnlinePerformance&
SBIDOnlinePerformance::operator=(const SBIDOnlinePerformance& t)
{
    _copy(t);
    return *this;
}

///	Methods used by SBIDManager
SBIDOnlinePerformancePtr
SBIDOnlinePerformance::createInDB(Common::sb_parameters& p)
{
    DataAccessLayer* dal=Context::instance()->dataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
    QString q;

    //	Insert
    q=QString
    (
        "INSERT INTO ___SB_SCHEMA_NAME___online_performance "
        "( "
            "record_performance_id, "
            "path "
        ") "
        "VALUES "
        "( "
            "%1, "
            "'%2' "
        ") "
    )
        .arg(p.albumPerformanceID)
        .arg(Common::escapeSingleQuotes(p.path))
    ;
    dal->customize(q);
    qDebug() << SB_DEBUG_INFO << q;
    QSqlQuery insert(q,db);
    Q_UNUSED(insert);

    //	Instantiate
    SBIDOnlinePerformance op(dal->retrieveLastInsertedKey());
    op._albumPerformanceID =p.albumPerformanceID;
    op._path               =p.path;

    //	Done
    return std::make_shared<SBIDOnlinePerformance>(op);
}

SBSqlQueryModel*
SBIDOnlinePerformance::find(const Common::sb_parameters& tobeFound,SBIDOnlinePerformancePtr existingOnlinePerformancePtr)
{
    Q_UNUSED(existingOnlinePerformancePtr);

    //	MatchRank:
    //	0	-	exact match with specified artist (0 or 1 in data set).
    //	1	-	exact match with any other artist (0 or more in data set).
    //	2	-	soundex match with any other artist (0 or more in data set).
    QString q=QString
    (
        //	match on foreign keys
        "SELECT "
            "0 AS matchRank, "
            "p.online_performance_id, "
            "p.record_performance_id, "
            "p.path "
        "FROM "
            "___SB_SCHEMA_NAME___online_performance p "
        "WHERE "
            "p.record_performance_id=%1 "
        "ORDER BY "
            "1,3 "
    )
        .arg(tobeFound.albumPerformanceID)
    ;
    qDebug() << SB_DEBUG_INFO << q;
    return new SBSqlQueryModel(q);
}

SBIDOnlinePerformancePtr
SBIDOnlinePerformance::instantiate(const QSqlRecord& r)
{
    int i=0;

    SBIDOnlinePerformance op(Common::parseIntFieldDB(&r,i++));
    op._albumPerformanceID =Common::parseIntFieldDB(&r,i++);

    op._path               =r.value(i++).toString();

    return std::make_shared<SBIDOnlinePerformance>(op);
}

SBSqlQueryModel*
SBIDOnlinePerformance::retrieveSQL(SBKey key)
{
    QString q=QString
    (
        "SELECT DISTINCT "
            "op.online_performance_id, "
            "op.record_performance_id, "
            "op.path "
        "FROM "
            "___SB_SCHEMA_NAME___online_performance op "
        "%1  "
    )
        .arg(key.validFlag()?QString("WHERE op.online_performance_id=%1").arg(key.itemID()):QString())
    ;

    return new SBSqlQueryModel(q);
}

QStringList
SBIDOnlinePerformance::updateSQL(const Common::db_change db_change) const
{
    QStringList SQL;

    if(deletedFlag() && db_change==Common::db_delete)
    {
        //	CWIP
    }
    else if(!deletedFlag() && changedFlag() && db_change==Common::db_update)
    {
        SQL.append(QString
        (
            "UPDATE ___SB_SCHEMA_NAME___online_performance "
            "SET "
                "record_performance_id=%1, "
                "path='%2' "
            "WHERE "
                "online_performance_id=%3 "
        )
            .arg(this->_albumPerformanceID)
            .arg(this->_path)
            .arg(this->itemID())
        );
    }

    qDebug() << SB_DEBUG_INFO << SQL;
    return SQL;
}

///	Private methods
void
SBIDOnlinePerformance::_copy(const SBIDOnlinePerformance &c)
{
    SBIDBase::_copy(c);

    _albumPerformanceID =c._albumPerformanceID;
    _path               =c._path;

    _playPosition       =c._playPosition;
}

void
SBIDOnlinePerformance::_init()
{
    _albumPerformanceID=-1;
    _path=QString();
    _playPosition=-1;
}
