#include <QDebug>

#include "SBIDSong.h"

#include "CacheManager.h"
#include "Context.h"
#include "DataAccessLayer.h"
#include "Preloader.h"
#include "SBDialogSelectItem.h"
#include "SBTableModel.h"
#include "SqlQuery.h"

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
    return QString("Song - %1 / %2")
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
SBIDSong::charts(Common::retrieve_sbtablemodel) const
{
    SBTableModel* tm=new SBTableModel();
    QMap<SBIDChartPerformancePtr,SBIDChartPtr> chartToPerformances=this->charts(Common::retrieve_qmap());
    tm->populateChartsByItemType(SBKey::Song,chartToPerformances);
    return tm;
}

QMap<SBIDChartPerformancePtr, SBIDChartPtr>
SBIDSong::charts(Common::retrieve_qmap) const
{
    return Preloader::chartItems(*this);
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

    SqlQuery query(q,db);

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
SBIDSong::playlists(Common::retrieve_sbtablemodel)
{
    SBTableModel* tm=new SBTableModel();
    tm->populatePlaylists(this->playlists(Common::retrieve_qvector()));
    return tm;
}

QVector<SBIDSong::PlaylistOnlinePerformance>
SBIDSong::playlists(Common::retrieve_qvector)
{
    getSemaphore();
    if(!_playlistOnlinePerformances.count())
    {
        //	Playlists may not be loaded -- retrieve again
        this->_loadPlaylists();
    }
    releaseSemaphore();

    return _playlistOnlinePerformances;
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

    SqlQuery q1(db);
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
        SqlQuery q2(q,db);
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

SBKey
SBIDSong::songOriginalPerformerKey() const
{
    SBIDSongPerformancePtr spPtr=originalSongPerformancePtr();
    SB_RETURN_IF_NULL(spPtr,SBKey());

    SBIDPerformerPtr pPtr=spPtr->performerPtr();
    SB_RETURN_IF_NULL(pPtr,SBKey());
    return pPtr->key();
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
SBIDSong::retrieveAllSongs(const QChar& startsWith, qsizetype offset, qsizetype size)
{
    //	List songs with actual online performance only
    QString whereClause;
    QString limitClause;

    if(startsWith!=QChar('\x0'))
    {
        whereClause=QString("WHERE LOWER(LEFT(s.title,1))='%1'").arg(startsWith.toLower());
    }
    if(size>0)
    {
        limitClause=QString("LIMIT %1").arg(size);
    }
    const QString q=QString
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
        "%4 "
        "ORDER BY "
            "3,5,7 "
        "OFFSET "
            "%5 "
        "%6 "
    )
        .arg(SBKey::Song)
        .arg(SBKey::Performer)
        .arg(SBKey::Album)
        .arg(whereClause)
        .arg(offset)
        .arg(limitClause)
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

///	Other
int
SBIDSong::setAndSave(SBIDSongPtr orgSongPtr,const QString& editTitle, const QString& editPerformerName, int editYearOfRelease, const QString& editNotes, const QString& editLyrics, QString& updateText, bool modifyScreenStack, bool refreshData)
{
    //	This method has to be static as it may invalidate the orgSongPtr.

    //	Test cases:
    //	A1.	[simple rename] Bad - U2: change to Badaa. Should be simple renames.
    //	A2.	[simple rename w/ case] Badaa - U2 to BadaA. Should take into account case change.
    //	A3.	[switch original performer to completely new performer] BadaA - U2: change performer to U22.
    //	A4.	[merge song (within performer)] Badaa - U22 to Acrobat - U2. Note that album listing for Acrobat should include 'Bad' albums.
    //	A5.	[remove performances without album performances] Edit (no changes) and save Acrobat - U2 one more time to remove the U22 entry.

    //	B.	[switch original performer to non-original performer] Dancing Barefoot: change from Patti Smith to U2 and back

    //	C.	[switch original performer to different existing performer] "C" Moon Cry Like A Baby: Simple Minds -> U2

    //	D.	[Assign song to different song and its non-original performer]  Assign <whatever> to Sunday Bloody Sunday by The Royal Philharmonic Orchestra

    //	Refresh database
    //	F2.	[merge song (within performer)] Bad - U2 to Acrobat. Note that album listing for Acrobat should include 'Bad' albums.

    //	G.	[merge to different performer] "C" Moon Cry Like A Baby/Simple Minds -> "40"/U2

    //	H.	[merge song with existing song by renaming original performer] Get Lucky - Daft Poonk => Get Lucky - Daft Poonk & Squirrel W.
    //	I.	[simple edits]: 'Elusive Dreams ' -> 'Elusive Dreams': should just save the new title.

    //	Algorithm:
    //	1.	determine current original performance (orgOpPtr).
    //	2.	detect year, notes change -> metaDataChangeFlag
    //	3.	find same song performance based on edit data 'as is' (altSpPtr).
    //		If found:
    //			-	if songID is same, set original performance ID
    //			-	otherwise: merge -> save. [merge duplicate]
    //	4.	if performer has changed:
    //		4.1	lookup changed performer (userMatch)
    //			4.1.1	if not found:
    //				-	create new performer,
    //				-	create new song performance,
    //				-	set originalPerformanceID of song
    //				-	set other attributes, save song -- completely new performer means no other matches can be found.
    //			4.1.2	if found:
    //				-	find if song performance exist with edited song title & changed performer.
    //					-	If found:
    //						-	if song title has not changed, set new original performance
    //						-	otherwise: merge -> save.
    //					-	If not found, find song with matching edit song title.
    //						-	if found: merge
    //						-	if not found:
    //							-	create new song performance,
    //							-	set originalPerformanceID of song
    //							-	set other attributes, save song -- completely new performer means no other matches can be found.
    //		4.2	(else) if performer has not changed, title has changed:
    //			-	find performance with org performer and edit title (userMatch)
    //				-	if found: merge
    //				-	if not found:
    //					-	set other attributes, save new title and other attributes.
    //	5.	Consolidate song performances -- remove non-original performances not appearing on
    //		any album performance

    //	Data
    DataAccessLayer* dal=Context::instance()->dataAccessLayer();
    QString restorePoint=dal->createRestorePoint();

    //	Cache Mgrs
    CacheManager* cm=Context::instance()->cacheManager();
    CacheSongPerformanceMgr* spMgr=cm->songPerformanceMgr();
    CachePerformerMgr* peMgr=cm->performerMgr();
    CacheSongMgr* sMgr=cm->songMgr();

    //	1.	Pointers to original song.
    SBIDSongPerformancePtr orgSpPtr=orgSongPtr->originalSongPerformancePtr();
    qDebug() << SB_DEBUG_INFO << orgSongPtr->songTitle();


    SB_RETURN_IF_NULL(orgSongPtr,0);
    SB_RETURN_IF_NULL(orgSpPtr,0);

    //	Pointer to new song, only if found
    SBIDSongPtr newSongPtr;

    //	Flags
    bool metaDataChangedFlag=0;
    bool songTitleChangedFlag=0;
    bool mergedFlag=0;
    bool setMetaDataFlag=0;	//	if set, update meta data

    //	1.	Check if song title only has changed.
    if(editPerformerName==orgSongPtr->songOriginalPerformerName())
    {
        if(Common::comparable(editTitle)==Common::comparable(orgSongPtr->songTitle()))
        {
            songTitleChangedFlag=1;
        }
    }

    //	2.	Determine metaDataChangedFlag
    if(editYearOfRelease!=orgSpPtr->year() || editNotes!=orgSongPtr->notes() || editLyrics!=orgSongPtr->lyrics())
    {
        metaDataChangedFlag=1;
        setMetaDataFlag=1;
    }

    //	3.	Look up if song already exists.
    //	if(songTitleChangedFlag==0)
    {
        //	Keep the scope of altSpPtr local
        SBIDSongPerformancePtr altSpPtr=SBIDSongPerformance::retrieveSongPerformanceByPerformer(
                    editTitle,
                    editPerformerName,
                    orgSpPtr->songPerformanceID());

        if(altSpPtr)
        {
            if(altSpPtr->songID()==orgSongPtr->songID())
            {
                newSongPtr=orgSongPtr;
                newSongPtr->setOriginalPerformanceID(altSpPtr->songPerformanceID());
            }
            else
            {
                newSongPtr=altSpPtr->songPtr();
                SB_RETURN_IF_NULL(newSongPtr,0);
                mergedFlag=1;
                setMetaDataFlag=0;
            }
        }
    }

    //	4.	if performer has changed
    if(!mergedFlag)
    {
        if(editPerformerName!=orgSpPtr->songPerformerName())
        {
            SBIDPerformerPtr newPPtr;

            //		4.1	lookup changed performer (userMatch)
            Common::sb_parameters p;
            p.performerName=editPerformerName;
            p.performerID=-1;
            Common::result result=peMgr->userMatch(p,SBIDPerformerPtr(),newPPtr);

            if(result==Common::result_canceled)
            {
                qDebug() << SB_DEBUG_WARNING << "none selected -- exit from import";
                return 0;
            }
            else if(result==Common::result_missing)
            {
                newSongPtr=orgSongPtr;

                //	4.1.1	if not found:
                //	Create performer
                Common::sb_parameters performer;
                performer.performerName=p.performerName;
                newPPtr=peMgr->createInDB(performer);

                //	Create new song performance
                p.songID=newSongPtr->songID();
                p.performerID=newPPtr->performerID();
                p.year=orgSpPtr->year();
                SBIDSongPerformancePtr newSpPtr=spMgr->createInDB(p);
                newSongPtr->addSongPerformance(newSpPtr);

                //	set originalPerformanceID of song
                newSongPtr->setOriginalPerformanceID(newSpPtr->songPerformanceID());

                //	set other attributes, save song -- completely new performer means no other matches can be found.
                setMetaDataFlag=1;
            }
            else if(result==Common::result_exists_derived || result==Common::result_exists_user_selected)
            {
                //	4.1.2	if found:
                //		find if song performance exist with edited song title & changed performer.
                SBIDSongPerformancePtr altSpPtr=SBIDSongPerformance::retrieveSongPerformanceByPerformer(
                            editTitle,
                            newPPtr->performerName(),
                            orgSpPtr->songPerformanceID());

                if(altSpPtr)
                {
                    if(altSpPtr->songID()==orgSongPtr->songID())
                    {
                        newSongPtr=orgSongPtr;
                        newSongPtr->setOriginalPerformanceID(altSpPtr->songPerformanceID());
                    }
                    else
                    {
                        //		If found, merge -> save.
                        newSongPtr=altSpPtr->songPtr();
                        SB_RETURN_IF_NULL(newSongPtr,0);
                        mergedFlag=1;
                    }
                }
                else
                {
                    //		If not found, find song with matching edit song title.
                    Common::sb_parameters p;
                    p.songTitle=editTitle;
                    p.performerID=newPPtr->performerID();
                    p.performerName=newPPtr->performerName();
                    p.year=orgSpPtr->year();
                    p.notes=orgSpPtr->notes();

                    Common::result result=sMgr->userMatch(p,SBIDSongPtr(),newSongPtr);
                    if(result==Common::result_canceled)
                    {
                        qDebug() << SB_DEBUG_WARNING << "none selected -- exit from import";
                        return 0;
                    }
                    if(result==Common::result_exists_derived || result==Common::result_exists_user_selected)
                    {
                        //		if found: merge
                        SB_RETURN_IF_NULL(newSongPtr,0);
                        mergedFlag=1;
                    }
                    if(result==Common::result_missing)
                    {
                        //		if not found:
                        if(!newSongPtr)
                        {
                            newSongPtr=orgSongPtr;
                        }
                        //	Create new song performance
                        p.songID=orgSongPtr->songID();
                        p.performerID=newPPtr->performerID();
                        p.year=editYearOfRelease;
                        SBIDSongPerformancePtr newSpPtr=spMgr->createInDB(p);
                        newSongPtr->addSongPerformance(newSpPtr);

                        //	set originalPerformanceID of song
                        newSongPtr->setOriginalPerformanceID(newSpPtr->songPerformanceID());

                        //	set other attributes, save song -- completely new performer means no other matches can be found.
                        setMetaDataFlag=1;
                    }
                }
            }
        }
        else
        {
            //	4.2	(else) if performer has not changed, title has changed:
            Common::sb_parameters p;
            p.songTitle=editTitle;
            p.performerID=orgSpPtr->songPerformerID();
            p.performerName=orgSpPtr->songPerformerName();
            p.year=orgSpPtr->year();
            p.notes=orgSpPtr->notes();

            Common::result result=sMgr->userMatch(p,SBIDSongPtr(),newSongPtr);
            if(result==Common::result_canceled)
            {
                qDebug() << SB_DEBUG_WARNING << "none selected -- exit from import";
                return 0;
            }
            else if(result==Common::result_exists_derived || result==Common::result_exists_user_selected)
            {
                SB_RETURN_IF_NULL(newSongPtr,0);
                if(orgSongPtr->songID()!=newSongPtr->songID())
                {
                    mergedFlag=1;
                }
            }
            else if(result==Common::result_missing)
            {
                newSongPtr=orgSongPtr;
                newSongPtr->setSongTitle(editTitle);
                setMetaDataFlag=1;
                songTitleChangedFlag=0;
            }
        }
    }

    if(mergedFlag)
    {
        sMgr->merge(orgSongPtr,newSongPtr);
        setMetaDataFlag=0;
    }

    if(setMetaDataFlag && metaDataChangedFlag)
    {
        SB_RETURN_IF_NULL(newSongPtr,0);
        newSongPtr->setNotes(editNotes);
        newSongPtr->setLyrics(editLyrics);
        SBIDSongPerformancePtr spPtr=newSongPtr->originalSongPerformancePtr();
        if(spPtr)
        {
            spPtr->setYear(editYearOfRelease);
        }
    }

    //	5.	Consolidate song performances with what is on any album.
    //		If a non-original performer does not exist on an album, remove this (there is no other way to remove this).
    if(!mergedFlag)
    {
        QMapIterator<int,SBIDSongPerformancePtr> it(newSongPtr->songPerformances());
        while(it.hasNext())
        {
            it.next();
            SBIDSongPerformancePtr spPtr=it.value();
            int songPerformanceID=spPtr->songPerformanceID();	//	CWIP: new data collection to keep track of this by songPerformanceID

            QVectorIterator<SBIDAlbumPerformancePtr> apIT(newSongPtr->allPerformances());
            bool foundFlag=0;
            while(apIT.hasNext() && !foundFlag)
            {
                SBIDAlbumPerformancePtr apPtr=apIT.next();
                if(apPtr->songPerformanceID()==songPerformanceID)
                {
                    foundFlag=1;
                }
            }

            if(!foundFlag && editPerformerName!=spPtr->songPerformerName())
            {
                newSongPtr->removeSongPerformance(spPtr);
            }
        }
    }

    if(songTitleChangedFlag==1)
    {
        if(!newSongPtr)
        {
            newSongPtr=orgSongPtr;
        }
        if(newSongPtr->key()==orgSongPtr->key() && songTitleChangedFlag==1)
        {
            orgSongPtr->setSongTitle(editTitle);
        }
    }

    cm->debugShowChanges("before save");
    //const bool successFlag=cm->saveChanges("Saving Song",refreshData);
    const bool successFlag=cm->saveChanges("Saving Song",0);	//	TAKE ABOVE VERSION

    if(successFlag)
    {
        ProgressDialog::instance()->startDialog(__SB_PRETTY_FUNCTION__,"Refreshing Data",1);
        ProgressDialog::instance()->update(__SB_PRETTY_FUNCTION__,"step:refresh",1,5);

        //	Update screenstack, display notice, etc.
        updateText=QString("Saved song %1%2%3.")
            .arg(QChar(96))      //	1
            .arg(newSongPtr->songTitle())   //	2
            .arg(QChar(180));    //	3

        //	Update screenstack
        ProgressDialog::instance()->update(__SB_PRETTY_FUNCTION__,"step:refresh",2,5);

        ProgressDialog::instance()->update(__SB_PRETTY_FUNCTION__,"step:refresh",3,5);
        if(mergedFlag && modifyScreenStack)
        {
            if(refreshData)
            {
                //	Refresh models -- since song got removed.

                ScreenStack* st=Context::instance()->screenStack();

                qDebug() << SB_DEBUG_INFO << "refresh";

                //	newSongPtr->refreshDependents(1);		UNCOMMENT BEFORE DEPLOY
                ScreenItem from(orgSongPtr->key());
                ScreenItem to(newSongPtr->key());
                st->replace(from,to);
            }
        }

        ProgressDialog::instance()->update(__SB_PRETTY_FUNCTION__,"step:refresh",4,5);
        if(mergedFlag || songTitleChangedFlag)
        {
            ProgressDialog::instance()->update(__SB_PRETTY_FUNCTION__,"step:refresh",4,5);
            ProgressDialog::instance()->finishDialog(__SB_PRETTY_FUNCTION__);
            return 1;
        }


        ProgressDialog::instance()->finishStep(__SB_PRETTY_FUNCTION__,"step:refresh");
        ProgressDialog::instance()->finishDialog(__SB_PRETTY_FUNCTION__);
    }
    else
    {
        dal->restore(restorePoint);
    }
    ProgressDialog::instance()->finishDialog(__SB_PRETTY_FUNCTION__);

    return 0;

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
        SqlQuery qName(q,db);

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
    SqlQuery insert(q,db);
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
        .arg(p.songTitle)
    ;
    return new SBSqlQueryModel(q,-1,1);
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
SBIDSong::userMatch(const Common::sb_parameters& p, SBIDSongPtr exclude, SBIDSongPtr& found)
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
        else if(p.suppressDialogsFlag==0)
        {
            if(matches[1].count()>1)
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
            else
            {
                result=Common::result_missing;
            }
        }
        else
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

    qm->deleteLater();
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

    SqlQuery query(db);
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

