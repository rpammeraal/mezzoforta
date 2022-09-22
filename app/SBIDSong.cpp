#include <QDebug>

#include "SBIDSong.h"

#include "CacheManager.h"
#include "Context.h"
#include "DataAccessLayer.h"
#include "Preloader.h"
#include "SBDialogSelectItem.h"
#include "SBTableModel.h"

///	Ctors
SBIDSong::SBIDSong(SBIDSong &c):SBIDBase(c)
{
    _copy(c);
}

SBIDSong::~SBIDSong()
{

}

///	Public methods
int
SBIDSong::commonPerformerID() const
{
    return this->songOriginalPerformerID();
}

QString
SBIDSong::commonPerformerName() const
{
    return this->songOriginalPerformerName();
}

QString
SBIDSong::genericDescription() const
{
    return QString("Song - %1 / %2 (Not Available)")
        .arg(this->text())
        .arg(this->songOriginalPerformerName())
    ;
}

QString
SBIDSong::iconResourceLocation() const
{
    return iconResourceLocationStatic();
}

QMap<int,SBIDOnlinePerformancePtr>
SBIDSong::onlinePerformances(bool updateProgressDialogFlag) const
{
    QMap<int,SBIDOnlinePerformancePtr> list;
    const SBIDSongPerformancePtr spPtr=originalSongPerformancePtr();
    if(spPtr)
    {
        list=spPtr->onlinePerformances(updateProgressDialogFlag);
    }
    //	CWIP: if !spPtr, find other songPerformance that can be played
    return list;
}

void
SBIDSong::sendToPlayQueue(bool enqueueFlag)
{
    const SBIDSongPerformancePtr spPtr=originalSongPerformancePtr();
    if(spPtr)
    {
        spPtr->sendToPlayQueue(enqueueFlag);
    }
    //	CWIP: if !spPtr, find other songPerformance that can be played
}

void
SBIDSong::setToReloadFlag()
{
    SBIDBase::setToReloadFlag();
    QMapIterator<int,SBIDSongPerformancePtr> it(songPerformances());
    while(it.hasNext())
    {
        it.next();
        int performerID=it.key();
        SBIDPerformerPtr pPtr=SBIDPerformer::retrievePerformer(performerID);
        if(pPtr)
        {
            pPtr->setToReloadFlag();
        }
    }
}

QString
SBIDSong::text() const
{
    return this->_songTitle;
}

QString
SBIDSong::type() const
{
    return QString("song");
}


///	Song specific methods
SBTableModel*
SBIDSong::albums()
{
    SBTableModel* tm=new SBTableModel();
    getSemaphore();
    if(_albumPerformances.count()==0)
    {
        const_cast<SBIDSong *>(this)->_loadAlbumPerformances();
    }
    tm->populateAlbumsBySong(_albumPerformances);
    releaseSemaphore();
    return tm;
}

void
SBIDSong::addSongPerformance(int performerID,int year,const QString& notes)
{
    CacheManager* cm=Context::instance()->cacheManager();
    CacheSongPerformanceMgr* spMgr=cm->songPerformanceMgr();
    SBIDSongPerformancePtr spPtr;
    Q_UNUSED(performerID);
    Q_UNUSED(year);
    Q_UNUSED(notes);

    getSemaphore();
    if(_songPerformances.count()==0)
    {
        this->_loadSongPerformances();
    }

    if(!_songPerformances.contains(performerID))
    {

        Common::sb_parameters p;
        p.songID=this->songID();
        p.performerID=performerID;
        p.year=year;
        p.notes=notes;

        spPtr=spMgr->createInDB(p);
    }
    releaseSemaphore();

    if(spPtr)
    {
        addSongPerformance(spPtr);
    }
}

SBIDSongPerformancePtr
SBIDSong::addSongPerformance(SBIDSongPerformancePtr spPtr)
{
    getSemaphore();
    if(_songPerformances.count()==0)
    {
        this->_loadSongPerformances();
    }

    SB_RETURN_IF_NULL(spPtr,SBIDSongPerformancePtr());
    if(!_songPerformances.contains(spPtr->songPerformanceID()))
    {
        _songPerformances[spPtr->songPerformerID()]=spPtr;
    }
    else
    {
        spPtr=_songPerformances[spPtr->songPerformerID()];
    }
    releaseSemaphore();
    return spPtr;
}

QVector<SBIDAlbumPerformancePtr>
SBIDSong::allPerformances()
{
    if(_albumPerformances.count()==0)
    {
        const_cast<SBIDSong *>(this)->refreshDependents();
    }
    return QVector<SBIDAlbumPerformancePtr>(_albumPerformances);
}

SBTableModel*
SBIDSong::charts() const
{
    SBTableModel* tm=new SBTableModel();
    QMap<SBIDChartPerformancePtr,SBIDChartPtr> chartToPerformances=Preloader::chartItems(*this);
    tm->populateChartsByItemType(SBKey::Song,chartToPerformances);
    return tm;
}

void
SBIDSong::deleteIfOrphanized()
{
    DataAccessLayer* dal=Context::instance()->dataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
    int usageCount=0;

    QString q;
    QStringList table;
    table.append("chart");
    table.append("collection");
    table.append("record");
    table.append("online");
    table.append("playlist");
    QStringListIterator it(table);
    while(it.hasNext())
    {
        QString t=it.next();
        if(q.length())
        {
            q+=" + ";
        }
        q+=QString("(SELECT COUNT(*) FROM ___SB_SCHEMA_NAME___%1_performance WHERE song_id=%2) ").arg(t).arg(this->itemID());
    }
    q="SELECT "+q;

    dal->customize(q);

    QSqlQuery query(q,db);

    if(query.next())
    {
        usageCount=query.value(0).toInt();
    }
    if(usageCount==0)
    {
        QStringList SQL;

        //	No usage anywhere. Remove song, performance, lyrics, toplay
        table.clear();
        table.append("toplay");
        table.append("lyrics");
        table.append("performance");
        table.append("song");

        QStringListIterator it(table);
        while(it.hasNext())
        {
            QString t=it.next();
            SQL.append(QString("DELETE FROM ___SB_SCHEMA_NAME___%1 WHERE song_id=%2").arg(t).arg(itemID()));
        }
        dal->executeBatch(SQL);
    }
}

int
SBIDSong::numAlbumPerformances()
{
    getSemaphore();
    if(_albumPerformances.count()==0)
    {
        const_cast<SBIDSong *>(this)->_loadAlbumPerformances();
    }
    releaseSemaphore();
    return _albumPerformances.count();
}

SBTableModel*
SBIDSong::playlists()
{
    getSemaphore();
    if(!_playlistOnlinePerformances.count())
    {
        //	Playlists may not be loaded -- retrieve again
        this->_loadPlaylists();
    }
    SBTableModel* tm=new SBTableModel();
    tm->populatePlaylists(_playlistOnlinePerformances);
    releaseSemaphore();
    return tm;
}

QMap<int,SBIDSongPerformancePtr>
SBIDSong::songPerformances()
{
    getSemaphore();
    if(_songPerformances.count()==0)
    {
        const_cast<SBIDSong *>(this)->_loadSongPerformances();
    }
    releaseSemaphore();
    return _songPerformances;
}

QVector<SBIDOnlinePerformancePtr>
SBIDSong::onlinePerformancesPreloader() const
{
    return Preloader::onlinePerformances(SBIDOnlinePerformance::onlinePerformancesBySong_Preloader(this->songID()));
}

SBIDAlbumPerformancePtr
SBIDSong::performance(int albumID, int albumPosition)
{
    SBIDAlbumPerformancePtr apPtr;

    getSemaphore();
    if(_albumPerformances.count()==0)
    {
        const_cast<SBIDSong *>(this)->_loadAlbumPerformances();
    }

    for(int i=0;i<_albumPerformances.size() && !apPtr;i++)
    {
        int currentAlbumID=_albumPerformances.at(i)->albumID();
        int currentAlbumPosition=_albumPerformances.at(i)->albumPosition();

        if(currentAlbumID==albumID && currentAlbumPosition==albumPosition)
        {
            apPtr=_albumPerformances.at(i);
        }
    }
    releaseSemaphore();

    return apPtr;
}

QVector<int>
SBIDSong::performerIDList()
{
    QVector<int> list;

    getSemaphore();
    if(_songPerformances.count()==0)
    {
        const_cast<SBIDSong *>(this)->_loadSongPerformances();
    }

    QMapIterator<int,SBIDSongPerformancePtr> _spIT(_songPerformances);
    while(_spIT.hasNext())
    {
        _spIT.next();
        const int performerID=_spIT.key();
        if(!list.contains(performerID))
        {
            list.append(performerID);
        }
    }
    releaseSemaphore();
    return list;
}

void
SBIDSong::updateSoundexFields()
{
    DataAccessLayer* dal=Context::instance()->dataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());

    QString q=QString
    (
        "SELECT DISTINCT "
            "s.title "
        "FROM "
            "___SB_SCHEMA_NAME___song s "
        "WHERE "
            "s.soundex IS NULL "
        "ORDER BY "
            "s.title "
    );

    QSqlQuery q1(db);
    q1.exec(dal->customize(q));

    QString title;
    QString soundex;
    while(q1.next())
    {
        title=q1.value(0).toString();
        soundex=Common::soundex(title);

        q=QString
        (
            "UPDATE "
                "___SB_SCHEMA_NAME___song "
            "SET "
                "soundex='%1'"
            "WHERE "
                "title='%2'"
        )
            .arg(soundex)
            .arg(Common::escapeSingleQuotes(title))
        ;
        dal->customize(q);
        qDebug() << SB_DEBUG_INFO << q;

        QSqlQuery q2(q,db);
        q2.exec();
    }
}

///	Setters, Changers
void
SBIDSong::removeSongPerformance(SBIDSongPerformancePtr spPtr)
{
    getSemaphore();
    if(_songPerformances.contains(spPtr->songPerformerID()))
    {
        _songPerformances.remove(spPtr->songPerformerID());
    }
    spPtr->setDeletedFlag();
    releaseSemaphore();
}

///	Pointers
SBIDSongPerformancePtr
SBIDSong::originalSongPerformancePtr() const
{
    CacheManager* cm=Context::instance()->cacheManager();
    CacheSongPerformanceMgr* spMgr=cm->songPerformanceMgr();
    return spMgr->retrieve(SBIDSongPerformance::createKey(_originalSongPerformanceID));
}

///	Redirectors
QString
SBIDSong::songOriginalPerformerName() const
{
    SBIDSongPerformancePtr spPtr=originalSongPerformancePtr();
    return (spPtr?spPtr->songPerformerName():QString());
}

int
SBIDSong::songOriginalPerformerID() const
{
    SBIDSongPerformancePtr spPtr=originalSongPerformancePtr();
    return (spPtr?spPtr->songPerformerID():-1);
}

int
SBIDSong::songOriginalYear() const
{
    SBIDSongPerformancePtr spPtr=originalSongPerformancePtr();
    return (spPtr?spPtr->year():-1);
}

///	Operators
SBIDSong::operator QString() const
{
    return QString("SBIDSong:sID=%1:t=%2:oSPID=%3")
            .arg(itemID())
            .arg(_songTitle)
            .arg(_originalSongPerformanceID)
    ;
}

//	Methods required by SBIDManagerTemplate
SBKey
SBIDSong::createKey(int songID)
{
    return SBKey(SBKey::Song,songID);
}

void
SBIDSong::refreshDependents(bool forcedFlag)
{
    getSemaphore();
    if(forcedFlag==1 || _albumPerformances.count()==0)
    {
        _loadAlbumPerformances();
    }
    if(forcedFlag==1 || _playlistOnlinePerformances.count()==0)
    {
        _loadPlaylists();
    }
    if(forcedFlag==1 || _songPerformances.count()==0)
    {
        _loadSongPerformances();
    }
    releaseSemaphore();
}


//	Static methods
SBSqlQueryModel*
SBIDSong::retrieveAllSongs()
{
    //	List songs with actual online performance only
    QString q=QString
    (
        "SELECT "
            "s.title || ' ' || COALESCE(a.name,'') || ' ' || COALESCE(r.title,'')  AS SB_KEYWORDS, "
            "CAST(%1 AS VARCHAR)||':'||CAST(s.song_id AS VARCHAR) AS SB_ITEM_KEY1, "
            "s.title AS song, "
            "CAST(%2 AS VARCHAR)||':'||CAST(a.artist_id AS VARCHAR) AS SB_ITEM_KEY2, "
            "COALESCE(a.name,'n/a') AS performer, "
            "CAST(%3 AS VARCHAR)||':'||CAST(r.record_id AS VARCHAR) AS SB_ITEM_KEY3, "
            "COALESCE(r.title,'n/a') AS album "
        "FROM "
            "___SB_SCHEMA_NAME___song s "
                "LEFT JOIN ___SB_SCHEMA_NAME___performance p ON "
                    "s.song_id = p.song_id "
                "LEFT JOIN ___SB_SCHEMA_NAME___artist a ON "
                    "p.artist_id=a.artist_id "
                "LEFT JOIN ___SB_SCHEMA_NAME___record_performance rp ON "
                    "p.preferred_record_performance_id=rp.record_performance_id "
                "LEFT JOIN ___SB_SCHEMA_NAME___record r ON "
                    "rp.record_id=r.record_id "
        "ORDER BY "
            "3,5,7 "

    )
        .arg(SBKey::Song)
        .arg(SBKey::Performer)
        .arg(SBKey::Album)
    ;

    return new SBSqlQueryModel(q);
}

SBIDSongPtr
SBIDSong::retrieveSong(SBKey key)
{
    SBIDSongPtr sPtr;

    if(key.validFlag())
    {
        if(key.itemType()==SBKey::Song)
        {
            CacheManager* cm=Context::instance()->cacheManager();
            CacheSongMgr* smgr=cm->songMgr();
            sPtr=smgr->retrieve(key);
        }
        else if(key.itemType()==SBKey::AlbumPerformance)
        {
            SBIDAlbumPerformancePtr apPtr=SBIDAlbumPerformance::retrieveAlbumPerformance(key);
            SB_RETURN_IF_NULL(apPtr,SBIDSongPtr());
            sPtr=apPtr->songPtr();
        }
        else if(key.itemType()==SBKey::OnlinePerformance)
        {
            SBIDOnlinePerformancePtr opPtr=SBIDOnlinePerformance::retrieveOnlinePerformance(key);
            SB_RETURN_IF_NULL(opPtr,SBIDSongPtr());
            sPtr=opPtr->songPtr();
        }
        else if(key.itemType()==SBKey::SongPerformance)
        {
            SBIDSongPerformancePtr opPtr=SBIDSongPerformance::retrieveSongPerformance(key);
            SB_RETURN_IF_NULL(opPtr,SBIDSongPtr());
            sPtr=opPtr->songPtr();
        }
        else if(key.itemType()==SBKey::ChartPerformance)
        {
            SBIDChartPerformancePtr cpPtr=SBIDChartPerformance::retrieveChartPerformance(key);
            SB_RETURN_IF_NULL(cpPtr,SBIDSongPtr());
            sPtr=cpPtr->songPtr();
        }
        else
        {
            qDebug() << SB_DEBUG_ERROR << "cannot determine song from" << key;
        }
    }
    return sPtr;
}

SBIDSongPtr
SBIDSong::retrieveSong(int songID)
{
    return retrieveSong(createKey(songID));
}

QString
SBIDSong::iconResourceLocationStatic()
{
    return QString(":/images/SongIcon.png");
}

///	Protected methods
SBIDSong::SBIDSong():SBIDBase(SBKey::Song,-1)
{
    _init();
}

SBIDSong::SBIDSong(int songID):SBIDBase(SBKey::Song,songID)
{
    _init();
}

SBIDSong&
SBIDSong::operator=(SBIDSong& t)
{
    _copy(t);
    return *this;
}

SBIDSongPtr
SBIDSong::createInDB(Common::sb_parameters& p)
{
    DataAccessLayer* dal=Context::instance()->dataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
    QString q;

    if(p.songTitle.length()==0)
    {
        //	Give new song unique name
        int maxNum=1;
        q=QString("SELECT title FROM ___SB_SCHEMA_NAME___song WHERE name %1 \"New Song%\"")
            .arg(dal->getILike())
        ;
        dal->customize(q);
        qDebug() << SB_DEBUG_INFO << q;
        QSqlQuery qName(q,db);

        while(qName.next())
        {
            p.songTitle=qName.value(0).toString();
            p.songTitle.replace("New Song ","");
            int i=p.songTitle.toInt();
            if(i>=maxNum)
            {
                maxNum=i+1;
            }
        }
        p.songTitle=QString("New Song %1").arg(maxNum);
    }

    //	Insert
    q=QString
    (
        "INSERT INTO ___SB_SCHEMA_NAME___song "
        "( "
            "title, "
            "notes, "
            "soundex "
        ") "
        "SELECT "
            "'%1', "
            "'%2', "
            "'%3' "
    )
        .arg(Common::escapeSingleQuotes(p.songTitle))
        .arg(Common::escapeSingleQuotes(p.notes))
        .arg(Common::soundex(p.songTitle))
    ;
    dal->customize(q);
    qDebug() << SB_DEBUG_INFO << q;
    QSqlQuery insert(q,db);
    Q_UNUSED(insert);

    //	Instantiate
    SBIDSong s(dal->retrieveLastInsertedKey());
    s._songTitle=p.songTitle;
    s._notes    =p.notes;

    //	Done
    return std::make_shared<SBIDSong>(s);
}

SBSqlQueryModel*
SBIDSong::find(const Common::sb_parameters& p,SBIDSongPtr existingSongPtr)
{
    QString newSoundex=Common::soundex(p.songTitle);
    int excludeSongID=(existingSongPtr?existingSongPtr->songID():-1);

    //	MatchRank:
    //	0	-	exact match with specified artist (0 or 1 in data set).
    //	0	-	exact match based on ID's
    //	1	-	exact match on song title with any other artist (0 or more in data set).
    //	2	-	soundex match with any other artist (0 or more in data set).
    QString q=QString
    (
        "SELECT "
            "CASE WHEN p.artist_id=%2 THEN 0 ELSE 1 END AS matchRank, "
            "s.song_id, "
            "s.title, "
            "s.notes, "
            "l.lyrics, "
            "s.original_performance_id "
        "FROM "
            "___SB_SCHEMA_NAME___song s "
                "LEFT JOIN ___SB_SCHEMA_NAME___performance p ON "
                    "p.song_id=s.song_id "
                "LEFT JOIN ___SB_SCHEMA_NAME___lyrics l ON "
                    "s.song_id=l.song_id "
        "WHERE "
            "s.original_performance_id IS NOT NULL AND "
            "( "
                "REPLACE(LOWER(title),' ','') = '%1' OR "
                "s.song_id=%5 "
            ") "
            "%4 "
        "UNION "
        "SELECT "
            "2 AS matchRank, "
            "s.song_id, "
            "s.title, "
            "s.notes, "
            "l.lyrics, "
            "s.original_performance_id "
        "FROM "
            "___SB_SCHEMA_NAME___song s "
                "LEFT JOIN ___SB_SCHEMA_NAME___lyrics l ON "
                    "s.song_id=l.song_id "
        "WHERE "
            "s.original_performance_id IS NOT NULL AND "
            "( "
                "SUBSTR(s.soundex,1,LENGTH('%3'))='%3'  "
                //"SUBSTR('%3',1,LENGTH(s.soundex))=s.soundex "
            ") AND "
            "length(s.soundex)<= 2*length('%3') AND "
            "REPLACE(LOWER(s.title),' ','') != '%1' "
            "%4 "
        "ORDER BY "
            "1,3 "
    )
        .arg(Common::escapeSingleQuotes(Common::comparable(p.songTitle)))
        .arg(p.performerID)
        .arg(newSoundex)
        .arg(excludeSongID==-1?"":QString(" AND s.song_id!=%1").arg(excludeSongID))
        .arg(p.songID)
    ;
    return new SBSqlQueryModel(q);
}

SBIDSongPtr
SBIDSong::instantiate(const QSqlRecord &r)
{
    int i=0;

    SBIDSong song(Common::parseIntFieldDB(&r,i++));
    song._songTitle                   =r.value(i++).toString();
    song._notes                       =r.value(i++).toString();
    song._lyrics                      =r.value(i++).toString();
    song._originalSongPerformanceID   =Common::parseIntFieldDB(&r,i++);

    return std::make_shared<SBIDSong>(song);
}

void
SBIDSong::mergeFrom(SBIDSongPtr &fromPtr)
{
    CacheManager* cm=Context::instance()->cacheManager();
    _loadSongPerformances();	//	make sure list is loaded.

    //	In the rare case that:
    //	-	originalPerformanceID is not set for this
    //	-	there is at least songPerformance,
    //	Set originalPerformanceID
    int originalPerformanceID=this->originalSongPerformanceID();
    if(originalPerformanceID==-1)
    {
        QMapIterator<int,SBIDSongPerformancePtr> it(this->songPerformances());
        if(it.hasNext())
        {
            it.next();
            SBIDSongPerformancePtr spPtr=it.value();
            if(spPtr)
            {
                this->setOriginalPerformanceID(spPtr->songPerformanceID());
            }
        }
    }

    //	SongPerformance
    CacheSongPerformanceMgr *spMgr=cm->songPerformanceMgr();
    QMapIterator<int,SBIDSongPerformancePtr> fromIt(fromPtr->songPerformances());

    getSemaphore();
    while(fromIt.hasNext())
    {
        fromIt.next();
        int performerID=fromIt.key();
        SBIDSongPerformancePtr fromSpPtr=fromIt.value();

        if(_songPerformances.contains(performerID))
        {
            SBIDSongPerformancePtr toSpPtr=_songPerformances[performerID];
            spMgr->merge(fromSpPtr,toSpPtr);
        }
        else
        {
            //	Assign to current
            fromSpPtr->setSongID(this->songID());
            _songPerformances[performerID]=fromSpPtr;
        }
    }

    //	Lyrics
    if(fromPtr->lyrics().length() && this->lyrics().length()==0)
    {
        _lyrics=fromPtr->lyrics();
    }

    //	set originalPerformanceID on the fromPtr
    fromPtr->setOriginalPerformanceID(-1);
    releaseSemaphore();
}

void
SBIDSong::postInstantiate(SBIDSongPtr &ptr)
{
    Q_UNUSED(ptr);
}

SBSqlQueryModel*
SBIDSong::retrieveSQL(SBKey key)
{
    QString q=QString
    (
        "SELECT DISTINCT "
            "s.song_id,"
            "s.title, "
            "s.notes, "
            "l.lyrics, "
            "s.original_performance_id "
        "FROM "
            "___SB_SCHEMA_NAME___song s "
                "LEFT JOIN ___SB_SCHEMA_NAME___lyrics l ON "
                    "s.song_id=l.song_id "
        "%1  "
        "ORDER BY "
            "s.title "
    )
        .arg(key.validFlag()?QString("WHERE s.song_id=%1").arg(key.itemID()):QString())
    ;
    qDebug() << SB_DEBUG_INFO << q;

    return new SBSqlQueryModel(q);
}

QStringList
SBIDSong::updateSQL(const Common::db_change db_change) const
{
    QStringList SQL;

    if(deletedFlag() && db_change==Common::db_delete)
    {
        SQL.append(QString
        (
            "DELETE FROM "
                "___SB_SCHEMA_NAME___lyrics "
            "WHERE "
                "song_id=%1 "
        )
            .arg(this->itemID())
        );
        SQL.append(QString
        (
            "DELETE FROM "
                "___SB_SCHEMA_NAME___song "
            "WHERE "
                "song_id=%1 "
        )
            .arg(this->itemID())
        );
    }
    else if(changedFlag() && db_change==Common::db_update)
    {
        SQL.append(QString
        (
            "UPDATE ___SB_SCHEMA_NAME___song "
            "SET "
                "original_performance_id=CASE WHEN %1=-1 THEN NULL ELSE %1 END, "
                "title='%2', "
                "notes='%3' "
            "WHERE "
                "song_id=%4 "
        )
            .arg(_originalSongPerformanceID)
            .arg(Common::escapeSingleQuotes(this->_songTitle))
            .arg(Common::escapeSingleQuotes(this->_notes))
            .arg(this->itemID())
        );

        SQL.append(QString
        (
            "UPDATE ___SB_SCHEMA_NAME___lyrics "
            "SET "
                "lyrics='%2' "
            "WHERE "
                "song_id=%1 "
        )
            .arg(this->itemID())
            .arg(Common::escapeSingleQuotes(this->_lyrics))
        );
    }
    return SQL;
}

Common::result
SBIDSong::userMatch(const Common::sb_parameters& p, SBIDSongPtr exclude, SBIDSongPtr& found, bool showAllChoicesFlag)
{
    CacheManager* cm=Context::instance()->cacheManager();
    CacheSongMgr* smgr=cm->songMgr();
    Common::result result=Common::result_canceled;
    QMap<int,QList<SBIDSongPtr>> matches;

    if(smgr->find(p,exclude,matches))
    {
        if(matches[0].count()==1)
        {
            //	Dataset indicates an exact match if the 2nd record identifies an exact match.
            found=matches[0][0];
            result=Common::result_exists_derived;
        }
        else if((showAllChoicesFlag==1) || (matches[1].count()>1))
        {
            //	Dataset has at least two records, of which the 2nd one is an soundex match,
            //	display pop-up
            SBDialogSelectItem* pu=SBDialogSelectItem::selectSong(p,exclude,matches);
            pu->exec();

            //	Go back to screen if no item has been selected
            if(pu->hasSelectedItem()!=0)
            {
                SBIDPtr selected=pu->getSelected();
                if(selected)
                {
                    //	Existing song is choosen
                    found=SBIDSong::retrieveSong(selected->itemID());
                    found->refreshDependents();
                    result=Common::result_exists_user_selected;
                }
                else
                {
                    result=Common::result_missing;
                }
            }
        }
        else if(showAllChoicesFlag==0)
        {
            result=Common::result_missing;
        }
    }
    else
    {
        result=Common::result_missing;
    }
    return result;
}

///	Private methods
void
SBIDSong::_copy(SBIDSong &c)
{
    getSemaphore(); c.getSemaphore();
    SBIDBase::_copy(c);
    _songTitle                 =c._songTitle;
    _notes                     =c._notes;
    _lyrics                    =c._lyrics;
    _originalSongPerformanceID =c._originalSongPerformanceID;

    _playlistOnlinePerformances=c._playlistOnlinePerformances;
    _albumPerformances         =c._albumPerformances;
    _songPerformances          =c._songPerformances;
    c.releaseSemaphore();releaseSemaphore();
}

void
SBIDSong::_init()
{
    _songTitle=QString();
    _notes=QString();
    _lyrics=QString();
    _originalSongPerformanceID=-1;
}

void
SBIDSong::_loadAlbumPerformances()
{
    SBSqlQueryModel* qm=SBIDAlbumPerformance::performancesBySong(songID());
    CacheManager* cm=Context::instance()->cacheManager();
    CacheAlbumPerformanceMgr* apmgr=cm->albumPerformanceMgr();

    //	Load performances including dependents, this will set its internal pointers
    _albumPerformances=apmgr->retrieveSet(qm);

    delete qm;
}

void
SBIDSong::_loadPlaylists()
{
    _playlistOnlinePerformances=_loadPlaylistOnlinePerformanceListFromDB();
}

void
SBIDSong::_loadSongPerformances()
{
    _songPerformances=_loadSongPerformancesFromDB();
}

void
SBIDSong::_setSongTitle(const QString &songTitle)
{
    _songTitle=songTitle;
    setChangedFlag();
}

///	Aux methods
QVector<SBIDSong::PlaylistOnlinePerformance>
SBIDSong::_loadPlaylistOnlinePerformanceListFromDB() const
{
    QVector<SBIDSong::PlaylistOnlinePerformance> playlistOnlinePerformanceList;

    DataAccessLayer* dal=Context::instance()->dataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
    QString q=QString
    (
        "SELECT DISTINCT "
            "pp.playlist_id, "
            "op.online_performance_id "
        "FROM "
            "___SB_SCHEMA_NAME___playlist_detail pp "
                "JOIN ___SB_SCHEMA_NAME___online_performance op ON "
                    "pp.online_performance_id=op.online_performance_id "
                "JOIN ___SB_SCHEMA_NAME___record_performance rp ON "
                    "op.record_performance_id=rp.record_performance_id "
                "JOIN ___SB_SCHEMA_NAME___performance p ON "
                    "rp.performance_id=p.performance_id "
                "JOIN ___SB_SCHEMA_NAME___song s ON "
                    "p.song_id=s.song_id "
        "WHERE "
            "p.song_id=%1 "
    )
        .arg(this->songID())
    ;

    qDebug() << SB_DEBUG_INFO << q;
    QSqlQuery query(db);
    query.exec(dal->customize(q));

    //	Set up progress dialog
    int progressCurrentValue=0;
    int progressMaxValue=query.size();
    if(progressMaxValue<0)
    {
        //	Count items
        while(query.next())
        {
            progressMaxValue++;
        }
    }
    ProgressDialog::instance()->update(__SB_PRETTY_FUNCTION__,"step1",progressCurrentValue,progressMaxValue);

    query.first();
    query.previous();
    while(query.next())
    {
        SBIDPlaylistPtr plPtr=SBIDPlaylist::retrievePlaylist(query.value(0).toInt());	//	Note: used to include loading of dependents
        SBIDOnlinePerformancePtr opPtr=SBIDOnlinePerformance::retrieveOnlinePerformance(query.value(1).toInt());	//	Note: used to include loading of dependents

        SBIDSong::PlaylistOnlinePerformance r;
        r.plPtr=plPtr;
        r.opPtr=opPtr;

        playlistOnlinePerformanceList.append(r);
        ProgressDialog::instance()->update(__SB_PRETTY_FUNCTION__,"step1",progressCurrentValue++,progressMaxValue);
    }
    ProgressDialog::instance()->finishStep(__SB_PRETTY_FUNCTION__,"step1");
    return playlistOnlinePerformanceList;
}

QMap<int,SBIDSongPerformancePtr>
SBIDSong::_loadSongPerformancesFromDB() const
{
    SBSqlQueryModel* qm=SBIDSongPerformance::performancesBySong(songID());
    CacheManager* cm=Context::instance()->cacheManager();
    CacheSongPerformanceMgr* spmgr=cm->songPerformanceMgr();

    //	Load performances including dependents, this will set its internal pointers
    QMap<int,SBIDSongPerformancePtr> songPerformances=spmgr->retrieveMap(qm);
    delete qm;
    return songPerformances;
}

