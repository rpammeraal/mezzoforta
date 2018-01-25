#include "SBIDAlbumPerformance.h"

#include "CacheManager.h"
#include "Context.h"
#include "DataAccessLayer.h"
#include "SBIDOnlinePerformance.h"

///	Ctors, dtors
SBIDAlbumPerformance::SBIDAlbumPerformance(const SBIDAlbumPerformance &p):SBIDBase(p)
{
    _copy(p);
}

SBIDAlbumPerformance::~SBIDAlbumPerformance()
{
}

///	Inherited methods
int
SBIDAlbumPerformance::commonPerformerID() const
{
    return this->songPerformerID();
}

QString
SBIDAlbumPerformance::commonPerformerName() const
{
    return this->songPerformerName();
}

QString
SBIDAlbumPerformance::iconResourceLocation() const
{
    return QString(":/images/SongIcon.png");
}

SBKey::ItemType
SBIDAlbumPerformance::itemType() const
{
    return SBKey::AlbumPerformance;
}

QString
SBIDAlbumPerformance::genericDescription() const
{
    return QString("Song - %1 [%2] / %3 - %4")
        .arg(this->text())
        .arg(this->_duration.toString(SBDuration::sb_hhmmss_format))
        .arg(this->songPerformerName())
        .arg(this->albumTitle().length()?QString("'%1'").arg(albumTitle()):QString())
    ;
}

QMap<int,SBIDOnlinePerformancePtr>
SBIDAlbumPerformance::onlinePerformances(bool updateProgressDialogFlag) const
{
    Q_UNUSED(updateProgressDialogFlag);
    QMap<int,SBIDOnlinePerformancePtr> list;
    const SBIDOnlinePerformancePtr opPtr=preferredOnlinePerformancePtr();
    if(opPtr)
    {
        list[0]=opPtr;
    }
    return list;
}

void
SBIDAlbumPerformance::sendToPlayQueue(bool enqueueFlag)
{
    const SBIDOnlinePerformancePtr opPtr=preferredOnlinePerformancePtr();
    if(opPtr)
    {
        opPtr->sendToPlayQueue(enqueueFlag);
    }
}

QString
SBIDAlbumPerformance::text() const
{
    return this->songTitle();
}

QString
SBIDAlbumPerformance::type() const
{
    return QString("song performance");
}


///	SBIDAlbumPerformance specific methods
int
SBIDAlbumPerformance::albumID() const
{
    return _albumID;
}

///	Setters
void
SBIDAlbumPerformance::setAlbumPosition(int position)
{
    _albumPosition=position;
    setChangedFlag();
}

///	Pointers
SBIDAlbumPtr
SBIDAlbumPerformance::albumPtr() const
{
    CacheManager* cm=Context::instance()->cacheManager();
    CacheAlbumMgr* aMgr=cm->albumMgr();
    return aMgr->retrieve(SBIDAlbum::createKey(_albumID));
}

SBIDSongPerformancePtr
SBIDAlbumPerformance::songPerformancePtr() const
{
    CacheManager* cm=Context::instance()->cacheManager();
    CacheSongPerformanceMgr* spMgr=cm->songPerformanceMgr();
    return spMgr->retrieve(SBIDSongPerformance::createKey(_songPerformanceID));
}

SBIDOnlinePerformancePtr
SBIDAlbumPerformance::preferredOnlinePerformancePtr() const
{
    CacheManager* cm=Context::instance()->cacheManager();
    CacheOnlinePerformanceMgr* opMgr=cm->onlinePerformanceMgr();
    return opMgr->retrieve(SBIDOnlinePerformance::createKey(_preferredOnlinePerformanceID));
}

///	Redirectors
SBKey
SBIDAlbumPerformance::albumKey() const
{
    SBIDAlbumPtr aPtr=albumPtr();
    SB_RETURN_IF_NULL(aPtr,SBKey());
    return aPtr->key();
}

int
SBIDAlbumPerformance::albumPerformerID() const
{
    SBIDAlbumPtr aPtr=albumPtr();
    return (aPtr?aPtr->albumPerformerID():-1);
}

QString
SBIDAlbumPerformance::albumPerformerName() const
{
    SBIDAlbumPtr aPtr=albumPtr();
    return (aPtr?aPtr->albumPerformerName():QString());
}

QString
SBIDAlbumPerformance::albumTitle() const
{
    SBIDAlbumPtr aPtr=albumPtr();
    return (aPtr?aPtr->albumTitle():QString());
}

int
SBIDAlbumPerformance::albumYear() const
{
    SBIDAlbumPtr aPtr=albumPtr();
    return (aPtr?aPtr->albumYear():0);
}

int
SBIDAlbumPerformance::songID() const
{
    SBIDSongPerformancePtr spPtr=songPerformancePtr();
    return (spPtr?spPtr->songID():-1);
}

int
SBIDAlbumPerformance::songPerformanceID() const
{
    SBIDSongPerformancePtr spPtr=songPerformancePtr();
    return (spPtr?spPtr->songPerformanceID():-1);
}

int
SBIDAlbumPerformance::songPerformerID() const
{
    SBIDSongPerformancePtr spPtr=songPerformancePtr();
    return (spPtr?spPtr->songPerformerID():-1);
}

SBKey
SBIDAlbumPerformance::songPerformerKey() const
{
    SBIDSongPerformancePtr spPtr=songPerformancePtr();
    SB_RETURN_IF_NULL(spPtr,SBKey());
    return spPtr->songPerformerKey();
}

SBIDSongPtr
SBIDAlbumPerformance::songPtr() const
{
    SBIDSongPerformancePtr spPtr=songPerformancePtr();
    return (spPtr?spPtr->songPtr():SBIDSongPtr());
}

QString
SBIDAlbumPerformance::songTitle() const
{
    SBIDSongPerformancePtr spPtr=songPerformancePtr();
    return (spPtr?spPtr->songTitle():QString());
}

QString
SBIDAlbumPerformance::songPerformerName() const
{
    SBIDSongPerformancePtr spPtr=songPerformancePtr();
    return (spPtr?spPtr->songPerformerName():QString());
}

int
SBIDAlbumPerformance::year() const
{
    SBIDSongPerformancePtr spPtr=songPerformancePtr();
    return (spPtr?spPtr->year():-1);
}


///	Operators
SBIDAlbumPerformance::operator QString()
{
    return QString("SBIDAlbumPerformance:apID=%1:spID=%2:aID=%3:pos=%4")
            .arg(itemID())
            .arg(_songPerformanceID)
            .arg(_albumID)
            .arg(_albumPosition)
    ;
}

//	Methods required by SBIDManagerTemplate
void
SBIDAlbumPerformance::refreshDependents(bool forcedFlag)
{
    Q_UNUSED(forcedFlag);
}

//	Static methods
SBKey
SBIDAlbumPerformance::createKey(int albumPerformanceID)
{
    return SBKey(SBKey::AlbumPerformance,albumPerformanceID);
}

SBIDAlbumPerformancePtr
SBIDAlbumPerformance::findByFK(const Common::sb_parameters &p)
{
    SBIDAlbumPerformancePtr spPtr;
    CacheManager* cm=Context::instance()->cacheManager();
    CacheAlbumPerformanceMgr* spMgr=cm->albumPerformanceMgr();
    QMap<int,QList<SBIDAlbumPerformancePtr>> matches;
    int count=spMgr->find(p,SBIDAlbumPerformancePtr(),matches,1);

    if(count)
    {
        if(matches[0].count()==1)
        {
            spPtr=matches[0][0];
        }
    }
    return spPtr;
}

QString
SBIDAlbumPerformance::performancesByAlbum_Preloader(int albumID)
{
    //	Only returns preferred online performances for each record performance.
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

            "rp.record_performance_id, "          //	20
            "l.lyrics, "
            "p.performance_id, "
            "NULL AS sole_id, "
            "s.original_performance_id, "

            "p.preferred_record_performance_id, "  //	25
            "rp.preferred_online_performance_id "
        "FROM "
            "___SB_SCHEMA_NAME___song s "
                "JOIN ___SB_SCHEMA_NAME___performance p ON "
                    "s.song_id=p.song_id  "
                "JOIN ___SB_SCHEMA_NAME___artist a ON "
                    "p.artist_id=a.artist_id "
                "JOIN ___SB_SCHEMA_NAME___record_performance rp ON "
                    "rp.performance_id=p.performance_id "
                "JOIN ___SB_SCHEMA_NAME___record r ON "
                    "rp.record_id=r.record_id AND "
                    "r.record_id=%1 "
                "LEFT JOIN ___SB_SCHEMA_NAME___lyrics l ON "
                    "s.song_id=l.song_id "
    )
        .arg(albumID)
    ;
}

QString
SBIDAlbumPerformance::performancesByPerformer_Preloader(int performerID)
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
            "p.preferred_record_performance_id "
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
                "LEFT JOIN ___SB_SCHEMA_NAME___online_performance op ON "
                    "rp.record_performance_id=op.record_performance_id "
                "LEFT JOIN ___SB_SCHEMA_NAME___lyrics l ON "
                    "s.song_id=l.song_id "
        "WHERE "
            "p.artist_id=%1 "
    )
        .arg(performerID)
    ;
}

SBSqlQueryModel*
SBIDAlbumPerformance::performancesBySong(int songID)
{
    QString q=QString
    (
        "SELECT DISTINCT "
            "rp.record_performance_id, "
            "rp.performance_id, "
            "rp.record_id, "
            "rp.record_position, "
            "rp.duration, "
            "rp.notes, "
            "rp.preferred_online_performance_id "
        "FROM "
            "___SB_SCHEMA_NAME___song s "
                "JOIN ___SB_SCHEMA_NAME___performance p ON " //	Removed LEFT. Want to get existing album performances.
                    "s.song_id=p.song_id "
                "JOIN ___SB_SCHEMA_NAME___record_performance rp ON " //	Removed LEFT. See above.
                    "p.performance_id=rp.performance_id "
        "WHERE s.song_id=%1 "
    )
        .arg(songID)
    ;

    qDebug() << SB_DEBUG_INFO << q;
    return new SBSqlQueryModel(q);
}

SBSqlQueryModel*
SBIDAlbumPerformance::performancesBySongPerformance(int songPerformanceID)
{
    QString q=QString
    (
        "SELECT DISTINCT "
            "rp.record_performance_id, "
            "rp.performance_id, "
            "rp.record_id, "
            "rp.record_position, "
            "rp.duration, "
            "rp.notes, "
            "rp.preferred_online_performance_id "
        "FROM "
            "___SB_SCHEMA_NAME___song s "
                "JOIN ___SB_SCHEMA_NAME___performance p ON " //	Removed LEFT. Want to get existing album performances.
                    "s.song_id=p.song_id "
                "JOIN ___SB_SCHEMA_NAME___record_performance rp ON " //	Removed LEFT. See above.
                    "p.performance_id=rp.performance_id "
        "WHERE p.performance_id=%1 "
    )
        .arg(songPerformanceID)
    ;

    qDebug() << SB_DEBUG_INFO << q;
    return new SBSqlQueryModel(q);
}

SBIDAlbumPerformancePtr
SBIDAlbumPerformance::retrieveAlbumPerformance(SBKey key)
{
    CacheManager* cm=Context::instance()->cacheManager();
    CacheAlbumPerformanceMgr* pfMgr=cm->albumPerformanceMgr();
    return pfMgr->retrieve(key);
}

SBIDAlbumPerformancePtr
SBIDAlbumPerformance::retrieveAlbumPerformance(int albumPerformanceID)
{
    return retrieveAlbumPerformance(createKey(albumPerformanceID));
}

///	Protected methods
SBIDAlbumPerformance::SBIDAlbumPerformance():SBIDBase(SBKey::AlbumPerformance,-1)
{
    _init();
}

SBIDAlbumPerformance::SBIDAlbumPerformance(int albumPerformanceID):SBIDBase(SBKey::AlbumPerformance,albumPerformanceID)
{
    _init();
}

SBIDAlbumPerformance&
SBIDAlbumPerformance::operator=(const SBIDAlbumPerformance& t)
{
    _copy(t);
    return *this;
}

SBIDAlbumPerformancePtr
SBIDAlbumPerformance::createInDB(Common::sb_parameters& p)
{
    DataAccessLayer* dal=Context::instance()->dataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
    QString q;

    //	Insert
    q=QString
    (
        "INSERT INTO ___SB_SCHEMA_NAME___record_performance "
        "( "
            "performance_id, "
            "record_id, "
            "record_position, "
            "duration, "
            "notes"
        ")"
        "VALUES "
        "( "
            "%1, "
            "%2, "
            "%3, "
            "'%4', "
            "'%5' "
        ") "
    )
        .arg(p.songPerformanceID)
        .arg(p.albumID)
        .arg(p.albumPosition)
        .arg(p.duration.toString(SBDuration::sb_full_hhmmss_format))
        .arg(Common::escapeSingleQuotes(p.notes))
    ;
    dal->customize(q);
    qDebug() << SB_DEBUG_INFO << q;
    QSqlQuery insert(q,db);
    Q_UNUSED(insert);

    //	Instantiate
    SBIDAlbumPerformance ap(dal->retrieveLastInsertedKey());
    ap._songPerformanceID =p.songPerformanceID;
    ap._albumID           =p.albumID;
    ap._albumPosition     =p.albumPosition;
    ap._duration          =p.duration;
    ap._notes             =p.notes;

    //	Done
    return std::make_shared<SBIDAlbumPerformance>(ap);
}

///
/// \brief SBIDAlbumPerformance::find
/// \param tobeFound
/// \param existingAlbumPerformancePtr
/// \return
///
/// Only used by SBIDAlbum::addAlbumPerformance.
/// If multiple ways to find is needed, add.
SBSqlQueryModel*
SBIDAlbumPerformance::find(const Common::sb_parameters& p,SBIDAlbumPerformancePtr existingAlbumPerformancePtr)
{
    Q_UNUSED(existingAlbumPerformancePtr);

    //	MatchRank:
    //	0	-	exact match with specified artist (0 or 1 in data set).
    //	1	-	exact match with any other artist (0 or more in data set).
    //	2	-	soundex match with any other artist (0 or more in data set).
    QString q=QString
    (
        //	match on foreign keys
        "SELECT "
            "0 AS matchRank, "
            "p.record_performance_id, "
            "p.performance_id, "
            "p.record_id, "
            "p.record_position, "
            "p.duration, "
            "p.notes, "
            "p.preferred_online_performance_id "
        "FROM "
            "___SB_SCHEMA_NAME___record_performance p "
        "WHERE "
            "p.performance_id=%1 AND "
            "p.record_id=%2 AND "
            "p.record_position=%3 "
    )
        .arg(p.songPerformanceID)
        .arg(p.albumID)
        .arg(p.albumPosition)
    ;
    qDebug() << SB_DEBUG_INFO << q;
    return new SBSqlQueryModel(q);
}

SBIDAlbumPerformancePtr
SBIDAlbumPerformance::instantiate(const QSqlRecord &r)
{
    int i=0;

    SBIDAlbumPerformance ap(Common::parseIntFieldDB(&r,i++));
    ap._songPerformanceID           =Common::parseIntFieldDB(&r,i++);
    ap._albumID                     =Common::parseIntFieldDB(&r,i++);
    ap._albumPosition               =Common::parseIntFieldDB(&r,i++);
    ap._duration                    =r.value(i++).toString();
    ap._notes                       =r.value(i++).toString();
    ap._preferredOnlinePerformanceID=Common::parseIntFieldDB(&r,i++);
    ap._orgAlbumPosition=ap._albumPosition;

    qDebug() << SB_DEBUG_INFO << ap.key() << ap._preferredOnlinePerformanceID;
    return std::make_shared<SBIDAlbumPerformance>(ap);
}

void
SBIDAlbumPerformance::mergeFrom(SBIDAlbumPerformancePtr fromApPtr)
{
    SB_RETURN_VOID_IF_NULL(fromApPtr);
    //	Reset albumPerformanceID in online_performance
    SBSqlQueryModel* qm;

    qm=SBIDOnlinePerformance::retrieveOnlinePerformancesByAlbumPerformance(fromApPtr->albumPerformanceID());
    CacheManager* cm=Context::instance()->cacheManager();
    CacheOnlinePerformanceMgr* opmgr=cm->onlinePerformanceMgr();
    SB_RETURN_VOID_IF_NULL(qm);
    SB_RETURN_VOID_IF_NULL(opmgr);

    for(int i=0;i>qm->rowCount();i++)
    {
        int onlinePerformanceID=qm->record(i).value(0).toInt();
        SBIDOnlinePerformancePtr opPtr=SBIDOnlinePerformance::retrieveOnlinePerformance(onlinePerformanceID);
        if(opPtr)
        {
            opPtr->setAlbumPerformanceID(this->albumPerformanceID());
        }
    }

    //	Reset preferredAlbumPerformanceID in performance
    qm=SBIDSongPerformance::performancesByPreferredAlbumPerformanceID(fromApPtr->albumPerformanceID());
    CacheSongPerformanceMgr* smgr=cm->songPerformanceMgr();
    SB_RETURN_VOID_IF_NULL(qm);
    SB_RETURN_VOID_IF_NULL(smgr);

    for(int i=0;i>qm->rowCount();i++)
    {
        int songPerformanceID=qm->record(i).value(0).toInt();
        SBIDSongPerformancePtr spPtr=SBIDSongPerformance::retrieveSongPerformance(songPerformanceID);
        if(spPtr)
        {
            spPtr->setPreferredAlbumPerformanceID(this->albumPerformanceID());
        }
    }
}

SBSqlQueryModel*
SBIDAlbumPerformance::retrieveSQL(SBKey key)
{
    QString q=QString
    (
        "SELECT DISTINCT "
            "rp.record_performance_id, "
            "rp.performance_id, "
            "rp.record_id, "
            "rp.record_position, "
            "rp.duration, "
            "rp.notes, "
            "rp.preferred_online_performance_id "
        "FROM "
            "___SB_SCHEMA_NAME___record_performance rp "
        "%1 "
    )
        .arg(key.validFlag()?QString("WHERE rp.record_performance_id=%1").arg(key.itemID()):QString())
    ;

    qDebug() << SB_DEBUG_INFO << q;
    return new SBSqlQueryModel(q);
}

QStringList
SBIDAlbumPerformance::updateSQL(const Common::db_change db_change) const
{
    QStringList SQL;

    if(deletedFlag() && db_change==Common::db_delete)
    {
        SQL.append(QString
        (
            "DELETE FROM "
                "___SB_SCHEMA_NAME___online_performance "
            "WHERE "
                "record_performance_id IN "
                "( "
                    "SELECT "
                        "record_performance_id "
                    "FROM "
                        "___SB_SCHEMA_NAME___record_performance "
                    "WHERE "
                        "record_id=%1 AND "
                       "record_position=%2 "
                ") "
        )
            .arg(this->_albumID)
            .arg(this->_albumPosition)
        );

        SQL.append(QString
        (
            "DELETE FROM ___SB_SCHEMA_NAME___record_performance "
            "WHERE record_id=%1 AND record_position=%2 "
        )
            .arg(this->_albumID)
            .arg(this->_albumPosition)
        );
    }
    else if(changedFlag() && db_change==Common::db_update)
    {
        SQL.append(QString
        (
            "UPDATE ___SB_SCHEMA_NAME___record_performance "
            "SET "
                "performance_id=%1, "
                "preferred_online_performance_id=NULLIF(%2,-1), "
                "record_id=%3, "
                "record_position=%4, "
                "duration='%5', "
                "notes='%6' "
            "WHERE "
                "record_performance_id=%7 "
        )
            .arg(this->_songPerformanceID)
            .arg(this->_preferredOnlinePerformanceID)
            .arg(this->_albumID)
            .arg(this->_albumPosition)
            .arg(this->_duration.toString(SBDuration::sb_full_hhmmss_format))
            .arg(Common::escapeSingleQuotes(this->_notes))
            .arg(this->itemID())
        );
    }

    if(SQL.count())
    {
        qDebug() << SB_DEBUG_INFO << SQL;
    }
    return SQL;
}

//	Private methods
void
SBIDAlbumPerformance::_copy(const SBIDAlbumPerformance &c)
{
    SBIDBase::_copy(c);

    _songPerformanceID            =c._songPerformanceID;
    _albumID                      =c._albumID;
    _albumPosition                =c._albumPosition;
    _duration                     =c._duration;
    _notes                        =c._notes;
    _preferredOnlinePerformanceID =c._preferredOnlinePerformanceID;

    _orgAlbumPosition             =c._orgAlbumPosition;
}

void
SBIDAlbumPerformance::_init()
{
    _songPerformanceID=-1;
    _albumID=-1;
    _albumPosition=-1;
    _duration=SBDuration();
    _notes=QString();
    _preferredOnlinePerformanceID=-1;
    _orgAlbumPosition=-1;
}
