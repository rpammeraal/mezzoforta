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

SBIDBase::sb_type
SBIDAlbum::itemType() const
{
    return SBIDBase::sb_type_album;
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

void
SBIDAlbum::sendToPlayQueue(bool enqueueFlag)
{
    ProgressDialog::instance()->show("Loading songs","SBIDAlbum::sendToPlayQueue",3);
    QMap<int,SBIDOnlinePerformancePtr> list;

    if(_albumPerformances.count()==0)
    {
        const_cast<SBIDAlbum *>(this)->refreshDependents(0,0);
    }

    int index=0;
    int progressCurrentValue=0;
    int progressMaxValue=_albumPerformances.count();
    ProgressDialog::instance()->update("SBIDAlbum::sendToPlayQueue",0,progressMaxValue);
    QMapIterator<int,SBIDAlbumPerformancePtr> pIT(_albumPerformances);
    while(pIT.hasNext())
    {
        pIT.next();
        const SBIDAlbumPerformancePtr apPtr=pIT.value();
        const SBIDOnlinePerformancePtr opPtr=apPtr->preferredOnlinePerformancePtr();
        if(opPtr && opPtr->path().length()>0)
        {
            list[index++]=opPtr;
        }
        ProgressDialog::instance()->update("SBIDAlbum::sendToPlayQueue",progressCurrentValue++,progressMaxValue);
    }
    ProgressDialog::instance()->finishStep("SBIDAlbum::sendToPlayQueue");

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

///	Album specific methods
//	either add to a list of songs in SBIDAlbum (preferred)
//	or as saveSongToAlbum().
QStringList
SBIDAlbum::addSongToAlbum(const SBIDSong &song) const
{
    Q_UNUSED(song);
    QStringList SQL;

//    //	Insert performance if not exists
//    SQL.append
//    (
//        QString
//        (
//            "INSERT INTO ___SB_SCHEMA_NAME___performance "
//            "( "
//                "song_id, "
//                "artist_id, "
//                "year, "
//                "notes "
//            ") "
//            "SELECT "
//                "song_id, "
//                "%1, "	//	artist_id
//                "%3, "
//                "'' "
//            "FROM "
//                "___SB_SCHEMA_NAME___song s "
//            "WHERE "
//                "song_id=%2 AND "
//                "NOT EXISTS "
//                "( "
//                    "SELECT "
//                        "1 "
//                    "FROM "
//                        "___SB_SCHEMA_NAME___performance "
//                    "WHERE "
//                        "song_id=%2 AND "
//                        "artist_id=%1 "
//                ") "
//        )
//            .arg(song.songPerformerID())
//            .arg(song.songID())
//            .arg(song.year())
//    );

//    //	Insert record performance
//    SQL.append
//    (
//        QString
//        (
//            "INSERT INTO ___SB_SCHEMA_NAME___record_performance "
//            "( "
//                "song_id, "
//                "artist_id, "
//                "record_id, "
//                "record_position, "
////                "op_song_id, "
////                "op_artist_id, "
////                "op_record_id, "
////                "op_record_position, "
//                "duration "
//            ") "

//            "VALUES "
//            "( "
//                "%1, "
//                "%2, "
//                "%3, "
//                "%4, "
////                "%1, "
////                "%2, "
////                "%3, "
////                "%4, "
//                "'00:00:00' "
//             ") "
//        )
//            .arg(song.songID())
//            .arg(song.songPerformerID())
//            .arg(this->albumID())
//            .arg(song.albumPosition())
//    );
    return SQL;
}

SBIDAlbumPerformancePtr
SBIDAlbum::addAlbumPerformance(int songID, int performerID, int albumPosition, int year, const QString& path, const SBDuration& duration, const QString& notes)
{
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    SBIDSongPerformanceMgr* spMgr=Context::instance()->getSongPerformanceMgr();
    SBIDAlbumPerformanceMgr* apMgr=Context::instance()->getAlbumPerformanceMgr();
    SBIDOnlinePerformanceMgr* opMgr=Context::instance()->getOnlinePerformanceMgr();
    SBIDSongPtr sPtr;
    SBIDSongPerformancePtr spPtr;
    SBIDAlbumPerformancePtr apPtr;
    Common::sb_parameters p;
    bool newSongPerformanceFlag=0;
    bool newAlbumPerformanceFlag=0;

    if(_albumPerformances.count()==0)
    {
        qDebug() << SB_DEBUG_INFO;
        _loadAlbumPerformances();
    }

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
        spPtr=SBIDSongPerformance::createNew(p);
        spMgr->add(spPtr);
        spMgr->commit(spPtr,dal);
        qDebug() << SB_DEBUG_INFO
                 << "new apID=" << spPtr->songPerformanceID()
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
        apPtr=SBIDAlbumPerformance::createNew(p);
        apMgr->add(apPtr);
        apMgr->commit(apPtr,dal);
        qDebug() << SB_DEBUG_INFO
                 << "new apID=" << apPtr->albumID()
        ;
    }
    if(!_albumPerformances.contains(apPtr->albumPerformanceID()))
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
        opPtr=SBIDOnlinePerformance::createNew(p);
        opMgr->add(opPtr);
        opMgr->commit(opPtr,dal);
        qDebug() << SB_DEBUG_INFO
                 << "new opID=" << opPtr->onlinePerformanceID()
        ;
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

QMap<int,SBIDAlbumPerformancePtr>
SBIDAlbum::albumPerformances() const
{
    if(_albumPerformances.count()==0)
    {
        const_cast<SBIDAlbum *>(this)->refreshDependents();
    }
    return _albumPerformances;
}

SBDuration
SBIDAlbum::duration() const
{
    SBDuration duration;
    if(_albumPerformances.count()==0)
    {
        const_cast<SBIDAlbum *>(this)->refreshDependents(0,0);
    }

    QMapIterator<int,SBIDAlbumPerformancePtr> apIT(_albumPerformances);
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

QStringList
SBIDAlbum::mergeAlbum(const SBIDBase& to) const
{
    Q_UNUSED(to);
    QStringList SQL;
    /*
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
    int maxPosition=0;

    //	Find max position in to
    QString q=QString
    (
        "SELECT DISTINCT "
            "MAX(record_position) "
        "FROM "
            "___SB_SCHEMA_NAME___record_performance rp "
        "WHERE "
            "rp.record_id=%1 "
    )
        .arg(to.albumID())
    ;
    dal->customize(q);
    qDebug() << SB_DEBUG_INFO << q;

    QSqlQuery query(q,db);

    if(query.next())
    {
        maxPosition=query.value(0).toInt();
        maxPosition++;
    }

    //	Update rock_performance.non_op fields
    SQL.append
    (
        QString
        (
            "UPDATE "
                "___SB_SCHEMA_NAME___record_performance "
            "SET "
                "op_record_id=%1, "
                "op_record_position=op_record_position+%2 "
            "WHERE "
                "op_record_id=%3 "
        )
            .arg(to.albumID())
            .arg(maxPosition)
            .arg(this->albumID())
    );

    //	Update rock_performance.op fields
    SQL.append
    (
        QString
        (
            "UPDATE "
                "___SB_SCHEMA_NAME___record_performance "
            "SET "
                "record_id=%1, "
                "record_position=record_position+%2 "
            "WHERE "
                "record_id=%3 "
        )
            .arg(to.albumID())
            .arg(maxPosition)
            .arg(this->albumID())
    );

    //	Update online_performance
    SQL.append
    (
        QString
        (
            "UPDATE "
                "___SB_SCHEMA_NAME___online_performance "
            "SET "
                "record_id=%1, "
                "record_position=record_position+%2 "
            "WHERE "
                "record_id=%3 "
        )
            .arg(to.albumID())
            .arg(maxPosition)
            .arg(this->albumID())
    );

    //	Update toplay
    SQL.append
    (
        QString
        (
            "UPDATE "
                "___SB_SCHEMA_NAME___toplay "
            "SET "
                "record_id=%1, "
                "record_position=record_position+%2 "
            "WHERE "
                "record_id=%3 "
        )
            .arg(to.albumID())
            .arg(maxPosition)
            .arg(this->albumID())
    );

    //	Update playlist_performance
    SQL.append
    (
        QString
        (
            "UPDATE "
                "___SB_SCHEMA_NAME___playlist_performance "
            "SET "
                "record_id=%1, "
                "record_position=record_position+%2 "
            "WHERE "
                "record_id=%3 "
        )
            .arg(to.albumID())
            .arg(maxPosition)
            .arg(this->albumID())
    );

    //	Add existing category items to new album
    SQL.append
    (
        QString
        (
            "INSERT INTO ___SB_SCHEMA_NAME___category_record "
            "( "
                "record_id, "
                "category_id "
            ") "
            "SELECT "
                "record_id, "
                "category_id "
            "FROM "
                "___SB_SCHEMA_NAME___category_record cr "
            "WHERE "
                "cr.record_id=%1 "
            "AND  "
                "NOT EXISTS "
                "( "
                    "SELECT NULL "
                    "FROM "
                        "___SB_SCHEMA_NAME___category_record ce "
                    "WHERE "
                        "ce.record_id=%2 AND "
                        "ce.record_id=cr.record_id AND "
                        "ce.category_id=cr.category_id "
                ") "
        )
            .arg(this->albumID())
            .arg(to.albumID())
    );

    //	Remove category items from album
    SQL.append
    (
        QString
        (
            "DELETE FROM "
                "___SB_SCHEMA_NAME___category_record "
            "WHERE "
                "record_id=%1 "
        )
            .arg(this->albumID())
    );
    */

    return SQL;
}

void
SBIDAlbum::postInstantiate(SBIDAlbumPtr &ptr)
{
    Q_UNUSED(ptr);
}

QStringList
SBIDAlbum::mergeSongInAlbum(int newPosition, const SBIDBase& song) const
{
    Q_UNUSED(newPosition);
    Q_UNUSED(song);
    QStringList SQL;

//    //	Move any performances from merged song to alt table
//    SQL.append
//    (
//        QString
//        (
//            "INSERT INTO ___SB_SCHEMA_NAME___online_performance_alt "
//            "SELECT "
//                "song_id, "
//                "artist_id, "
//                "record_id, "
//                "%1, "
//                "format_id, "
//                "path, "
//                "source_id, "
//                "last_play_date, "
//                "play_order, "
//                "insert_order "
//            "FROM "
//                "___SB_SCHEMA_NAME___online_performance op "
//            "WHERE"
//                "record_id=%2 AND "
//                "record_position=%3 "
//        )
//            .arg(newPosition)
//            .arg(this->albumID())
//            .arg(song.albumPosition())
//    );

//    //	Delete performance from op table
//    SQL.append
//    (
//        QString
//        (
//            "DELETE ___SB_SCHEMA_NAME___online_performance op "
//            "WHERE"
//                "record_id=%1 AND "
//                "record_position=%2 "
//        )
//            .arg(this->albumID())
//            .arg(song.albumPosition())
//    );

//    //	Update toplay
//    SQL.append
//    (
//        QString
//        (
//            "UPDATE "
//                "___SB_SCHEMA_NAME___toplay "
//            "SET "
//                "record_position=%1 "
//            "WHERE "
//                "record_id=%2 AND "
//                "record_position=%3 "
//        )
//            .arg(newPosition)
//            .arg(this->albumID())
//            .arg(song.albumPosition())
//    );

//    //	Update playlist_performance
//    SQL.append
//    (
//        QString
//        (
//            "UPDATE "
//                "___SB_SCHEMA_NAME___playlist_performance "
//            "SET "
//                "record_position=%1 "
//            "WHERE "
//                "record_id=%2 AND "
//                "record_position=%3 "
//        )
//            .arg(newPosition)
//            .arg(this->albumID())
//            .arg(song.albumPosition())
//    );

//    //	Delete song from record_performance
//    SQL.append
//    (
//        QString
//        (
//            "DELETE FROM "
//                "___SB_SCHEMA_NAME___record_performance "
//            "WHERE "
//                "record_id=%1 AND "
//                "record_position=%2 "
//        )
//            .arg(this->albumID())
//            .arg(song.albumPosition())
//    );

    return SQL;
}

int
SBIDAlbum::numPerformances() const
{
    if(_albumPerformances.count()==0)
    {
        const_cast<SBIDAlbum *>(this)->refreshDependents(0,0);
    }
    return _albumPerformances.count();
}

SBTableModel*
SBIDAlbum::performances() const
{
    if(_albumPerformances.count()==0)
    {
        const_cast<SBIDAlbum *>(this)->refreshDependents(0,0);
    }
    SBTableModel* tm=new SBTableModel();
    tm->populatePerformancesByAlbum(_albumPerformances);
    return tm;
}

///
/// \brief SBIDAlbum::processNewSongList
/// \param newSongList
///
/// Take newSongList as a curated list of songs and save this to the database.
void
SBIDAlbum::processNewSongList(QVector<MusicLibrary::MLentityPtr> &newSongList)
{
    Q_UNUSED(newSongList);
//    SBIDAlbumPerformanceMgr* apMgr=Context::instance()->getAlbumPerformanceMgr();
//    QVector<MusicLibrary::MLentityPtr> orgSongList(_albumPerformances.count()+1); //	<position:1>
//    QVector<int> changedSongs;         //	contains keys <unsorted>
//    QMap<int,int> mergedTo;            //	<songID:songPerformerID,mergedToIndex:1>
//    QMap<int,int> orgIDToPositionMap;  //	<MLentity::albumPerformanceID,position:1>
//    QMap<int,int> newIDToPositionMap;  //	<MLentity::albumPerformanceID,position:1>
//    QVector<int> allKeys;              //	Contains all MLEntityPtr::albumPerformanceID
//    int maxPosition=0;                 //	maxPosition:1

//    //	A.	Create data structures
//    //		1.	original list of songs as it is currently stored
//    qDebug() << SB_DEBUG_INFO;
//    QMapIterator<int,SBIDAlbumPerformancePtr> apIT(_loadAlbumPerformancesFromDB());
//    while(apIT.hasNext())
//    {
//        qDebug() << SB_DEBUG_INFO;
//        apIT.next();
//        SBIDAlbumPerformancePtr apPtr=apIT.value();

//        qDebug() << SB_DEBUG_INFO;
//        if(apPtr)
//        {
//            qDebug() << SB_DEBUG_INFO;
//            MusicLibrary::MLentity orgSong;
//            orgSong.songTitle=apPtr->songTitle();
//            orgSong.songID=apPtr->songID();
//            orgSong.songPerformerName=apPtr->songPerformerName();
//            orgSong.songPerformerID=apPtr->songPerformerID();
//            orgSong.albumTitle=this->albumTitle();
//            orgSong.albumID=this->albumID();
//            orgSong.albumPosition=apPtr->albumPosition();
//            orgSong.albumPerformerName=apPtr->albumPerformerName();
//            orgSong.albumPerformerID=apPtr->albumPerformerID();
//            orgSong.ID=apPtr->albumPerformanceID();
			
//            const int key=orgSong.ID;

//            orgSongList[orgSong.albumPosition]=std::make_shared<MusicLibrary::MLentity>(orgSong);
//            orgIDToPositionMap[key]=orgSong.albumPosition;
//            maxPosition=(orgSong.albumPosition>maxPosition)?orgSong.albumPosition:maxPosition;

//            if(!allKeys.contains(key))
//            {
//                allKeys.append(key);
//            }
//            qDebug() << SB_DEBUG_INFO;
//        }
//    }

//    //		2.	process list of songs
//    QVectorIterator<MusicLibrary::MLentityPtr> nslIT(newSongList);
//    qDebug() << SB_DEBUG_INFO;
//    while(nslIT.hasNext())
//    {
//        MusicLibrary::MLentityPtr currentPtr=nslIT.next();

//        if(currentPtr)
//        {
//            qDebug() << SB_DEBUG_INFO
//                     << currentPtr->songTitle
//                     << currentPtr->albumPosition
//                     << currentPtr->songID
//                     << currentPtr->songPerformerID
//                     << currentPtr->albumID
//                     << currentPtr->albumPosition
//            ;
//            const int key=currentPtr->ID;

//            if(currentPtr->mergedToAlbumPosition!=0)
//            {
//                qDebug() << SB_DEBUG_INFO << currentPtr->mergedToAlbumPosition;
//                MusicLibrary::MLentityPtr mergedToPtr=newSongList.at(currentPtr->mergedToAlbumPosition);
//                if(mergedToPtr)
//                {
//                    const int mergedToKey=mergedToPtr->ID;

//                    mergedTo[key]=mergedToKey;
//                }
//            }
//            newIDToPositionMap[key]=currentPtr->albumPosition;

//            //	Determine new, changed
//            if(orgIDToPositionMap.contains(key))
//            {
//                int position=orgIDToPositionMap[key];
//                MusicLibrary::MLentityPtr orgPtr=orgSongList[position];

//                SB_DEBUG_IF_NULL(orgPtr);
//                if(!(orgPtr->compareID(*currentPtr)))
//                {
//                    changedSongs.append(key);
//                }
//            }
//            maxPosition=(currentPtr->albumPosition>maxPosition)?currentPtr->albumPosition:maxPosition;

//            if(!allKeys.contains(key))
//            {
//                if(!currentPtr->removedFlag)
//                {
//                    //	Removed songs has their id's set to -1. Don't add removed songs.
//                    allKeys.append(key);
//                }
//            }
//        }
//    }

//    qDebug() << SB_DEBUG_INFO;
//    _showAlbumPerformances("before");
//    QMap<int,SBIDAlbumPerformancePtr> newAlbumPerformances;	//	1:based, index is record position
//    _removedAlbumPerformances.clear();	//	1:based, index is record position
//    _addedAlbumPerformances.clear();	//	1:based, index is record position
//    QVectorIterator<int> akIT(allKeys);
//    while(akIT.hasNext())
//    {
//        const int currentKey=akIT.next();
//        MusicLibrary::MLentityPtr orgSong;
//        MusicLibrary::MLentityPtr newSong;

//        int orgPosition=-1;
//        if(orgIDToPositionMap.contains(currentKey))
//        {
//            orgPosition=orgIDToPositionMap[currentKey];
//            orgSong=orgSongList.at(orgPosition);
//        }
//        int newPosition=-1;
//        if(newIDToPositionMap.contains(currentKey))
//        {
//            newPosition=newIDToPositionMap[currentKey];
//            newSong=newSongList.at(newPosition);
//        }

//        bool removedFlag=0;
//        if(!(newSong) || (newSong && newSong->removedFlag))
//        {
//            removedFlag=1;
//        }
//        bool addFlag=0;
//        if(!orgSong)
//        {
//            addFlag=1;
//        }

//        qDebug() << SB_DEBUG_INFO
//                 << currentKey
//                 << (removedFlag?"DEL":(addFlag?"ADD":"---"))
//                 << (orgSong?orgSong->songTitle:newSong->songTitle)
//                 << (mergedTo.contains(currentKey)?mergedTo[currentKey]:-1)
//                 << orgPosition
//                 << newPosition
//        ;

//        if(removedFlag)
//        {
//            _removedAlbumPerformances.append(_albumPerformances[orgPosition]);
//        }
//        else if(addFlag)
//        {
//            QSqlRecord r;
//            QSqlField f;

//            f=QSqlField("f1",QVariant::Int);    f.setValue(-1);                           r.append(f);
//            f=QSqlField("f2",QVariant::Int);    f.setValue(newSong->songPerformanceID);   r.append(f);
//            f=QSqlField("f3",QVariant::Int);    f.setValue(newSong->albumID);             r.append(f);
//            f=QSqlField("f4",QVariant::Int);    f.setValue(newSong->albumPosition);       r.append(f);
//            f=QSqlField("f5",QVariant::String); f.setValue(newSong->duration.toString()); r.append(f);
//            f=QSqlField("f6",QVariant::String); f.setValue(newSong->notes);               r.append(f);
//            f=QSqlField("f7",QVariant::Int);    f.setValue(-1);                           r.append(f);

//            SBIDAlbumPerformancePtr apPtr=SBIDAlbumPerformance::instantiate(r);
//            _addedAlbumPerformances.append(apPtr);
//            apMgr->add(apPtr);
//        }
//        else
//        {
//            qDebug() << SB_DEBUG_INFO << orgPosition << newPosition;
//            newAlbumPerformances[newPosition]=_albumPerformances[orgPosition];
//            newAlbumPerformances[newPosition]->setAlbumPosition(newPosition);
//        }
//    }
//    qDebug() << SB_DEBUG_INFO;
//    _showAlbumPerformances("after");
    return;
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
                "___SB_SCHEMA_NAME___toplay "
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
                "___SB_SCHEMA_NAME___toplay "
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

bool
SBIDAlbum::saveSongToAlbum(const SBIDSong &song) const
{
    Q_UNUSED(song);
//    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
//    QStringList SQL;

//    //	Determine extension
//    QFileInfo fi(song._path);
//    QString suffix=fi.suffix().trimmed().toLower();

//    //	Determine relative path
//    Properties* p=Context::instance()->getProperties();
//    QString pathRoot=p->musicLibraryDirectorySchema()+'/';

//    QString relPath=song._path;
//    relPath=relPath.replace(pathRoot,QString(),Qt::CaseInsensitive);

//    //	It is assumed that song and performance already exist
//    SQL.append
//    (
//        QString
//        (
//            "INSERT INTO ___SB_SCHEMA_NAME___record_performance "
//            "( "
//                "song_id, "
//                "artist_id, "
//                "record_id, "
//                "record_position, "
//                "op_song_id, "
//                "op_artist_id, "
//                "op_record_id, "
//                "op_record_position, "
//                "duration "
//            ") "
//            "VALUES "
//            "( "
//                "%1, "
//                "%2, "
//                "%3, "
//                "%4, "
//                "%1, "
//                "%2, "
//                "%3, "
//                "%4, "
//                "'%5' "
//             ") "
//        )
//            .arg(song.songID())
//            .arg(song.songPerformerID())
//            .arg(this->albumID())
//            .arg(song.albumPosition())
//            .arg(song.duration().toString(Duration::sb_full_hhmmss_format))
//    );

//    SQL.append
//    (
//        QString
//        (
//            "INSERT INTO ___SB_SCHEMA_NAME___online_performance "
//            "( "
//                "song_id, "
//                "artist_id, "
//                "record_id, "
//                "record_position, "
//                "format_id, "
//                "path, "
//                "source_id, "
//                "insert_order "
//            ") "
//            "SELECT "
//                "%1, "
//                "%2, "
//                "%3, "
//                "%4, "
//                "df.format_id, "
//                "E'%6', "
//                "0, "
//                "___SB_DB_ISNULL___(m.max_insert_order,0)+1 "
//            "FROM "
//                "digital_format df, "
//                "( "
//                    "SELECT MAX(insert_order) AS max_insert_order "
//                    "FROM ___SB_SCHEMA_NAME___online_performance "
//                ") m "
//            "WHERE "
//                "df.extension=E'%5' "

//        )
//            .arg(song.songID())
//            .arg(song.songPerformerID())
//            .arg(this->albumID())
//            .arg(song.albumPosition())
//            .arg(suffix)
//            .arg(Common::escapeSingleQuotes(relPath))
//    );
//    return dal->executeBatch(SQL);
    return 0;
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


QStringList
SBIDAlbum::updateSongOnAlbumWithNewOriginal(const SBIDSong &song)
{
    Q_UNUSED(song);
    QStringList SQL;

//    SQL.append
//    (
//        QString
//        (
//            "UPDATE "
//                "___SB_SCHEMA_NAME___online_performance "
//            "SET "
//                "song_id=%1, "
//                "artist_id=%2 "
//            "WHERE "
//                "record_id=%3 AND "
//                "record_position=%4 "
//        )
//            .arg(song.songID())
//            .arg(song.songPerformerID())
//            .arg(this->albumID())
//            .arg(song.albumPosition())
//    );

//    SQL.append
//    (
//        QString
//        (
//            "UPDATE "
//                "___SB_SCHEMA_NAME___record_performance "
//            "SET "
//                "song_id=%1, "
//                "artist_id=%2 "
//            "WHERE "
//                "record_id=%3 AND "
//                "record_position=%4 "
//        )
//            .arg(song.songID())
//            .arg(song.songPerformerID())
//            .arg(this->albumID())
//            .arg(song.albumPosition())
//    );

//    SQL.append
//    (
//        QString
//        (
//            ";WITH a AS "
//            "( "
//                "SELECT "
//                    "song_id, "
//                    "artist_id "
//                "FROM "
//                    "online_performance "
//                "WHERE  "
//                    "record_id=%1 AND  "
//                    "record_position=%2  "
//            ") "
//            "UPDATE record_performance  "
//            "SET  "
//                "op_song_id=(SELECT song_id FROM a), "
//                "op_artist_id=(SELECT artist_id FROM a) "
//            "WHERE  "
//                "record_id=%1 AND  "
//                "record_position=%2  "
//        )
//            .arg(this->albumID())
//            .arg(song.albumPosition())
//    );

    return SQL;
}

QStringList
SBIDAlbum::updateSongOnAlbum(const SBIDSong &song)
{
    Q_UNUSED(song);
    QStringList SQL;
    /*

    qDebug() << SB_DEBUG_INFO
             << song.songID()
             << song.songPerformerID()
             << song.albumID()
             << song.albumPosition()
             << song.notes()
    ;
    //	Update notes
    SQL.append
    (
        QString
        (
            "UPDATE "
                "___SB_SCHEMA_NAME___record_performance "
            "SET "
                "notes=E'%1' "
            "WHERE "
                "record_id=%2 AND "
                "record_position=%3 "
        )
            .arg(Common::escapeSingleQuotes(song.notes()))
            .arg(this->albumID())
            .arg(song.albumPosition())
    );

    //	Update performer
    SQL.append
    (
        QString
        (
            "UPDATE "
                "___SB_SCHEMA_NAME___online_performance "
            "SET "
                "artist_id=%1 "
            "WHERE "
                "record_id=%2 AND "
                "record_position=%3 "
        )
            .arg(song.songPerformerID())
            .arg(this->albumID())
            .arg(song.albumPosition())
    );

    SQL.append
    (
        QString
        (
            "UPDATE "
                "___SB_SCHEMA_NAME___record_performance "
            "SET "
                "op_artist_id=%1 "
            "WHERE "
                "op_record_id=%2 AND "
                "op_record_position=%3 "
        )
            .arg(song.songPerformerID())
            .arg(this->albumID())
            .arg(song.albumPosition())
    );

    SQL.append
    (
        QString
        (
            "UPDATE "
                "___SB_SCHEMA_NAME___record_performance "
            "SET "
                "artist_id=%1 "
            "WHERE "
                "record_id=%2 AND "
                "record_position=%3 "
        )
            .arg(song.songPerformerID())
            .arg(this->albumID())
            .arg(song.albumPosition())
    );
    */
    return SQL;
}

///	Pointers
SBIDPerformerPtr
SBIDAlbum::performerPtr() const
{
    SBIDPerformerMgr* pMgr=Context::instance()->getPerformerMgr();
    return pMgr->retrieve(
                SBIDPerformer::createKey(_albumPerformerID),
                SBIDManagerTemplate<SBIDPerformer,SBIDBase>::open_flag_parentonly);
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
QString
SBIDAlbum::createKey(int albumID,int unused)
{
    Q_UNUSED(unused);
    return albumID>=0?QString("%1:%2")
        .arg(SBIDBase::sb_type_album)
        .arg(albumID):QString("x:x")	//	Return invalid key if albumID<0
    ;
}

QString
SBIDAlbum::key() const
{
    return createKey(this->albumID());
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
SBIDAlbum::retrieveAlbum(int albumID,bool noDependentsFlag)
{
    SBIDAlbumMgr* amgr=Context::instance()->getAlbumMgr();
    SBIDAlbumPtr albumPtr;
    if(albumID>=0)
    {
        albumPtr=amgr->retrieve(createKey(albumID),(noDependentsFlag==1?SBIDManagerTemplate<SBIDAlbum,SBIDBase>::open_flag_parentonly:SBIDManagerTemplate<SBIDAlbum,SBIDBase>::open_flag_default));
    }
    return albumPtr;
}

SBIDAlbumPtr
SBIDAlbum::retrieveUnknownAlbum()
{
    Properties* properties=Context::instance()->getProperties();
    SBIDAlbumMgr* amgr=Context::instance()->getAlbumMgr();
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
            "r.notes, "
            "r.year "
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

    //	Find performer 'VARIOUS ARTISTS'
    SBIDPerformerPtr peptr=SBIDPerformer::retrieveVariousPerformers();
    p.albumPerformerID=peptr->performerID();

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
        .arg(p.albumPerformerID)
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
    album._albumPerformerID=p.albumPerformerID;
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
             << tobeFound.albumTitle
             << tobeFound.performerID
    ;

    //	MatchRank:
    //	0	-	exact match with specified artist (0 or 1 in data set).
    //	2	-	exact match with any other artist (0 or more in data set).
    QString q=QString
    (
        "SELECT "
            "CASE WHEN a.artist_id=%2 THEN 0 ELSE 2 END AS matchRank, "
            "p.record_id, "
            "p.title, "
            "a.artist_id, "
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
            "p.title, "
            "a.artist_id, "
            "p.year, "
            "p.genre, "
            "p.notes "
        "FROM "
            "___SB_SCHEMA_NAME___record p "
                "JOIN ___SB_SCHEMA_NAME___artist a ON "
                    "p.artist_id=a.artist_id, "
            "article t "
        "WHERE "
            //"LOWER(p.title)!=LOWER(regexp_replace(p.title,'^'||aa.word,'','i')) AND "
            //"LOWER(regexp_replace(p.title,'^'||aa.word,'','i'))=LOWER('%4') AND "
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
    ;
    return new SBSqlQueryModel(q);
}

SBIDAlbumPtr
SBIDAlbum::instantiate(const QSqlRecord &r)
{
    SBIDAlbum album;
    album._albumID         =r.value(0).toInt();
    album._albumPerformerID=r.value(1).toInt();
    album._albumTitle      =r.value(2).toString();
    album._genre           =r.value(3).toString();
    album._notes           =r.value(4).toString();
    album._year            =r.value(5).toInt();

    return std::make_shared<SBIDAlbum>(album);
}

void
SBIDAlbum::mergeTo(SBIDAlbumPtr &to)
{
    Q_UNUSED(to);
}

void
SBIDAlbum::openKey(const QString &key, int &albumID)
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
            "r.notes, "
            "r.year "
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
SBIDAlbum::updateSQL() const
{
    QStringList SQL;

    qDebug() << SB_DEBUG_INFO
             << deletedFlag()
             << newFlag()
             << mergedFlag()
             << changedFlag()
    ;

    if(deletedFlag() && !newFlag())
    {

    }
    else if(newFlag() && !deletedFlag())
    {

    }
    else if(!mergedFlag() && !deletedFlag() && changedFlag())
    {
        SQL.append(QString(
            "UPDATE ___SB_SCHEMA_NAME___record "
            "SET "
                "artist_id=%2, "
                "title='%3', "
                "year=%4, "
                "notes='%5', "
                "genre='%6' "
            "WHERE "
                "record_id=%1 "
        )
            .arg(this->albumID())
            .arg(this->albumPerformerID())
            .arg(Common::escapeSingleQuotes(this->albumTitle()))
            .arg(this->year())
            .arg(Common::escapeSingleQuotes(this->notes()))
            .arg(Common::escapeSingleQuotes(this->genre()))
        );

        SQL.append(_updateSQLAlbumPerformances());
    }

    if(SQL.count()==0)
    {
        SBMessageBox::standardWarningBox("__FILE__ __LINE__ No SQL generated.");
    }

    return SQL;
}

Common::result
SBIDAlbum::userMatch(const Common::sb_parameters &p, SBIDAlbumPtr exclude, SBIDAlbumPtr& found)
{
    SBIDAlbumMgr* amgr=Context::instance()->getAlbumMgr();
    Common::result result=Common::result_canceled;
    QMap<int,QList<SBIDAlbumPtr>> matches;

    if(amgr->find(p,exclude,matches))
    {
        if(matches[0].count()==1)
        {
            //	Dataset indicates an exact match if the 2nd record identifies an exact match.
            found=matches[0][0];
            result=Common::result_exists;
        }
        else
        {
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
                    found=std::dynamic_pointer_cast<SBIDAlbum>(selected);
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
    _albumPerformanceID2AlbumPositionMap=t._albumPerformanceID2AlbumPositionMap;
    _addedAlbumPerformances             =t._addedAlbumPerformances;
    _removedAlbumPerformances           =t._removedAlbumPerformances;
}

void
SBIDAlbum::_init()
{
    _sb_item_type=SBIDBase::sb_type_album;

    _albumID=-1;
    _albumPerformerID=-1;
    _albumTitle=QString();
    _genre=QString();
    _notes=QString();
    _year=-1;

    _albumPerformances.clear();
    _albumPerformanceID2AlbumPositionMap.clear();
    _addedAlbumPerformances.clear();
    _removedAlbumPerformances.clear();
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

//    QVectorIterator<SBIDOnlinePerformancePtr> opIT(_albumPerformances);
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
    QMapIterator<int,SBIDAlbumPerformancePtr> apIT(_albumPerformances);
    qDebug() << SB_DEBUG_INFO << title;
    while(apIT.hasNext())
    {
        apIT.next();
        const SBIDAlbumPerformancePtr apPtr=apIT.value();

        qDebug() << SB_DEBUG_INFO << apPtr->operator QString();
        ;
    }
}
