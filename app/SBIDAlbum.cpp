#include <QStringList>

#include "SBIDAlbum.h"

#include "Context.h"
#include "Preloader.h"
#include "ProgressDialog.h"
#include "SBDialogSelectItem.h"
#include "SBIDPerformer.h"
#include "SBMessageBox.h"
#include "SBModelQueuedSongs.h"
#include "SBSqlQueryModel.h"
#include "SBTableModel.h"

SBIDAlbum::SBIDAlbum(const SBIDAlbum &c):SBIDBase(c)
{
    _copy(c);
}

SBIDAlbum::~SBIDAlbum()
{

}

///	Public methods
int
SBIDAlbum::commonPerformerID() const
{
    return this->albumPerformerID();
}

QString
SBIDAlbum::commonPerformerName() const
{
    return this->albumPerformerName();
}

QString
SBIDAlbum::genericDescription() const
{
    return "Album - "+this->text()+" by "+this->albumPerformerName();
}

QString
SBIDAlbum::iconResourceLocation() const
{
    return ":/images/NoAlbumCover.png";
}

int
SBIDAlbum::itemID() const
{
    return this->albumID();
}

Common::sb_type
SBIDAlbum::itemType() const
{
    return Common::sb_type_album;
}

///
/// \brief SBIDAlbum::save
/// \return
///
/// Inserts a new album or updates an existing album.
bool
SBIDAlbum::save()
{
    if(this->_albumID==-1)
    {
        //	Insert new
        DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
        QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
        QString newSoundex=Common::soundex(this->albumPerformerName());
        QString q;

        q=QString
        (
            "INSERT INTO ___SB_SCHEMA_NAME___record "
            "( "
                "record_id, "
                "artist_id, "
                "title, "
                "media, "
                "year, "
                "genre, "
                "notes "
            ") "
            "SELECT "
                "MAX(record_id)+1, "
                "%1, "
                "'%2', "
                "'digital', "
                "%3, "
                "'%4', "
                "'%5' "
            "FROM "
                "___SB_SCHEMA_NAME___record "
        )
            .arg(this->_albumPerformerID)
            .arg(Common::escapeSingleQuotes(this->_albumTitle))
            .arg(this->_year)
            .arg(Common::escapeSingleQuotes(this->_genre))
            .arg(Common::escapeSingleQuotes(this->_notes))
        ;


        dal->customize(q);
        qDebug() << SB_DEBUG_INFO << q;
        QSqlQuery insert(q,db);
        QSqlError e=insert.lastError();
        if(e.isValid())
        {
            SBMessageBox::databaseErrorMessageBox(q,e);
            return false;
        }

        //	Get ID if newly added record
        q=QString
        (
            "SELECT "
                "record_id "
            "FROM "
                "___SB_SCHEMA_NAME___record "
            "WHERE "
                "title='%1' AND "
                "artist_id=%2 "
        )
            .arg(Common::escapeSingleQuotes(this->_albumTitle))
            .arg(this->_albumPerformerID)
        ;

        dal->customize(q);
        QSqlQuery select(q,db);
        select.next();
        this->_albumID=select.value(0).toInt();
    }
    else
    {
        //	Update existing
    }
    return true;
}

QMap<int,SBIDOnlinePerformancePtr>
SBIDAlbum::onlinePerformances(bool updateProgressDialogFlag) const
{
    QMap<int,SBIDAlbumPerformancePtr> albumPerformances=this->albumPerformances();

    int progressCurrentValue=0;
    int progressMaxValue=albumPerformances.count()*2;
    if(updateProgressDialogFlag)
    {
        ProgressDialog::instance()->update("SBIDAlbum::onlinePerformances",0,progressMaxValue);
    }

    //	Collect onlinePerformancePtrs and their album position
    QMapIterator<int,SBIDAlbumPerformancePtr> pIT(albumPerformances);
    QMap<int, SBIDOnlinePerformancePtr> position2OnlinePerformancePtr;
    int i=0;
    while(pIT.hasNext())
    {
        pIT.next();
        const SBIDAlbumPerformancePtr apPtr=pIT.value();
        const SBIDOnlinePerformancePtr opPtr=apPtr->preferredOnlinePerformancePtr();
        if(opPtr && opPtr->path().length()>0)
        {
            position2OnlinePerformancePtr[i++]=opPtr;
        }
        if(updateProgressDialogFlag)
        {
            ProgressDialog::instance()->update("SBIDAlbum::onlinePerformances",progressCurrentValue++,progressMaxValue);
        }
    }

    //	Now put everything in order. Note that some albumPositions may be missing.
    QMap<int,SBIDOnlinePerformancePtr> list;
    QMapIterator<int,SBIDOnlinePerformancePtr> po2olIT(position2OnlinePerformancePtr);
    int index=0;
    while(po2olIT.hasNext())
    {
        po2olIT.next();
        list[index++]=po2olIT.value();
        if(updateProgressDialogFlag)
        {
            ProgressDialog::instance()->update("SBIDAlbum::onlinePerformances",progressCurrentValue++,progressMaxValue);
        }
    }
    if(updateProgressDialogFlag)
    {
        ProgressDialog::instance()->finishStep("SBIDAlbum::onlinePerformances");
    }

    return list;
}

void
SBIDAlbum::sendToPlayQueue(bool enqueueFlag)
{
    ProgressDialog::instance()->show("Loading songs","SBIDAlbum::sendToPlayQueue",1);

    QMap<int,SBIDOnlinePerformancePtr> list=this->onlinePerformances(1);
    SBModelQueuedSongs* mqs=Context::instance()->getSBModelQueuedSongs();
    mqs->populate(list,enqueueFlag);

    ProgressDialog::instance()->hide();
}

QString
SBIDAlbum::text() const
{
    return _albumTitle;
}

QString
SBIDAlbum::type() const
{
    return "album";
}

SBIDAlbumPerformancePtr
SBIDAlbum::addAlbumPerformance(int songID, int performerID, int albumPosition, int year, const QString& path, const SBDuration& duration, const QString& notes)
{
    CacheManager* cm=Context::instance()->cacheManager();
    CacheSongPerformanceMgr* spMgr=cm->songPerformanceMgr();
    CacheAlbumPerformanceMgr* apMgr=cm->albumPerformanceMgr();
    CacheOnlinePerformanceMgr* opMgr=cm->onlinePerformanceMgr();
    SBIDSongPtr sPtr;
    SBIDSongPerformancePtr spPtr;
    SBIDAlbumPerformancePtr apPtr;
    Common::sb_parameters p;
    bool newSongPerformanceFlag=0;
    bool newAlbumPerformanceFlag=0;

    albumPerformances();	//	load albumPerformances if not already loaded

    //	Look up song (should exists)
    sPtr=SBIDSong::retrieveSong(songID);
    if(!sPtr)
    {
        qDebug() << SB_DEBUG_ERROR << "song does not exist. song_id=" << songID;
        return apPtr;
    }

    //	Look up song performance
    p.songID=songID;
    p.performerID=performerID;
    p.year=year;
    spPtr=SBIDSongPerformance::findByFK(p);
    if(!spPtr)
    {
        newSongPerformanceFlag=1;
        spPtr=spMgr->createInDB(p);
        ;
    }

    //	Lookup album performance
    p.songPerformanceID=spPtr->songPerformanceID();
    p.albumID=this->albumID();
    p.albumPosition=albumPosition;
    p.duration=duration;
    p.notes=notes;
    apPtr=SBIDAlbumPerformance::findByFK(p);
    if(!apPtr)
    {
        newAlbumPerformanceFlag=1;
        apPtr=apMgr->createInDB(p);
        ;
    }
    if(!albumPerformances().contains(apPtr->albumPerformanceID()))
    {
        _albumPerformances[apPtr->albumPerformanceID()]=apPtr;
        this->setChangedFlag();
    }

    //	Lookup online performance
    p.albumPerformanceID=apPtr->albumPerformanceID();
    p.path=path;
    SBIDOnlinePerformancePtr opPtr=SBIDOnlinePerformance::findByFK(p);
    if(!opPtr)
    {
        opPtr=opMgr->createInDB(p);
    }

    //	Set ID's pointing down the hierarchy
    if(sPtr->originalSongPerformanceID()<0)
    {
        sPtr->setOriginalPerformanceID(spPtr->songPerformanceID());
    }
    if(spPtr->preferredAlbumPerformanceID()<0)
    {
        spPtr->setPreferredAlbumPerformanceID(apPtr->albumPerformanceID());
    }
    if(apPtr->preferredOnlinePerformanceID()<0)
    {
        apPtr->setPreferredOnlinePerformanceID(opPtr->onlinePerformanceID());
    }

    return apPtr;
}

SBDuration
SBIDAlbum::duration() const
{
    SBDuration duration;

    QMapIterator<int,SBIDAlbumPerformancePtr> apIT(albumPerformances());
    while(apIT.hasNext())
    {
        apIT.next();

        const SBIDAlbumPerformancePtr apPtr=apIT.value();
        duration+=apPtr->duration();
    }
    return duration;
}

//SBSqlQueryModel*
//SBIDAlbum::matchAlbum() const
//{
//    //	MatchRank:
//    //	0	-	edited value (always one in data set).
//    //	1	-	exact match with specified artist (0 or 1 in data set).
//    //	2	-	exact match with any other artist (0 or more in data set).
//    QString q=QString
//    (
//        "SELECT "
//            "0 AS matchRank, "
//            "-1 AS album_id, "
//            "'%1' AS title, "
//            "%3 AS artist_id, "
//            "'%2' AS artistName "
//        "UNION "
//        "SELECT "
//            "CASE WHEN a.artist_id=%3 THEN 1 ELSE 2 END AS matchRank, "
//            "p.record_id, "
//            "p.title, "
//            "a.artist_id, "
//            "a.name "
//        "FROM "
//            "___SB_SCHEMA_NAME___record p "
//                "JOIN ___SB_SCHEMA_NAME___artist a ON "
//                    "p.artist_id=a.artist_id "
//        "WHERE "
//            "REPLACE(LOWER(p.title),' ','') = REPLACE(LOWER('%1'),' ','') AND "
//            "p.record_id!=%4 "
//        "ORDER BY "
//            "1 "
//    )
//        .arg(Common::escapeSingleQuotes(this->albumTitle()))
//        .arg(Common::escapeSingleQuotes(this->albumPerformerName()))
//        .arg(this->albumPerformerID())
//        .arg(this->albumID())
//    ;
//    return new SBSqlQueryModel(q);
//}

void
SBIDAlbum::postInstantiate(SBIDAlbumPtr &ptr)
{
    Q_UNUSED(ptr);
}

int
SBIDAlbum::numPerformances() const
{
    return albumPerformances().count();
}

QMap<int,SBIDAlbumPerformancePtr>
SBIDAlbum::albumPerformances() const
{
    if(_albumPerformances.count()==0)
    {
        const_cast<SBIDAlbum *>(this)->_loadAlbumPerformances();
    }
    return _albumPerformances;
}

QStringList
SBIDAlbum::removeAlbum()
{
    QStringList SQL;

    SQL.append
    (
        QString
        (
            "DELETE FROM "
                "___SB_SCHEMA_NAME___online_performance "
            "WHERE "
                "record_id=%1 "
        )
            .arg(this->albumID())
    );

    SQL.append
    (
        QString
        (
            "DELETE FROM "
                "___SB_SCHEMA_NAME___record_performance "
            "WHERE "
                "record_id=%1 "
        )
            .arg(this->albumID())
    );

    SQL.append
    (
        QString
        (
            "DELETE FROM "
                "___SB_SCHEMA_NAME___playlist_detail "
            "WHERE "
                "record_id=%1 "
        )
            .arg(this->albumID())
    );

    SQL.append
    (
        QString
        (
            "DELETE FROM "
                "___SB_SCHEMA_NAME___record "
            "WHERE "
                "record_id=%1 "
        )
            .arg(this->albumID())
    );

    return SQL;
}

QStringList
SBIDAlbum::removeSongFromAlbum(int position)
{
    QStringList SQL;

    SQL.append
    (
        QString
        (
            "DELETE FROM "
                "___SB_SCHEMA_NAME___online_performance "
            "WHERE "
                "record_performance IN "
                "( "
                    "SELECT "
                        "record_performance_id "
                    "FROM"
                        "___SB_SCHEMA_NAME___record_performance "
                    "WHERE "
                        "record_id=%1 AND "
                        "record_position=%2 "
                ") "
        )
            .arg(this->albumID())
            .arg(position)
    );

    SQL.append
    (
        QString
        (
            "DELETE FROM "
                "___SB_SCHEMA_NAME___record_performance "
            "WHERE "
                "record_id=%1 AND "
                "record_position=%2 "
        )
            .arg(this->albumID())
            .arg(position)
    );

    SQL.append
    (
        QString
        (
            "DELETE FROM "
                "___SB_SCHEMA_NAME___playlist_detail "
            "WHERE "
                "record_performance_id IN "
                    "( "
                        "SELECT "
                            "record_performance_id "
                        "FROM "
                            "___SB_SCHEMA_NAME___record_performance rp"
                        "WHERE "
                            "record_id=%1 AND "
                            "record_position=%2 "
                    ") "
        )
            .arg(this->albumID())
            .arg(position)
    );

    return SQL;
}

QStringList
SBIDAlbum::repositionSongOnAlbum(int fromPosition, int toPosition)
{
    QStringList SQL;

    //	Update rock_performance fields
    SQL.append
    (
        QString
        (
            "UPDATE "
                "___SB_SCHEMA_NAME___record_performance "
            "SET "
                "record_position=%1 "
            "WHERE "
                "record_id=%2 AND "
                "record_position=%3 "
        )
            .arg(toPosition)
            .arg(this->albumID())
            .arg(fromPosition)
    );

    return SQL;
}

SBTableModel*
SBIDAlbum::tableModelPerformances() const
{
    SBTableModel* tm=new SBTableModel();
    tm->populatePerformancesByAlbum(albumPerformances());
    return tm;
}

bool
SBIDAlbum::updateExistingAlbum(const SBIDBase& orgAlbum, const SBIDBase& newAlbum, const QStringList &extraSQL,bool commitFlag)
{
    Q_UNUSED(orgAlbum);
    Q_UNUSED(newAlbum);
    Q_UNUSED(extraSQL);
    Q_UNUSED(commitFlag);
//    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();

//    QStringList allQueries;
//    QString q;
    bool resultFlag=1;

    /*
    //	artist
    if(orgAlbum.albumPerformerID()!=newAlbum.albumPerformerID())
    {
        q=QString
        (
            "UPDATE ___SB_SCHEMA_NAME___record "
            "SET "
                "artist_id=%1 "
            "WHERE "
                "record_id=%2 "
        )
            .arg(newAlbum.albumPerformerID())
            .arg(newAlbum.albumID())
        ;
        allQueries.append(q);
    }

    //	title
    if(orgAlbum.albumTitle()!=newAlbum.albumTitle())
    {
        q=QString
        (
            "UPDATE ___SB_SCHEMA_NAME___record "
            "SET "
                "title='%1' "
            "WHERE "
                "record_id=%2 "
        )
            .arg(Common::escapeSingleQuotes(newAlbum.albumTitle()))
            .arg(newAlbum.albumID())
        ;
        allQueries.append(q);
    }

    //	year
    if(orgAlbum.year()!=newAlbum.year())
    {
        q=QString
        (
            "UPDATE ___SB_SCHEMA_NAME___record "
            "SET "
                "year=%1 "
            "WHERE "
                "record_id=%2 "
        )
            .arg(newAlbum.year())
            .arg(newAlbum.albumID())
        ;
        allQueries.append(q);
    }

    allQueries.append(extraSQL);

    resultFlag=dal->executeBatch(allQueries,commitFlag);
    */

    return resultFlag;
}

///	Pointers
SBIDPerformerPtr
SBIDAlbum::performerPtr() const
{
    CacheManager* cm=Context::instance()->cacheManager();
    CachePerformerMgr* pMgr=cm->performerMgr();
    return pMgr->retrieve(
                SBIDPerformer::createKey(_albumPerformerID),
                Cache::open_flag_parentonly);
}

///	Redirectors
QString
SBIDAlbum::albumPerformerName() const
{
    SBIDPerformerPtr pPtr=performerPtr();
    return (pPtr?pPtr->performerName():QString());
}

QString
SBIDAlbum::albumPerformerMBID() const
{
    SBIDPerformerPtr pPtr=performerPtr();
    return (pPtr?pPtr->MBID():QString());
}

///	Operators
SBIDAlbum::operator QString() const
{
    return QString("SBIDAlbum:%1:t=%2")
            .arg(_albumID)
            .arg(_albumTitle)
    ;
}

//	Methods required by SBIDManagerTemplate
SBKey
SBIDAlbum::createKey(int albumID)
{
    return SBKey(Common::sb_type_album,albumID);
}

void
SBIDAlbum::refreshDependents(bool showProgressDialogFlag,bool forcedFlag)
{
    if(showProgressDialogFlag)
    {
        ProgressDialog::instance()->show("Retrieving Album","SBIDChart::refreshDependents",1);
    }

    if(forcedFlag==1 || _albumPerformances.count()==0)
    {
        _loadAlbumPerformances();
    }
}

SBIDAlbumPtr
SBIDAlbum::retrieveAlbum(const SBKey& key,bool noDependentsFlag)
{
    CacheManager* cm=Context::instance()->cacheManager();
    CacheAlbumMgr* amgr=cm->albumMgr();
    return amgr->retrieve(key,(noDependentsFlag==1?Cache::open_flag_parentonly:Cache::open_flag_default));
}

SBIDAlbumPtr
SBIDAlbum::retrieveAlbum(int albumID,bool noDependentsFlag)
{
    return retrieveAlbum(createKey(albumID),noDependentsFlag);
}

SBIDAlbumPtr
SBIDAlbum::retrieveAlbumByPath(const QString& albumPath, bool noDependentsFlag)
{
    //	CWIP: need to store mapping in memory for faster retrieval. Maybe store path
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
    SBIDAlbumPtr aPtr;

    //	Find albumID, then retrieve through aMgr
    int albumID=-1;
    QString q=QString
    (
        "SELECT DISTINCT "
            "r.record_id, "
            "COUNT(DISTINCT r.record_id) "
        "FROM "
            "___SB_SCHEMA_NAME___online_performance op "
                "JOIN ___SB_SCHEMA_NAME___record_performance rp USING(record_performance_id) "
                "JOIN ___SB_SCHEMA_NAME___record r USING(record_id) "
        "WHERE "
            "LEFT(path,LENGTH('%1'))='%1' AND "
            "LENGTH('%1')<>0 "
        "GROUP BY "
            "r.record_id "
        "HAVING "
            "COUNT(DISTINCT r.record_id)=1 "
    )
        .arg(albumPath)
    ;

    dal->customize(q);
    qDebug() << SB_DEBUG_INFO << q;
    QSqlQuery qID(q,db);
    while(albumID==-1 && qID.next())
    {
        albumID=qID.value(0).toInt();
        int count=qID.value(1).toInt();
        qDebug() << SB_DEBUG_INFO << albumID << count;
        if(count!=1)
        {
            qDebug() << SB_DEBUG_INFO;
            albumID=-1;
        }
    }

    if(albumID!=-1)
    {
        aPtr=SBIDAlbum::retrieveAlbum(albumID,noDependentsFlag);
    }
    return aPtr;
}

SBIDAlbumPtr
SBIDAlbum::retrieveAlbumByTitlePerformer(const QString &albumTitle, const QString &performerName, bool noDependentsFlag)
{
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
    SBIDAlbumPtr aPtr;

    //	Find albumID, then retrieve through aMgr
    int albumID=-1;
    QString q=QString
    (
        "SELECT DISTINCT "
            "r.record_id "
        "FROM "
            "___SB_SCHEMA_NAME___record r "
                "INNER JOIN ___SB_SCHEMA_NAME___artist a ON "
                    "r.artist_id=a.artist_id "
        "WHERE "
            "r.title=%1 AND "
            "a.name=%2 "
    )
        .arg(albumTitle)
        .arg(performerName)
    ;

    dal->customize(q);
    qDebug() << SB_DEBUG_INFO << q;
    QSqlQuery qID(q,db);
    while(albumID==-1 && qID.next())
    {
        albumID=qID.value(0).toInt();
    }

    if(albumID!=-1)
    {
        aPtr=SBIDAlbum::retrieveAlbum(albumID,noDependentsFlag);
    }

    return aPtr;
}

SBIDAlbumPtr
SBIDAlbum::retrieveUnknownAlbum()
{
    Properties* properties=Context::instance()->getProperties();
    CacheManager* cm=Context::instance()->cacheManager();
    CacheAlbumMgr* amgr=cm->albumMgr();
    int albumID=properties->configValue(Properties::sb_unknown_album_id).toInt();
    SBIDAlbumPtr albumPtr=SBIDAlbum::retrieveAlbum(albumID,1);

    if(!albumPtr)
    {
        Common::sb_parameters p;
        p.albumTitle="UNKNOWN ALBUM";
        p.notes="Used for unknown albums";
        p.year=1900;

        albumPtr=amgr->createInDB(p);
    }
    return  albumPtr;
}

///	Static methods
SBSqlQueryModel*
SBIDAlbum::albumsByPerformer(int performerID)
{
    QString q=QString
    (
        "SELECT DISTINCT "
            "r.record_id, "
            "r.artist_id, "
            "r.title, "
            "r.genre, "
            "r.year, "
            "r.notes "
        "FROM "
            "___SB_SCHEMA_NAME___record r "
                "INNER JOIN ___SB_SCHEMA_NAME___artist a ON "
                    "r.artist_id=a.artist_id "
        "WHERE "
            "r.artist_id=%1 "
    )
        .arg(performerID)
    ;

    qDebug() << SB_DEBUG_INFO << q;
    return new SBSqlQueryModel(q);
}

///	Protected methods
SBIDAlbum::SBIDAlbum():SBIDBase()
{
    _init();
}

SBIDAlbum&
SBIDAlbum::operator=(const SBIDAlbum& t)
{
    _copy(t);
    return *this;
}

SBIDAlbumPtr
SBIDAlbum::createInDB(Common::sb_parameters& p)
{
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
    QString q;

    if(p.albumTitle.length()==0)
    {
        //	Give new playlist unique name

        int maxNum=1;
        q=QString("SELECT title FROM ___SB_SCHEMA_NAME___record WHERE name %1 \"New Album%\"").arg(dal->getILike());
        dal->customize(q);
        qDebug() << SB_DEBUG_INFO << q;
        QSqlQuery qName(q,db);

        while(qName.next())
        {
            p.albumTitle=qName.value(0).toString();
            p.albumTitle.replace("New Album ","");
            int i=p.albumTitle.toInt();
            if(i>=maxNum)
            {
                maxNum=i+1;
            }
        }
        p.albumTitle=QString("New Album %1").arg(maxNum);
    }

    if(p.performerID==-1)
    {
        //	Find performer 'VARIOUS ARTISTS' and set performerID
        SBIDPerformerPtr peptr=SBIDPerformer::retrieveVariousPerformers();
        p.performerID=peptr->performerID();
    }

    //	Insert
    q=QString
    (
        "INSERT INTO ___SB_SCHEMA_NAME___record "
        "( "
            "artist_id, "
            "title, "
            "media, "
            "genre, "
            "notes, "
            "year "
        ") "
        "VALUES "
        "( "
            "%1, "
            "'%2', "
            "'%3', "
            "'%4', "
            "'%5', "
            "%6 "
        ") "
    )
        .arg(p.performerID)
        .arg(Common::escapeSingleQuotes(p.albumTitle))
        .arg("CD")
        .arg(p.genre)
        .arg(p.notes)
        .arg(p.year)
    ;

    dal->customize(q);
    qDebug() << SB_DEBUG_INFO << q;
    QSqlQuery insert(q,db);
    Q_UNUSED(insert);

    //	Instantiate
    SBIDAlbum album;
    album._albumID         =dal->retrieveLastInsertedKey();
    album._albumPerformerID=p.performerID;
    album._albumTitle      =p.albumTitle;
    album._genre           =p.genre;
    album._notes           =p.notes;
    album._year            =p.year;

    //	Done
    return std::make_shared<SBIDAlbum>(album);
}

SBSqlQueryModel*
SBIDAlbum::find(const Common::sb_parameters& tobeFound,SBIDAlbumPtr existingAlbumPtr)
{
    int excludeID=(existingAlbumPtr?existingAlbumPtr->albumID():-1);

    qDebug() << SB_DEBUG_INFO
             << tobeFound.albumID
             << tobeFound.albumTitle
             << tobeFound.performerID
             << tobeFound.performerName
    ;

    //	MatchRank:
    //	0	-	exact match with specified performer (0 or 1)
	//	1	-	other match with specified performer (0 or more)
    //	2	-	exact match with any other artist (0 or more)
    //	3	-	other match with any other artist (0 or more)
    QString q=QString
    (
        "SELECT "
            "CASE WHEN a.artist_id=%2 OR REPLACE(LOWER(a.name),' ','')=REPLACE(LOWER('%6'),' ','') THEN 0 ELSE 2 END AS matchRank, "
            "p.record_id, "
            "a.artist_id, "
            "p.title, "
            "p.year, "
            "p.genre, "
            "p.notes "
        "FROM "
            "___SB_SCHEMA_NAME___record p "
                "JOIN ___SB_SCHEMA_NAME___artist a ON "
                    "p.artist_id=a.artist_id "
        "WHERE "
            "REPLACE(LOWER(p.title),' ','') = REPLACE(LOWER('%1'),' ','') AND "
            "p.record_id!=(%3) "
        "UNION "
        "SELECT "
            "CASE WHEN a.artist_id=%2 THEN 1 ELSE 3 END AS matchRank, "
            "p.record_id, "
            "a.artist_id, "
            "p.title, "
            "p.year, "
            "p.genre, "
            "p.notes "
        "FROM "
            "___SB_SCHEMA_NAME___record p "
                "JOIN ___SB_SCHEMA_NAME___artist a ON "
                    "p.artist_id=a.artist_id, "
            "article t "
        "WHERE "
            "p.record_id!=(%3) AND "
            "( "
                "p.title!=('%5') AND "
                "LENGTH(p.title)>LENGTH(t.word) AND "
                "LOWER(SUBSTR(p.title,1,LENGTH(t.word))) || ' '= t.word || ' ' AND "
                "LOWER(SUBSTR(p.title,LENGTH(t.word)+2))=LOWER('%4') "
            ") "
            "OR "
            "( "
                "LENGTH(p.title)>LENGTH(t.word) AND "
                "LOWER(SUBSTR(p.title,1,LENGTH('%4')))=LOWER('%4') AND "
                "( "
                    "LOWER(SUBSTR(p.title,LENGTH(a.name)-LENGTH(t.word)+0))=' '||LOWER(t.word) OR "
                    "LOWER(SUBSTR(p.title,LENGTH(a.name)-LENGTH(t.word)+0))=','||LOWER(t.word)  "
                ") "
            ") "
        "ORDER BY "
            "1 "
    )
        .arg(Common::escapeSingleQuotes(Common::removeAccents(tobeFound.albumTitle)))
        .arg(tobeFound.performerID)
        .arg(excludeID)
        .arg(Common::escapeSingleQuotes(Common::removeArticles(tobeFound.albumTitle)))
        .arg(Common::escapeSingleQuotes(tobeFound.albumTitle))
        .arg(Common::escapeSingleQuotes(Common::removeAccents(tobeFound.performerName)))
    ;
    return new SBSqlQueryModel(q);
}

SBIDAlbumPtr
SBIDAlbum::instantiate(const QSqlRecord &r)
{
    SBIDAlbum album;
    int i=0;

    album._albumID         =Common::parseIntFieldDB(&r,i++);
    album._albumPerformerID=Common::parseIntFieldDB(&r,i++);
    album._albumTitle      =r.value(i++).toString();
    album._genre           =r.value(i++).toString();
    album._year            =r.value(i++).toInt();
    album._notes           =r.value(i++).toString();

    album._year=(album._year<1900?1900:album._year);
    return std::make_shared<SBIDAlbum>(album);
}

void
SBIDAlbum::mergeFrom(SBIDAlbumPtr& aPtrFrom)
{
    CacheManager* cm=Context::instance()->cacheManager();
    CacheAlbumPerformanceMgr* apmgr=cm->albumPerformanceMgr();

    //	Find next albumPosition
    int nextAlbumPosition=0;
    QMapIterator<int,SBIDAlbumPerformancePtr> to(albumPerformances());
    while(to.hasNext())
    {
        to.next();
        SBIDAlbumPerformancePtr apPtr=to.value();
        nextAlbumPosition=apPtr->albumPosition()>nextAlbumPosition?apPtr->albumPosition():nextAlbumPosition;
    }
    nextAlbumPosition++;

    qDebug() << SB_DEBUG_INFO << aPtrFrom->albumID() << this->albumID() << nextAlbumPosition;

    //	Go thu each albumPerformance in from and merge
    QList<int> oldAlbumPositions;
    QMapIterator<int,SBIDAlbumPerformancePtr> from(aPtrFrom->albumPerformances());
    while(from.hasNext())
    {
        from.next();
        SBIDAlbumPerformancePtr fromApPtr=from.value();
        oldAlbumPositions.append(fromApPtr->albumPosition());
    }

    qSort(oldAlbumPositions);
    from.toFront();
    while(from.hasNext())
    {
        from.next();
        SBIDAlbumPerformancePtr fromApPtr=from.value();
        SBIDAlbumPerformancePtr toApPtr=_findAlbumPerformanceBySongPerformanceID(fromApPtr->songPerformanceID());
        if(toApPtr)
        {
            qDebug() << SB_DEBUG_INFO << fromApPtr->albumPerformanceID() << toApPtr->albumPerformanceID();
            apmgr->merge(fromApPtr,toApPtr);
        }
        else
        {
            qDebug() << SB_DEBUG_INFO << fromApPtr->albumPosition() << nextAlbumPosition;
            //	Append
            fromApPtr->setAlbumPosition(nextAlbumPosition+oldAlbumPositions.indexOf(fromApPtr->albumPosition()));
            fromApPtr->setAlbumID(this->albumID());
            apmgr->debugShow("apmgr:merge");
            _addedAlbumPerformances.append(fromApPtr);
        }
    }

    //	Go through playlist items and replace
    SBSqlQueryModel* qm=SBIDPlaylistDetail::playlistDetailsByAlbum(aPtrFrom->albumID());
    CachePlaylistDetailMgr* pdmgr=cm->playlistDetailMgr();
    SB_RETURN_VOID_IF_NULL(qm);
    SB_RETURN_VOID_IF_NULL(pdmgr);

    for(int i=0;i<qm->rowCount();i++)
    {
        int playlistDetailID=qm->record(i).value(0).toInt();
        SBIDPlaylistDetailPtr pdPtr=SBIDPlaylistDetail::retrievePlaylistDetail(playlistDetailID);
        if(pdPtr)
        {
            pdPtr->setAlbumID(this->albumID());
        }
    }
}

void
SBIDAlbum::openKey(const QString& key, int& albumID)
{
    QStringList l=key.split(":");
    albumID=l.count()==2?l[1].toInt():-1;
}

SBSqlQueryModel*
SBIDAlbum::retrieveSQL(const QString& key)
{
    int albumID=-1;
    openKey(key,albumID);
    QString q=QString
    (
        "SELECT DISTINCT "
            "r.record_id, "
            "r.artist_id, "
            "r.title, "
            "r.genre, "
            "r.year, "
            "r.notes "
        "FROM "
            "___SB_SCHEMA_NAME___record r "
        "%1 "
        "LIMIT 1 "
    )
        .arg(key.length()==0?"":QString("WHERE r.record_id=%1").arg(albumID))
    ;

    qDebug() << SB_DEBUG_INFO << q;
    return new SBSqlQueryModel(q);
}

QStringList
SBIDAlbum::updateSQL(const Common::db_change db_change) const
{
    QStringList SQL;

    qDebug() << SB_DEBUG_INFO
             << key()
             << deletedFlag()
             << changedFlag()
    ;

    if(deletedFlag() && db_change==Common::db_delete)
    {
        //	Do not remove descendants, this needs to be taken care of by SBIDMgrs.
        SQL.append(QString(
            "DELETE FROM "
                "___SB_SCHEMA_NAME___record "
            "WHERE "
                "record_id=%1 "
        )
            .arg(this->_albumID)
        );
    }
    else if(changedFlag() && db_change==Common::db_update)
    {
        SQL.append(QString(
            "UPDATE ___SB_SCHEMA_NAME___record "
            "SET "
                "artist_id=%1, "
                "title='%2', "
                "year=%3, "
                "notes='%4', "
                "genre='%5' "
            "WHERE "
                "record_id=%6 "
        )
            .arg(this->_albumPerformerID)
            .arg(Common::escapeSingleQuotes(this->_albumTitle))
            .arg(this->_year)
            .arg(Common::escapeSingleQuotes(this->_notes))
            .arg(Common::escapeSingleQuotes(this->_genre))
            .arg(this->_albumID)
        );

        SQL.append(_updateSQLAlbumPerformances());
    }

    return SQL;
}

Common::result
SBIDAlbum::userMatch(const Common::sb_parameters &p, SBIDAlbumPtr exclude, SBIDAlbumPtr& found)
{
    qDebug() << SB_DEBUG_INFO;
    CacheManager* cm=Context::instance()->cacheManager();
    CacheAlbumMgr* amgr=cm->albumMgr();
    Common::result result=Common::result_canceled;
    QMap<int,QList<SBIDAlbumPtr>> matches;

    if(amgr->find(p,exclude,matches))
    {
        int totalMatches=0;
        QMapIterator<int,QList<SBIDAlbumPtr>> itTMP(matches);
        while(itTMP.hasNext())
        {
            itTMP.next();
            int i=itTMP.key();
            totalMatches+=matches[i].count();
            qDebug()<< SB_DEBUG_INFO << i << matches[i].count();
        }

        if(matches[0].count()==1)
        {
            qDebug()<< SB_DEBUG_INFO;
            //	Dataset indicates an exact match if the 2nd record identifies an exact match.
            found=matches[0][0];
            result=Common::result_exists;
        }
        else if(totalMatches==1 && matches[2].count()==1)
        {
            qDebug()<< SB_DEBUG_INFO;
            //	Catch collection album as the one and only choice.
            SBIDAlbumPtr aPtr=matches[2][0];
            SBIDPerformerPtr vpPtr=SBIDPerformer::retrieveVariousPerformers();
            if(aPtr->albumPerformerID()==vpPtr->performerID())
            {
            qDebug()<< SB_DEBUG_INFO;
                found=aPtr;
                result=Common::result_exists;
            }
        }

        if(!found)
        {
            qDebug()<< SB_DEBUG_INFO;
            //	Dataset has at least two records, of which the 2nd one is an soundex match,
            //	display pop-up
            SBDialogSelectItem* pu=SBDialogSelectItem::selectAlbum(p,exclude,matches);
            pu->exec();

            //	Go back to screen if no item has been selected
            if(pu->hasSelectedItem()!=0)
            {
                SBIDPtr selected=pu->getSelected();
                if(selected)
                {
                    //	Existing album is choosen
                    found=SBIDAlbum::retrieveAlbum(selected->itemID());
                    found->refreshDependents();
                    result=Common::result_exists;
                }
                else
                {
                    result=Common::result_missing;
                }
            }
        }
    }
    else
    {
            qDebug()<< SB_DEBUG_INFO;
        result=Common::result_missing;
    }
    return result;
}

void
SBIDAlbum::clearChangedFlag()
{
    //	CWIP: unsure how to handle this (restructuring @ SBIDOnlinePerformance 5/15)
    SBIDBase::clearChangedFlag();
//    foreach(SBIDAlbumPerformancePtr performancePtr,_albumPerformances)
//    {
//        performancePtr->clearChangedFlag();
//    }
    //	AlbumPerformances are owned by SBIDAlbum -- don't clear these
}

void
SBIDAlbum::rollback()
{
    SBIDBase::rollback();
}

///	Private methods
void
SBIDAlbum::_copy(const SBIDAlbum &t)
{
    _albumID                            =t._albumID;
    _albumPerformerID                   =t._albumPerformerID;
    _albumTitle                         =t._albumTitle;
    _genre                              =t._genre;
    _notes                              =t._notes;
    _year                               =t._year;

    _albumPerformances                  =t._albumPerformances;
    _addedAlbumPerformances             =t._addedAlbumPerformances;
    _removedAlbumPerformances           =t._removedAlbumPerformances;
}

void
SBIDAlbum::_init()
{
    _sb_item_type=Common::sb_type_album;

    _albumID=-1;
    _albumPerformerID=-1;
    _albumTitle=QString();
    _genre=QString();
    _notes=QString();
    _year=-1;

    _albumPerformances.clear();
    _addedAlbumPerformances.clear();
    _removedAlbumPerformances.clear();
}

SBIDAlbumPerformancePtr
SBIDAlbum::_findAlbumPerformanceBySongPerformanceID(int songPerformanceID) const
{
    SBIDAlbumPerformancePtr apPtr;

    QMapIterator<int,SBIDAlbumPerformancePtr> apIT(albumPerformances());
    while(!apPtr && apIT.hasNext())
    {
        apIT.next();
        SBIDAlbumPerformancePtr ptr=apIT.value();
        apPtr=(ptr && ptr->songPerformanceID()==songPerformanceID)?ptr:SBIDAlbumPerformancePtr();
    }

    return apPtr;
}

void
SBIDAlbum::_loadAlbumPerformances()
{
    _albumPerformances=_loadAlbumPerformancesFromDB();
}

QMap<int,SBIDAlbumPerformancePtr>
SBIDAlbum::_loadAlbumPerformancesFromDB() const
{
    return Preloader::performanceMap(SBIDAlbumPerformance::performancesByAlbum_Preloader(this->albumID()));
}

QStringList
SBIDAlbum::_updateSQLAlbumPerformances() const
{
    QStringList SQL;

//    QVectorIterator<SBIDOnlinePerformancePtr> opIT(albumPerformances());
//    while(opIT.hasNext())
//    {
//        qDebug() << SB_DEBUG_INFO;

//        SQL.append(opIT.next()->updateSQL());
//    }
    return SQL;
}

void
SBIDAlbum::_showAlbumPerformances(const QString& title) const
{
    QMapIterator<int,SBIDAlbumPerformancePtr> apIT(albumPerformances());
    qDebug() << SB_DEBUG_INFO << title;
    while(apIT.hasNext())
    {
        apIT.next();
        const SBIDAlbumPerformancePtr apPtr=apIT.value();

        qDebug() << SB_DEBUG_INFO << apPtr->operator QString();
        ;
    }
}
