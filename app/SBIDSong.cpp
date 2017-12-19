#include <QDebug>

#include "SBIDSong.h"

#include "CacheManager.h"
#include "Context.h"
#include "DataAccessLayer.h"
#include "Preloader.h"
#include "SBDialogSelectItem.h"
#include "SBTableModel.h"

///	Ctors
SBIDSong::SBIDSong(const SBIDSong &c):SBIDBase(c)
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

SBKey::ItemType
SBIDSong::itemType() const
{
    return SBKey::Song;
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
SBIDSong::albums() const
{
    SBTableModel* tm=new SBTableModel();
    if(_albumPerformances.count()==0)
    {
        const_cast<SBIDSong *>(this)->refreshDependents();
    }
    tm->populateAlbumsBySong(_albumPerformances);
    return tm;
}

SBIDSongPerformancePtr
SBIDSong::addSongPerformance(int performerID,int year,const QString& notes)
{
    CacheManager* cm=Context::instance()->cacheManager();
    CacheSongPerformanceMgr* spMgr=cm->songPerformanceMgr();
    SBIDSongPerformancePtr spPtr;
    Q_UNUSED(performerID);
    Q_UNUSED(year);
    Q_UNUSED(notes);

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
    return addSongPerformance(spPtr);
}

QVector<SBIDAlbumPerformancePtr>
SBIDSong::allPerformances() const
{
    if(_albumPerformances.count()==0)
    {
        const_cast<SBIDSong *>(this)->refreshDependents();
    }
    return _albumPerformances;
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
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
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
    qDebug() << SB_DEBUG_INFO << q;

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
SBIDSong::numAlbumPerformances() const
{
    if(_albumPerformances.count()==0)
    {
        const_cast<SBIDSong *>(this)->_loadAlbumPerformances();
    }
    return _albumPerformances.count();
}

SBTableModel*
SBIDSong::playlists()
{
    if(!_playlistOnlinePerformances.count())
    {
        //	Playlists may not be loaded -- retrieve again
        this->_loadPlaylists();
    }
    SBTableModel* tm=new SBTableModel();
    tm->populatePlaylists(_playlistOnlinePerformances);
    return tm;
}

QMap<int,SBIDSongPerformancePtr>
SBIDSong::songPerformances() const
{
    if(_songPerformances.count()==0)
    {
        const_cast<SBIDSong *>(this)->_loadSongPerformances();
    }
    return _songPerformances;
}

QVector<SBIDOnlinePerformancePtr>
SBIDSong::onlinePerformancesPreloader() const
{
    return Preloader::onlinePerformances(SBIDOnlinePerformance::onlinePerformancesBySong_Preloader(this->songID()));
}

SBIDAlbumPerformancePtr
SBIDSong::performance(int albumID, int albumPosition) const
{
    SBIDAlbumPerformancePtr null;

    if(_albumPerformances.count()==0)
    {
        const_cast<SBIDSong *>(this)->refreshDependents();
    }

    for(int i=0;i<_albumPerformances.size();i++)
    {
        int currentAlbumID=_albumPerformances.at(i)->albumID();
        int currentAlbumPosition=_albumPerformances.at(i)->albumPosition();

        if(currentAlbumID==albumID && currentAlbumPosition==albumPosition)
        {
            return _albumPerformances.at(i);
        }
    }

    return null;
}

QVector<int>
SBIDSong::performerIDList() const
{
    QVector<int> list;

    if(_songPerformances.count()==0)
    {
        const_cast<SBIDSong *>(this)->_loadSongPerformances();
    }

    QMapIterator<int,SBIDSongPerformancePtr> _spIT(_songPerformances);
    while(_spIT.hasNext())
    {
        _spIT.next();
        const int performerID=_spIT.key();
        const SBIDSongPerformancePtr spPtr=_spIT.value();
        if(!list.contains(performerID))
        {
            list.append(performerID);
        }
    }
    return list;
}

void
SBIDSong::updateSoundexFields()
{
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
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
    qDebug() << SB_DEBUG_INFO << q;

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
    qDebug() << SB_DEBUG_INFO << ID() << "Removing id " << spPtr->songPerformanceID();

    if(_songPerformances.contains(spPtr->songPerformerID()))
    {
        qDebug() << SB_DEBUG_INFO << ID() << "Removing performance for performerID=" << spPtr->songPerformerID();
        _songPerformances.remove(spPtr->songPerformerID());
    }
    spPtr->setDeletedFlag();
}

///	Pointers
SBIDSongPerformancePtr
SBIDSong::originalSongPerformancePtr() const
{
    CacheManager* cm=Context::instance()->cacheManager();
    CacheSongPerformanceMgr* spMgr=cm->songPerformanceMgr();
    return spMgr->retrieve(
                SBIDSongPerformance::createKey(_originalSongPerformanceID),
                Cache::open_flag_parentonly);
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
SBIDSong::refreshDependents(bool showProgressDialogFlag,bool forcedFlag)
{
    if(showProgressDialogFlag)
    {
        ProgressDialog::instance()->show("Retrieving Data","SBIDSong::refreshDependents",4);
    }

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
            "s.title AS \"song title\", "
            "CAST(%2 AS VARCHAR)||':'||CAST(a.artist_id AS VARCHAR) AS SB_ITEM_KEY2, "
            "COALESCE(a.name,'n/a') AS \"original performer\", "
            "CAST(%3 AS VARCHAR)||':'||CAST(r.record_id AS VARCHAR) AS SB_ITEM_KEY3, "
            "COALESCE(r.title,'n/a') AS \"album title\" "
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
SBIDSong::retrieveSong(SBKey key,bool noDependentsFlag)
{
    CacheManager* cm=Context::instance()->cacheManager();
    CacheSongMgr* smgr=cm->songMgr();
    return smgr->retrieve(key,(noDependentsFlag==1?Cache::open_flag_parentonly:Cache::open_flag_default));
}

SBIDSongPtr
SBIDSong::retrieveSong(int songID,bool noDependentsFlag)
{
    return retrieveSong(createKey(songID),noDependentsFlag);
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
SBIDSong::operator=(const SBIDSong& t)
{
    _copy(t);
    return *this;
}

SBIDSongPtr
SBIDSong::createInDB(Common::sb_parameters& p)
{
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
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
                //"REPLACE(LOWER(s.title),' ','') = (LOWER('%1'),' ','') OR "
                "REGEXP_REPLACE(LOWER(title),'[^A-Za-z]','','g') = '%1' OR "
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
            //	"REPLACE(LOWER(s.title),' ','') != REPLACE(LOWER('%1'),' ','') "
            "REGEXP_REPLACE(LOWER(title),'[^A-Za-z]','','g') = '%1'  "
            "%4 "
        "ORDER BY "
            "1,3 "
    )
        .arg(Common::removeNonAlphanumericIncludingSpaces(p.songTitle.toLower()))
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
    song._originalSongPerformanceID   =r.value(i++).toInt();

    return std::make_shared<SBIDSong>(song);
}

void
SBIDSong::mergeFrom(SBIDSongPtr &fromPtr)
{
    CacheManager* cm=Context::instance()->cacheManager();

    //	SongPerformance
    CacheSongPerformanceMgr *spMgr=cm->songPerformanceMgr();
    _loadSongPerformances();	//	make sure list is loaded.
    QMapIterator<int,SBIDSongPerformancePtr> it(fromPtr->songPerformances());
    while(it.hasNext())
    {
        it.next();
        int performerID=it.key();
        SBIDSongPerformancePtr spPtr=it.value();

        if(_songPerformances.contains(performerID))
        {
            SBIDSongPerformancePtr toSpPtr=_songPerformances[performerID];
            spMgr->merge(spPtr,toSpPtr);
        }
        else
        {
            //	Assign to current
            spPtr->setSongID(this->songID());
            _songPerformances[performerID]=spPtr;

        }
    }

    //	Lyrics
    if(fromPtr->lyrics().length() && this->lyrics().length()==0)
    {
        _lyrics=fromPtr->lyrics();
    }

    //	set originalPerformanceID on the fromPtr
    fromPtr->setOriginalPerformanceID(-1);
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
    if(changedFlag() && db_change==Common::db_update)
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
            result=Common::result_exists;
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
                    result=Common::result_exists;
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

void
SBIDSong::clearChangedFlag()
{
    //	CWIP: find a more generic method -- maybe doing a full load from mgr
    SBIDBase::clearChangedFlag();
    foreach(SBIDSongPerformancePtr performancePtr,_songPerformances)
    {
        performancePtr->clearChangedFlag();
    }
    //	AlbumPerformances are owned by SBIDAlbum -- don't clear these
}

SBIDSongPerformancePtr
SBIDSong::addSongPerformance(SBIDSongPerformancePtr spPtr)
{
    if(_songPerformances.count()==0)
    {
        this->_loadSongPerformances();
    }

    if(!_songPerformances.contains(spPtr->songPerformanceID()))
    {
        _songPerformances[spPtr->songPerformerID()]=spPtr;
    }
    else
    {
        spPtr=_songPerformances[spPtr->songPerformerID()];
    }
    return spPtr;
}

///	Private methods
void
SBIDSong::_copy(const SBIDSong &c)
{
    SBIDBase::_copy(c);
    _songTitle                 =c._songTitle;
    _notes                     =c._notes;
    _lyrics                    =c._lyrics;
    _originalSongPerformanceID =c._originalSongPerformanceID;

    _playlistOnlinePerformances=c._playlistOnlinePerformances;
    _albumPerformances         =c._albumPerformances;
    _songPerformances          =c._songPerformances;
}

void
SBIDSong::_init()
{
    _songTitle=QString();
    _notes=QString();
    _lyrics=QString();
    _originalSongPerformanceID=-1;

    _albumPerformances.clear();
    _playlistOnlinePerformances.clear();
    _songPerformances.clear();
}

void
SBIDSong::_loadAlbumPerformances()
{
    SBSqlQueryModel* qm=SBIDAlbumPerformance::performancesBySong(songID());
    CacheManager* cm=Context::instance()->cacheManager();
    CacheAlbumPerformanceMgr* apmgr=cm->albumPerformanceMgr();

    //	Load performances including dependents, this will set its internal pointers
    _albumPerformances=apmgr->retrieveSet(qm,Cache::open_flag_default);

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

    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
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
    ProgressDialog::instance()->update("SBIDSong::_loadPlaylistOnlinePerformanceListFromDB",progressCurrentValue,progressMaxValue);

    query.first();
    query.previous();
    while(query.next())
    {
        SBIDPlaylistPtr plPtr=SBIDPlaylist::retrievePlaylist(query.value(0).toInt(),1);
        SBIDOnlinePerformancePtr opPtr=SBIDOnlinePerformance::retrieveOnlinePerformance(query.value(1).toInt(),1);

        SBIDSong::PlaylistOnlinePerformance r;
        r.plPtr=plPtr;
        r.opPtr=opPtr;

        playlistOnlinePerformanceList.append(r);
        ProgressDialog::instance()->update("SBIDSong::_loadPlaylistOnlinePerformanceListFromDB",progressCurrentValue++,progressMaxValue);
    }
    ProgressDialog::instance()->finishStep("SBIDSong::_loadPlaylistOnlinePerformanceListFromDB");
    return playlistOnlinePerformanceList;
}

QMap<int,SBIDSongPerformancePtr>
SBIDSong::_loadSongPerformancesFromDB() const
{
    SBSqlQueryModel* qm=SBIDSongPerformance::performancesBySong(songID());
    CacheManager* cm=Context::instance()->cacheManager();
    CacheSongPerformanceMgr* spmgr=cm->songPerformanceMgr();

    //	Load performances including dependents, this will set its internal pointers
    QMap<int,SBIDSongPerformancePtr> songPerformances=spmgr->retrieveMap(qm,Cache::open_flag_default);
    delete qm;
    return songPerformances;
}

