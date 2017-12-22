#include "SBIDSongPerformance.h"

#include "CacheManager.h"
#include "Context.h"
#include "DataAccessLayer.h"
#include "SBIDAlbumPerformance.h"

SBIDSongPerformance::SBIDSongPerformance(const SBIDSongPerformance &p):SBIDBase(p)
{
    _copy(p);
}

//	Inherited methods
int
SBIDSongPerformance::commonPerformerID() const
{
    return this->songPerformerID();
}

QString
SBIDSongPerformance::commonPerformerName() const
{
    return this->songPerformerName();
}

QString
SBIDSongPerformance::iconResourceLocation() const
{
    return QString(":/images/SongIcon.png");
}

SBKey::ItemType
SBIDSongPerformance::itemType() const
{
    return SBKey::SongPerformance;
}

QString
SBIDSongPerformance::genericDescription() const
{
    return QString("Song - %1 / %2")
        .arg(this->songTitle())
        .arg(this->songPerformerName())
    ;
}

QMap<int,SBIDOnlinePerformancePtr>
SBIDSongPerformance::onlinePerformances(bool updateProgressDialogFlag) const
{
    QMap<int,SBIDOnlinePerformancePtr> list;
    const SBIDAlbumPerformancePtr apPtr=preferredAlbumPerformancePtr();
    if(apPtr)
    {
        list=apPtr->onlinePerformances(updateProgressDialogFlag);
    }
    return list;
}

void
SBIDSongPerformance::sendToPlayQueue(bool enqueueFlag)
{
    const SBIDAlbumPerformancePtr apPtr=preferredAlbumPerformancePtr();
    if(apPtr)
    {
        apPtr->sendToPlayQueue(enqueueFlag);
    }
}

QString
SBIDSongPerformance::text() const
{
    return songTitle();
}

QString
SBIDSongPerformance::type() const
{
    return QString("song performance");
}

///	SBIDSongPerformance specific methods

///	Pointers
SBIDPerformerPtr
SBIDSongPerformance::performerPtr() const
{
    CacheManager* cm=Context::instance()->cacheManager();
    CachePerformerMgr* pMgr=cm->performerMgr();
    return pMgr->retrieve(
                SBIDPerformer::createKey(_performerID),
                Cache::open_flag_parentonly);
}

SBIDAlbumPerformancePtr
SBIDSongPerformance::preferredAlbumPerformancePtr() const
{
    CacheManager* cm=Context::instance()->cacheManager();
    CacheAlbumPerformanceMgr* apMgr=cm->albumPerformanceMgr();
    return apMgr->retrieve(
                SBIDAlbumPerformance::createKey(_preferredAlbumPerformanceID),
                Cache::open_flag_parentonly);
}

SBIDOnlinePerformancePtr
SBIDSongPerformance::preferredOnlinePerformancePtr() const
{
    const SBIDAlbumPerformancePtr apPtr=preferredAlbumPerformancePtr();
    if(apPtr)
    {
        return apPtr->preferredOnlinePerformancePtr();
    }
    return SBIDOnlinePerformancePtr();
}

SBIDSongPtr
SBIDSongPerformance::songPtr() const
{
    CacheManager* cm=Context::instance()->cacheManager();
    CacheSongMgr* sMgr=cm->songMgr();
    return sMgr->retrieve(
                SBIDSong::createKey(_songID),
                Cache::open_flag_parentonly);
}

///	Redirectors
QString
SBIDSongPerformance::songPerformerName() const
{
    SBIDPerformerPtr pPtr=performerPtr();
    return (pPtr?pPtr->performerName():QString());
}

SBKey
SBIDSongPerformance::songPerformerKey() const
{
    SBIDPerformerPtr pPtr=performerPtr();
    SB_RETURN_IF_NULL(pPtr,SBKey());
    return pPtr->key();
}

SBKey
SBIDSongPerformance::songKey() const
{
    SBIDSongPtr sPtr=songPtr();
    SB_RETURN_IF_NULL(sPtr,SBKey());
    return sPtr->key();
}

QString
SBIDSongPerformance::songTitle() const
{
    SBIDSongPtr sPtr=songPtr();
    return (sPtr?sPtr->songTitle():QString());
}


///	Operators
SBIDSongPerformance::operator QString()
{
    return QString("SBIDSongPerformance:spID=%1:sID=%2:pID=%3:opFlag=%4")
            .arg(itemID())
            .arg(_songID)
            .arg(_performerID)
    ;
}

//	Methods required by SBIDManagerTemplate
SBKey
SBIDSongPerformance::createKey(int songPerformanceID)
{
    return SBKey(SBKey::SongPerformance,songPerformanceID);
}

void
SBIDSongPerformance::refreshDependents(bool showProgressDialogFlag,bool forcedFlag)
{
    Q_UNUSED(showProgressDialogFlag);
    Q_UNUSED(forcedFlag);
}

//	Static methods
SBIDSongPerformancePtr
SBIDSongPerformance::findByFK(const Common::sb_parameters &p)
{
    SBIDSongPerformancePtr spPtr;
    CacheManager* cm=Context::instance()->cacheManager();
    CacheSongPerformanceMgr* spMgr=cm->songPerformanceMgr();
    QMap<int,QList<SBIDSongPerformancePtr>> matches;
    const int count=spMgr->find(p,SBIDSongPerformancePtr(),matches,1);

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
SBIDSongPerformance::performancesByPerformer_Preloader(int performerID)
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
                    "s.song_id=p.song_id  "
                "JOIN ___SB_SCHEMA_NAME___artist a ON "
                    "p.artist_id=a.artist_id "
                "LEFT JOIN ___SB_SCHEMA_NAME___record_performance rp ON "
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

SBIDSongPerformancePtr
SBIDSongPerformance::retrieveSongPerformance(const SBKey& key,bool noDependentsFlag)
{
    CacheManager* cm=Context::instance()->cacheManager();
    CacheSongPerformanceMgr* spMgr=cm->songPerformanceMgr();
    return spMgr->retrieve(key, (noDependentsFlag==1?Cache::open_flag_parentonly:Cache::open_flag_default));
}

SBIDSongPerformancePtr
SBIDSongPerformance::retrieveSongPerformance(int songPerformanceID,bool noDependentsFlag)
{
    return retrieveSongPerformance(createKey(songPerformanceID),noDependentsFlag);
}

SBIDSongPerformancePtr
SBIDSongPerformance::retrieveSongPerformanceByPerformer(const QString &songTitle, const QString &performerName, int excludeSongPerformanceID, bool noDependentsFlag)
{
    SBIDSongPerformancePtr spPtr;
    DataAccessLayer* dal=Context::instance()->dataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());

    //	Find out songPerformanceID
    QString q=QString
    (
        "SELECT "
            "p.performance_id "
        "FROM "
            "___SB_SCHEMA_NAME___performance p "
                "JOIN ___SB_SCHEMA_NAME___song s ON "
                    "p.song_id=s.song_id "
                "JOIN ___SB_SCHEMA_NAME___artist a ON "
                    "p.artist_id=a.artist_id "
        "WHERE "
            "p.performance_id!=%1 AND"
            "s.title='%2' AND "
            "a.name='%3' "
    )
        .arg(excludeSongPerformanceID)
        .arg(songTitle)
        .arg(performerName)
    ;

    dal->customize(q);
    qDebug() << SB_DEBUG_INFO << q;

    QSqlQuery select(q,db);
    select.next();

    if(!select.isNull(0))
    {
        int songPerformanceID=select.value(0).toInt();
        spPtr=retrieveSongPerformance(songPerformanceID,noDependentsFlag);
    }
    return spPtr;
}

SBIDSongPerformancePtr
SBIDSongPerformance::retrieveSongPerformanceByPerformerID(int songID, int performerID, bool noDependentsFlag)
{
    SBIDSongPerformancePtr spPtr;
    DataAccessLayer* dal=Context::instance()->dataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());

    //	Find out songPerformanceID
    QString q=QString
    (
        "SELECT "
            "performance_id "
        "FROM "
            "___SB_SCHEMA_NAME___performance "
        "WHERE "
            "song_id=%1 AND "
            "artist_id=%2 "
    )
        .arg(songID)
        .arg(performerID)
    ;

    dal->customize(q);
    qDebug() << SB_DEBUG_INFO << q;

    QSqlQuery select(q,db);
    select.next();

    if(!select.isNull(0))
    {
        int songPerformanceID=select.value(0).toInt();
        spPtr=retrieveSongPerformance(songPerformanceID,noDependentsFlag);
    }
    return spPtr;
}

SBSqlQueryModel*
SBIDSongPerformance::performancesBySong(int songID)
{
    QString q=QString
    (
        "SELECT "
            "p.artist_id, "
            "p.performance_id, "
            "p.song_id, "
            "p.artist_id, "
            "p.year, "
            "p.notes, "
            "p.preferred_record_performance_id "
        "FROM "
            "___SB_SCHEMA_NAME___performance p "
        "WHERE "
            "p.song_id=%1 "
    )
        .arg(songID)
    ;

    return new SBSqlQueryModel(q);
}

SBSqlQueryModel*
SBIDSongPerformance::performancesByPreferredAlbumPerformanceID(int preferredAlbumPerformanceID)
{
    QString q=QString
    (
        "SELECT "
            "performance_id "
        "FROM "
            "___SB_SCHEMA_NAME___performance  "
        "WHERE "
            "preferred_record_performance_id=%1 "
    )
        .arg(preferredAlbumPerformanceID)
    ;

    return new SBSqlQueryModel(q);
}

///	Protected methods
SBIDSongPerformance::SBIDSongPerformance():SBIDBase(SBKey::SongPerformance,-1)
{
    _init();
}

SBIDSongPerformance::SBIDSongPerformance(int songPerformanceID):SBIDBase(SBKey::SongPerformance,songPerformanceID)
{
    _init();
}

SBIDSongPerformance&
SBIDSongPerformance::operator=(const SBIDSongPerformance& t)
{
    _copy(t);
    return *this;
}

//	Methods used by SBIDManager
SBIDSongPerformancePtr
SBIDSongPerformance::createInDB(Common::sb_parameters& p)
{
    DataAccessLayer* dal=Context::instance()->dataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
    QString q;

    if(p.year<1900)
    {
        p.year=1900;
    }
    //	Insert
    q=QString
    (
        "INSERT INTO ___SB_SCHEMA_NAME___performance "
        "( "
            "song_id, "
            "artist_id, "
            "year, "
            "notes "
        ") "
        "VALUES "
        "( "
            "%1, "
            "%2, "
            "%3, "
            "'%4' "
        ") "
    )
        .arg(p.songID)
        .arg(p.performerID)
        .arg(p.year)
        .arg(p.notes)
    ;
    dal->customize(q);
    qDebug() << SB_DEBUG_INFO << q;
    QSqlQuery insert(q,db);
    Q_UNUSED(insert);

    //	Instantiate
    SBIDSongPerformance sp(dal->retrieveLastInsertedKey());
    sp._songID           =p.songID;
    sp._performerID      =p.performerID;
    sp._year             =p.year;
    sp._notes            =p.notes;

    //	Done
    return std::make_shared<SBIDSongPerformance>(sp);
}

SBSqlQueryModel*
SBIDSongPerformance::find(const Common::sb_parameters& tobeFound,SBIDSongPerformancePtr existingSongPerformancePtr)
{
    QString newSoundex=Common::soundex(tobeFound.songTitle);
    int excludeID=(existingSongPerformancePtr?existingSongPerformancePtr->songID():-1);

    //	MatchRank:
    //	0	-	exact match with specified artist (0 or 1 in data set).
    //	1	-	exact match with any other artist (0 or more in data set).
    //	2	-	soundex match with any other artist (0 or more in data set).
    QString q=QString
    (
        //	match on foreign keys
        "SELECT "
            "0 AS matchRank, "
            "p.performance_id, "
            "p.song_id, "
            "p.artist_id, "
            "p.year, "
            "p.notes "
        "FROM "
            "___SB_SCHEMA_NAME___performance p "
        "WHERE "
            "p.song_id=%5 AND "
            "p.artist_id=%2 "
        "UNION "
        //	match on title and optional performerID
        "SELECT "
            "CASE WHEN p.artist_id=%2 THEN 0 ELSE 1 END AS matchRank, "
            "p.performance_id, "
            "s.song_id, "
            "p.artist_id, "
            "p.year, "
            "p.notes "
        "FROM "
            "___SB_SCHEMA_NAME___performance p "
                "JOIN ___SB_SCHEMA_NAME___song s ON "
                    "p.song_id=s.song_id "
                    "%4 "
        "WHERE "
            "REPLACE(LOWER(s.title),' ','') = REPLACE(LOWER('%1'),' ','') "
        "UNION "
        //	soundex match, only if length of soundex > 0
        "SELECT "
            "2 AS matchRank, "
            "p.performance_id, "
            "s.song_id, "
            "p.artist_id, "
            "p.year, "
            "p.notes "
        "FROM "
            "___SB_SCHEMA_NAME___performance p "
                "JOIN ___SB_SCHEMA_NAME___song s ON "
                    "p.performance_id=s.original_performance_id "
                    "%4 "
        "WHERE "
            "LENGTH('%3')!=0 AND "
            "( "
                "SUBSTR(s.soundex,1,LENGTH('%3'))='%3' OR "
                "SUBSTR('%3',1,LENGTH(s.soundex))=s.soundex "
            ") "
        "ORDER BY "
            "1,3 "
    )
        .arg(Common::escapeSingleQuotes(Common::simplified(tobeFound.songTitle)))
        .arg(tobeFound.performerID)
        .arg(newSoundex)
        .arg(excludeID==-1?"":QString(" AND s.song_id!=(%1)").arg(excludeID))
        .arg(tobeFound.songID)
    ;
    qDebug() << SB_DEBUG_INFO << q;
    return new SBSqlQueryModel(q);
}

SBIDSongPerformancePtr
SBIDSongPerformance::instantiate(const QSqlRecord &r)
{
    int i=0;

    SBIDSongPerformance sP(Common::parseIntFieldDB(&r,i++));
    sP._songID                     =Common::parseIntFieldDB(&r,i++);
    sP._performerID                =Common::parseIntFieldDB(&r,i++);
    sP._year                       =r.value(i++).toInt();
    sP._notes                      =r.value(i++).toString();
    sP._preferredAlbumPerformanceID=Common::parseIntFieldDB(&r,i++);

    sP._year=(sP._year<1900?1900:sP._year);
    return std::make_shared<SBIDSongPerformance>(sP);
}

void
SBIDSongPerformance::mergeFrom(SBIDSongPerformancePtr &spPtrFrom)
{
    SBSqlQueryModel* qm;
    CacheManager* cm=Context::instance()->cacheManager();

    //	album_performance
    qm=SBIDAlbumPerformance::performancesBySongPerformance(spPtrFrom->songPerformanceID());
    CacheAlbumPerformanceMgr* apMgr=cm->albumPerformanceMgr();
    SB_RETURN_VOID_IF_NULL(qm);
    SB_RETURN_VOID_IF_NULL(apMgr);

    for(int i=0;i<qm->rowCount();i++)
    {
        int albumPerformanceID=qm->record(i).value(0).toInt();
        SBIDAlbumPerformancePtr aPtr=SBIDAlbumPerformance::retrieveAlbumPerformance(albumPerformanceID);
        aPtr->setSongPerformanceID(this->songPerformanceID());
    }

    //	chart_performance
    qm=SBIDChartPerformance::chartPerformancesBySongPerformance(spPtrFrom->songPerformanceID());
    CacheChartPerformanceMgr* cpMgr=cm->chartPerformanceMgr();
    SB_RETURN_VOID_IF_NULL(qm);
    SB_RETURN_VOID_IF_NULL(cpMgr);

    for(int i=0;i<qm->rowCount();i++)
    {
        int chartPerformanceID=qm->record(i).value(0).toInt();
        SBIDChartPerformancePtr cpPtr=SBIDChartPerformance::retrieveChartPerformance(chartPerformanceID);
        cpPtr->setSongPerformanceID(this->songPerformanceID());
    }
}

SBSqlQueryModel*
SBIDSongPerformance::retrieveSQL(SBKey key)
{
    QString q=QString
    (
        "SELECT "
            "p.performance_id, "
            "p.song_id, "
            "p.artist_id, "
            "p.year, "
            "p.notes, "
            "p.preferred_record_performance_id "
        "FROM "
            "___SB_SCHEMA_NAME___performance p "
        "%1 "
    )
        .arg(key.validFlag()?QString("WHERE p.performance_id=%1").arg(key.itemID()):QString())
    ;
    qDebug() << SB_DEBUG_INFO << q;

    return new SBSqlQueryModel(q);
}

QStringList
SBIDSongPerformance::updateSQL(const Common::db_change db_change) const
{
    QStringList SQL;

    if(this->_year<1900)
    {
        const_cast<SBIDSongPerformance *>(this)->_year=1900;
    }

    if(deletedFlag() && db_change==Common::db_delete)
    {
        SQL.append(QString
        (
            "DELETE FROM "
                "___SB_SCHEMA_NAME___performance "
            "WHERE "
                "performance_id=%1 "
        )
            .arg(this->itemID())
        );
    }
    else if(changedFlag() && db_change==Common::db_update)
    {
        SQL.append(QString
        (
            "UPDATE ___SB_SCHEMA_NAME___performance "
            "SET "
                "song_id=%1, "
                "artist_id=%2, "
                "preferred_record_performance_id=NULLIF(%3,-1), "
                "year=%4, "
                "notes='%5' "
            "WHERE "
                "performance_id=%6 "
        )
            .arg(this->_songID)
            .arg(this->_performerID)
            .arg(this->_preferredAlbumPerformanceID)
            .arg(this->_year)
            .arg(Common::escapeSingleQuotes(this->notes()))
            .arg(this->itemID())
        );
    }
    return SQL;
}

//	Private methods
void
SBIDSongPerformance::_copy(const SBIDSongPerformance &c)
{
    SBIDBase::_copy(c);

    _songID                     =c._songID;
    _performerID                =c._performerID;
    _year                       =c._year;
    _notes                      =c._notes;
    _preferredAlbumPerformanceID=c._preferredAlbumPerformanceID;
}

void
SBIDSongPerformance::_init()
{
    _songID=-1;
    _performerID=-1;
    _year=-1;
    _notes=QString();
    _preferredAlbumPerformanceID=-1;
}

void
SBIDSongPerformance::postInstantiate(SBIDSongPerformancePtr &ptr)
{
    Q_UNUSED(ptr);
}
