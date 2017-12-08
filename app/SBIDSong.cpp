#include <QDebug>

#include "SBIDSong.h"

#include "Common.h"
#include "Context.h"
#include "DataAccessLayer.h"
#include "Preloader.h"
#include "SBDialogSelectItem.h"
#include "SBIDAlbumPerformance.h"
#include "SBIDOnlinePerformance.h"
#include "SBMessageBox.h"
#include "SBSqlQueryModel.h"
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

int
SBIDSong::itemID() const
{
    return this->_songID;
}

SBIDBase::sb_type
SBIDSong::itemType() const
{
    return SBIDBase::sb_type_song;
}

//bool
//SBIDSong::save()
//{
    //DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    //QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
    //QStringList SQL;
//
//    if(this->_songID==-1)
//    {
//        QString newSoundex=Common::soundex(this->_songTitle);
//        QString q;

//        //	Find out new songID
//        q=QString
//        (
//            "SELECT "
//                "%1(MAX(song_id)+1,0) AS MaxSongID "
//            "FROM "
//                "___SB_SCHEMA_NAME___song "
//        )
//            .arg(dal->getIsNull())
//        ;

//        dal->customize(q);
//        qDebug() << SB_DEBUG_INFO << q;

//        QSqlQuery select(q,db);
//        select.next();

//        this->_songID=select.value(0).toInt();

//        //	Last minute cleanup of title
//        this->_songTitle=this->_songTitle.simplified();

//        //	Insert new song
//        SQL.append
//        (
//            QString
//            (
//                "INSERT INTO ___SB_SCHEMA_NAME___song "
//                "( "
//                    "song_id, "
//                    "title, "
//                    "soundex "
//                ") "
//                "SELECT "
//                    "%1, "
//                    "'%2', "
//                    "'%3' "
//            )
//                .arg(this->_songID)
//                .arg(Common::escapeSingleQuotes(this->_songTitle))
//                .arg(newSoundex)
//        );

//        //	Upsert new performance
//        SQL.append
//        (
//            QString
//            (
//                "INSERT INTO ___SB_SCHEMA_NAME___performance "
//                "( "
//                    "song_id, "
//                    "artist_id, "
//                    "year, "
//                    "notes "
//                ") "
//                "SELECT "
//                    "d.song_id, "
//                    "d.artist_id, "
//                    "NULLIF(d.year,0), "
//                    "d.notes "
//                "FROM "
//                    "( "
//                        "SELECT "
//                            "%1 as song_id, "
//                            "%2 as artist_id, "
//                            "%3 as year, "
//                            "CAST(E'%4' AS VARCHAR) as notes "
//                    ") d "
//                        "LEFT JOIN ___SB_SCHEMA_NAME___performance p ON "
//                            "d.song_id=p.song_id "
//                "WHERE "
//                    "NOT EXISTS "
//                    "( "
//                        "SELECT NULL "
//                        "FROM ___SB_SCHEMA_NAME___performance p "
//                        "WHERE d.song_id=p.song_id AND d.artist_id=p.artist_id "
//                    ") "
//                "GROUP BY "
//                    "d.song_id, "
//                    "d.artist_id, "
//                    "d.year, "
//                    "d.notes, "
//            )
//                .arg(this->_songID)
//                .arg(this->_sb_song_performer_id)
//                .arg(this->_year)
//                .arg(Common::escapeSingleQuotes(this->_notes))
//        );
//    }
//    else
//    {
//        //	Update existing
//    }
    //return dal->executeBatch(SQL);
//}

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
    SBIDSongPerformanceMgr* spMgr=Context::instance()->getSongPerformanceMgr();
    SBIDSongPerformancePtr songPerformancePtr;
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

        songPerformancePtr=spMgr->createInDB(p);
        _songPerformances[performerID]=songPerformancePtr;
    }
    else
    {
        songPerformancePtr=_songPerformances[performerID];
    }
    return songPerformancePtr;
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
    tm->populateChartsByItemType(SBIDBase::sb_type_song,chartToPerformances);
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
        q+=QString("(SELECT COUNT(*) FROM ___SB_SCHEMA_NAME___%1_performance WHERE song_id=%2) ").arg(t).arg(this->_songID);
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
            SQL.append(QString("DELETE FROM ___SB_SCHEMA_NAME___%1 WHERE song_id=%2").arg(t).arg(_songID));
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


//	TO ::find
///
/// \brief DataEntitySong::matchSong
/// \param newSongID
/// \param newSongTitle
/// \return
///
/// Match song title to any other existing song title regardless of
/// performer.
///
/// Eg.:
/// -	Syndayz Bloody Syndayz/Whoever -> Sunday Bloody Sunday/U2, or
/// 	                                  Syndayz Bloody Syndayz/Whoever
///
//SBSqlQueryModel*
//SBIDSong::matchSong() const
//{
//    QString newSoundex=Common::soundex(this->songTitle());

//    //	MatchRank:
//    //	0	-	edited value (always one in data set).
//    //	1	-	exact match with specified artist (0 or 1 in data set).
//    //	2	-	exact match with any other artist (0 or more in data set).
//    //	3	-	soundex match with any other artist (0 or more in data set).
//    QString q=QString
//    (
//        "SELECT "
//            "0 AS matchRank, "
//            "-1 AS song_id, "
//            "'%1' AS title, "
//            "%3 AS artist_id, "
//            "'%2' AS artistName "
//        "UNION "
//        "SELECT "
//            "CASE WHEN a.artist_id=%3 THEN 1 ELSE 2 END AS matchRank, "
//            "s.song_id, "
//            "s.title, "
//            "a.artist_id, "
//            "a.name "
//        "FROM "
//            "___SB_SCHEMA_NAME___performance p "
//                "JOIN ___SB_SCHEMA_NAME___song s ON "
//                    "p.song_id=s.song_id "
//                "JOIN ___SB_SCHEMA_NAME___artist a ON "
//                    "p.artist_id=a.artist_id "
//        "WHERE "
//            "REPLACE(LOWER(s.title),' ','') = REPLACE(LOWER('%1'),' ','') "
//        "UNION "
//        "SELECT "
//            "3 AS matchRank, "
//            "s.song_id, "
//            "s.title, "
//            "a.artist_id, "
//            "a.name "
//        "FROM "
//            "___SB_SCHEMA_NAME___performance p "
//                "JOIN ___SB_SCHEMA_NAME___song s ON "
//                    "p.song_id=s.song_id "
//                "JOIN ___SB_SCHEMA_NAME___artist a ON "
//                    "p.artist_id=a.artist_id "
//        "WHERE "
//            "p. sole_id=0 AND "
//            "( "
//                "SUBSTR(s.soundex,1,LENGTH('%4'))='%4' OR "
//                "SUBSTR('%4',1,LENGTH(s.soundex))=s.soundex "
//            ") "
//        "ORDER BY "
//            "1,3 "

//    )
//        .arg(Common::escapeSingleQuotes(this->songTitle()))
//        .arg(Common::escapeSingleQuotes(this->songPerformerName()))
//        .arg(this->songPerformerID())
//        .arg(newSoundex)
//    ;
//    return new SBSqlQueryModel(q);
//}

//	NOT MOVED TO ::find
///
/// \brief SBIDSong::matchSongWithinPerformer
/// \param newSongID
/// \param newSongTitle
/// \return
///
/// Match song title to a possible existing song title within given artist.
/// The use case is when a song is changed -- always match the song title
/// up within the scope of the given artist.
///
/// Eg.: input:
/// -	Syndayz Bloody Syndayz/U2 -> Sunday Bloody Sunday/U2
/// -	Syndayz Bloody Syndayz/Whoever -> Syndayz Bloody Syndayz/Whoever (Unchanged).
///
//SBSqlQueryModel*
//SBIDSong::matchSongWithinPerformer(const QString& newSongTitle) const
//{
//    //	Matches a song by artist
//    QString newSoundex=Common::soundex(newSongTitle);

//    //	MatchRank:
//    //	0	-	edited value (always one in data set).
//    //	1	-	exact match with new artist artist (0 or 1 in data set).
//    //	2	-	soundex match with new artist (0 or more in data set).
//    QString q=QString
//    (
//        "SELECT "
//            "0 AS matchRank, "
//            "-1 AS song_id, "
//            "'%1' AS title, "
//            "%3 AS artist_id, "
//            "'%2' AS artistName "
//        "UNION "
//        "SELECT "
//            "1 AS matchRank, "
//            "s.song_id, "
//            "s.title, "
//            "p.artist_id, "
//            "a.name "
//        "FROM "
//            "___SB_SCHEMA_NAME___song s "
//                "JOIN ___SB_SCHEMA_NAME___performance p ON "
//                    "s.song_id=p.song_id "
//                "JOIN ___SB_SCHEMA_NAME___artist a ON "
//                    "p.artist_id=a.artist_id AND "
//                    "a.artist_id IN (%3) "
//        "WHERE "
//            "s.song_id!=%4 AND "
//            "REPLACE(LOWER(s.title),' ','') = REPLACE(LOWER('%1'),' ','') "
//        "UNION "
//        "SELECT "
//            "2 AS matchRank, "
//            "s.song_id, "
//            "s.title, "
//            "p.artist_id, "
//            "a.name "
//        "FROM "
//            "___SB_SCHEMA_NAME___song s "
//                "JOIN ___SB_SCHEMA_NAME___performance p ON "
//                    "s.song_id=p.song_id "
//                "JOIN ___SB_SCHEMA_NAME___artist a ON "
//                    "p.artist_id=a.artist_id AND "
//                    "a.artist_id=%3 "
//        "WHERE "
//            "s.title!='%1' AND "
//            "( "
//                "SUBSTR(s.soundex,1,LENGTH('%5'))='%5' OR "
//                "SUBSTR('%5',1,LENGTH(s.soundex))=s.soundex "
//            ") "
//        "ORDER BY "
//            "1,3 "
//    )
//        .arg(Common::escapeSingleQuotes(newSongTitle))
//        .arg(Common::escapeSingleQuotes(this->songPerformerName()))
//        .arg(this->songPerformerID())
//        .arg(this->songID())
//        .arg(newSoundex)
//    ;
//    return new SBSqlQueryModel(q);
//}


//void
//SBIDSong::setPlaylistPosition(int playlistPosition)
//{
//    _sb_playlist_position=playlistPosition;
//    setChangedFlag();
//}

//int
//SBIDSong::songPerformerID() const
//{
//    return _sb_song_performer_id;
//}

//bool
//SBIDSong::updateExistingSong(const SBIDBase &oldSongID, SBIDSong &newSongID, const QStringList& extraSQL,bool commitFlag)
//{
//    Q_UNUSED(oldSongID);
//    Q_UNUSED(newSongID);
//    Q_UNUSED(extraSQL);
//    Q_UNUSED(commitFlag);

//    //DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
//    //QStringList allQueries;
//    //QString q;
//    bool resultFlag=1;

//    /*
//    //	The following flags should be mutually exclusive.
//    bool titleRenameFlag=0;
//    bool mergeToNewSongFlag=0;
//    bool mergeToExistingSongFlag=0;
//    bool updatePerformerFlag=0;

//    //	The following flags can be set independently from eachother.
//    //	However, they can be turned of by detecting any of the flags above.
//    bool yearOfReleaseChangedFlag=0;
//    bool notesChangedFlag=0;
//    bool lyricsChangedFlag=0;
//    bool extraSQLFlag=0;

//    qDebug() << SB_DEBUG_INFO << "old"
//        << ":sb_song_id=" << oldSongID.songID()
//        << ":sb_song_performer_id=" << oldSongID.songPerformerID()
//        << ":isOriginalPerformerFlag=" << oldSongID.originalPerformerFlag()
//    ;
//    qDebug() << SB_DEBUG_INFO << "new"
//        << ":sb_song_id=" << newSongID.songID()
//        << ":sb_song_performer_id=" << newSongID.songPerformerID()
//        << ":isOriginalPerformerFlag=" << newSongID.originalPerformerFlag()
//    ;

//    //	1.	Set attribute flags
//    if(oldSongID.year()!=newSongID.year())
//    {
//        yearOfReleaseChangedFlag=1;
//    }
//    if(oldSongID.notes()!=newSongID.notes())
//    {
//        notesChangedFlag=1;
//    }
//    if(oldSongID.lyrics()!=newSongID.lyrics())
//    {
//        lyricsChangedFlag=1;
//    }

//    //	2.	Determine what need to be done.
//    if(newSongID.songID()==-1)
//    {
//        //	New song does NOT exists
//        if(oldSongID.songPerformerID()!=newSongID.songPerformerID())
//        {
//            //	Different performer
//            mergeToNewSongFlag=1;
//        }
//        else
//        {
//            //	Same performer
//            titleRenameFlag=1;
//            newSongID.setSongID(oldSongID.songID());
//        }
//    }
//    else
//    {
//        //	New song exists
//        if(oldSongID.songID()!=newSongID.songID())
//        {
//            //	Songs are not the same -> merge
//            mergeToExistingSongFlag=1;
//        }
//        else if(oldSongID.songPerformerID()!=newSongID.songPerformerID())
//        {
//            //	Songs are the same, update performer
//            updatePerformerFlag=1;
//        }
//    }

//    if(extraSQL.count()>0)
//    {
//        extraSQLFlag=1;
//    }

//    //	3.	Sanity check on flags
//    if(
//        titleRenameFlag==0 &&
//        mergeToNewSongFlag==0 &&
//        mergeToExistingSongFlag==0 &&
//        updatePerformerFlag==0 &&

//        yearOfReleaseChangedFlag==0 &&
//        notesChangedFlag==0 &&
//        lyricsChangedFlag==0 &&

//        extraSQLFlag==0
//    )
//    {
//        SBMessageBox::standardWarningBox("No flags are set in saveSong");
//        return 0;
//    }

//    if((int)titleRenameFlag+(int)mergeToNewSongFlag+(int)mergeToExistingSongFlag+(int)updatePerformerFlag>1)
//    {
//        SBMessageBox::standardWarningBox("SaveSong: multiple flags set!");
//        return 0;
//    }

//    //	Discard attribute changes when merging
//    if(mergeToExistingSongFlag || mergeToNewSongFlag)
//    {
//        yearOfReleaseChangedFlag=0;
//        notesChangedFlag=0;
//        lyricsChangedFlag=0;
//        extraSQLFlag=0;
//    }

//    //	4.	Collect work to be done.
//    if(extraSQLFlag==1)
//    {
//        allQueries.append(extraSQL);
//    }

//    //		A.	Attribute changes
//    if(lyricsChangedFlag==1)
//    {
//        //	Insert record if not exists.
//        q=QString
//        (
//            "INSERT INTO "
//                "___SB_SCHEMA_NAME___lyrics "
//            "SELECT DISTINCT "
//                "%1,'' "
//            "FROM "
//                "___SB_SCHEMA_NAME___lyrics "
//            "WHERE "
//                "NOT EXISTS "
//                "( "
//                    "SELECT "
//                        "NULL "
//                    "FROM "
//                        "___SB_SCHEMA_NAME___lyrics "
//                    "WHERE "
//                        "song_id=%1 "
//                ")"
//        )
//            .arg(newSongID.songID())
//        ;
//        allQueries.append(q);

//        //	Now do the update
//        q=QString
//        (
//            "UPDATE ___SB_SCHEMA_NAME___lyrics "
//            "SET "
//                "lyrics='%1' "
//            "WHERE "
//                "song_id=%2 "
//        )
//            .arg(Common::escapeSingleQuotes(newSongID.lyrics()))
//            .arg(newSongID.songID())
//        ;
//        allQueries.append(q);
//    }

//    if(notesChangedFlag==1)
//    {
//        q=QString
//        (
//            "UPDATE ___SB_SCHEMA_NAME___song "
//            "SET "
//                "notes='%1' "
//            "WHERE "
//                "song_id=%2 "
//        )
//            .arg(Common::escapeSingleQuotes(newSongID.notes()))
//            .arg(newSongID.songID())
//        ;
//        allQueries.append(q);
//    }

//    if(yearOfReleaseChangedFlag==1)
//    {
//        q=QString
//        (
//            "UPDATE ___SB_SCHEMA_NAME___performance "
//            "SET "
//                "year='%1' "
//            "WHERE "
//                "song_id=%2 AND "
//                "artist_id=%3 "
//        )
//            .arg(newSongID.year())
//            .arg(newSongID.songID())
//            .arg(newSongID.songPerformerID())
//        ;
//        allQueries.append(q);
//    }

//    if(titleRenameFlag==1 || mergeToNewSongFlag==1)
//    {
//        q=QString
//        (
//            "UPDATE ___SB_SCHEMA_NAME___song "
//            "SET "
//                "title='%1', "
//                "soundex='%2' "
//            "WHERE "
//                "song_id=%3 "
//        )
//            .arg(Common::escapeSingleQuotes(newSongID.songTitle()))
//            .arg(Common::soundex(newSongID.songTitle()))
//            .arg(oldSongID.songID())
//        ;
//        allQueries.append(q);
//        newSongID.setSongID(oldSongID.songID());
//    }

//    //		B.	Non-attribute changes
//    //			A.	Create
//    if(updatePerformerFlag==1 || mergeToNewSongFlag==1 || mergeToExistingSongFlag==1)
//    {
//        //	Create performance if it does not exists.
//        q=QString
//        (
//            "INSERT INTO ___SB_SCHEMA_NAME___performance "
//            "( "
//                "song_id, "
//                "artist_id, "
//                "sole_id, "
//                "year "
//            ") "
//            "SELECT DISTINCT "
//                "%1, "
//                "%2, "
//                "0, "
//                "year "
//            "FROM "
//                "___SB_SCHEMA_NAME___performance "
//            "WHERE "
//                "song_id=%1 AND "
//                "sole_id=0 AND "
//                "NOT EXISTS "
//                "( "
//                    "SELECT "
//                        "NULL "
//                    "FROM "
//                        "___SB_SCHEMA_NAME___performance "
//                    "WHERE "
//                        "song_id=%1 AND "
//                        "artist_id=%2 "
//                ") "
//        )
//            .arg(newSongID.songID())
//            .arg(newSongID.songPerformerID())
//        ;
//        allQueries.append(q);
//    }

//    //			B.	Update
//    if(updatePerformerFlag==1 || mergeToNewSongFlag==1)
//    {
//        //	Update non-original performances
//        q=QString
//        (
//            "UPDATE "
//                "___SB_SCHEMA_NAME___performance "
//            "SET "
//                "song_id=%2 "
//            "WHERE "
//                "song_id=%1 "
//        )
//            .arg(newSongID.songID())
//            .arg(oldSongID.songID())
//        ;
//        allQueries.append(q);

//        //	Switch flag
//        q=QString
//        (
//            "UPDATE "
//                "___SB_SCHEMA_NAME___performance "
//            "SET "
//                "sole_id=CASE WHEN artist_id=%1 THEN 0 ELSE 1 END "
//            "WHERE "
//                "song_id=%2 "
//        )
//            .arg(newSongID.songPerformerID())
//            .arg(newSongID.songID())
//        ;
//        allQueries.append(q);
//    }

//    if(mergeToExistingSongFlag==1 || mergeToNewSongFlag==1)
//    {
//        //	Merge old with new.

//        //	1.	Update performance tables
//        QStringList performanceTable;
//        performanceTable.append("chart_performance");
//        performanceTable.append("collection_performance");
//        performanceTable.append("online_performance");
//        //performanceTable.append("playlist_performance");

//        for(int i=0;i<performanceTable.size();i++)
//        {
//            q=QString
//            (
//                "UPDATE "
//                    "___SB_SCHEMA_NAME___%1 "
//                "SET "
//                    "song_id=%2, "
//                    "artist_id=CASE WHEN artist_id=%5 THEN %3 ELSE artist_id END "
//                "WHERE "
//                    "song_id=%4 "
//             )
//                .arg(performanceTable.at(i))
//                .arg(newSongID.songID())
//                .arg(newSongID.songPerformerID())
//                .arg(oldSongID.songID())
//                .arg(oldSongID.songPerformerID())
//            ;
//            allQueries.append(q);
//        }

//        //	2.	Update record_performance for non-op_ fields
//        q=QString
//        (
//            "UPDATE "
//                "___SB_SCHEMA_NAME___record_performance "
//            "SET     "
//                "song_id=%1, "
//                "artist_id=CASE WHEN artist_id=%4 THEN %2 ELSE artist_id END "
//            "WHERE "
//                "song_id=%3 "
//         )
//            .arg(newSongID.songID())
//            .arg(newSongID.songPerformerID())
//            .arg(oldSongID.songID())
//            .arg(oldSongID.songPerformerID())
//        ;
//        allQueries.append(q);

//        //	3.	Update record_performance for op_ fields
//        q=QString
//        (
//            "UPDATE "
//                "___SB_SCHEMA_NAME___record_performance "
//            "SET     "
//                "op_song_id=%1, "
//                "op_artist_id=CASE WHEN op_artist_id=%4 THEN %2 ELSE artist_id END "
//            "WHERE "
//                "op_song_id=%3 "
//         )
//            .arg(newSongID.songID())
//            .arg(newSongID.songPerformerID())
//            .arg(oldSongID.songID())
//            .arg(oldSongID.songPerformerID())
//        ;
//        allQueries.append(q);
//    }

//    if(mergeToNewSongFlag==1)
//    {
//        //	1.	Update lyrics to point to new song
//        q=QString
//        (
//            "UPDATE "
//                "___SB_SCHEMA_NAME___lyrics "
//            "SET     "
//                "song_id=%1 "
//            "WHERE "
//                "song_id=%2 "
//         )
//            .arg(newSongID.songID())
//            .arg(oldSongID.songID())
//        ;
//        allQueries.append(q);
//    }

//    //			C.	Remove
//    if(mergeToExistingSongFlag==1)
//    {
//        //	1.	Remove lyrics
//        q=QString
//        (
//            "DELETE FROM "
//                "___SB_SCHEMA_NAME___lyrics "
//            "WHERE "
//                "song_id=%1 "
//        )
//            .arg(oldSongID.songID())
//        ;
//        allQueries.append(q);

//        //	2.	Remove online_performance
//        q=QString
//        (
//            "DELETE FROM "
//                "___SB_SCHEMA_NAME___online_performance "
//            "WHERE "
//                "song_id=%1 "
//        )
//            .arg(oldSongID.songID())
//        ;
//        allQueries.append(q);

//        //	3.	Remove toplay

//    if(mergeToExistingSongFlag==1 || mergeToNewSongFlag==1)
//    {
//        //	Remove original performance
//        q=QString
//        (
//            "DELETE FROM "
//                "___SB_SCHEMA_NAME___performance "
//            "WHERE "
//                "song_id=%1 "
//                //"artist_id=%2 "
//        )
//            .arg(oldSongID.songID())
//            //.arg(oldSongID.songPerformerID())
//        ;
//        allQueries.append(q);
//    }

//    if(mergeToExistingSongFlag==1)
//    {
//        //	Remove original song
//        q=QString
//        (
//            "DELETE FROM ___SB_SCHEMA_NAME___song "
//            "WHERE song_id=%1 "
//        )
//            .arg(oldSongID.songID())
//        ;
//        allQueries.append(q);
//    }

//    resultFlag=dal->executeBatch(allQueries,commitFlag);
//    */

//    return resultFlag;
//}

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
    SBIDSongPerformanceMgr* spMgr=Context::instance()->getSongPerformanceMgr();

    if(_songPerformances.contains(spPtr->songPerformerID()))
    {
        qDebug() << SB_DEBUG_INFO << ID() << "Removing performance for performerID=" << spPtr->songPerformerID();
        _songPerformances.remove(spPtr->songPerformerID());
    }
    spPtr->setDeletedFlag();
    spMgr->setChanged(spPtr);
}

///	Pointers
SBIDSongPerformancePtr
SBIDSong::originalSongPerformancePtr() const
{
    SBIDSongPerformanceMgr* spMgr=Context::instance()->getSongPerformanceMgr();
    return spMgr->retrieve(
                SBIDSongPerformance::createKey(_originalSongPerformanceID),
                SBIDManagerTemplate<SBIDSongPerformance,SBIDBase>::open_flag_parentonly);
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
            .arg(_songID)
            .arg(_songTitle)
            .arg(_originalSongPerformanceID)
    ;
}

//	Methods required by SBIDManagerTemplate
QString
SBIDSong::createKey(int songID)
{
    return songID>=0?QString("%1:%2")
        .arg(SBIDBase::sb_type_song)
        .arg(songID):QString("x:x")	//	return invalid key if songID<0
    ;
}

QString
SBIDSong::key() const
{
    return createKey(this->songID());
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
            "s.title, "
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
        .arg(SBIDBase::sb_type_song)
        .arg(SBIDBase::sb_type_performer)
        .arg(SBIDBase::sb_type_album)
    ;

    return new SBSqlQueryModel(q);
}

SBIDSongPtr
SBIDSong::retrieveSong(int songID,bool noDependentsFlag)
{
    SBIDSongMgr* smgr=Context::instance()->getSongMgr();
    SBIDSongPtr songPtr;
    if(songID>=0)
    {
        songPtr=smgr->retrieve(createKey(songID),(noDependentsFlag==1?SBIDManagerTemplate<SBIDSong,SBIDBase>::open_flag_parentonly:SBIDManagerTemplate<SBIDSong,SBIDBase>::open_flag_default));
    }
    return songPtr;
}

QString
SBIDSong::iconResourceLocationStatic()
{
    return QString(":/images/SongIcon.png");
}

///	Protected methods
SBIDSong::SBIDSong():SBIDBase()
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
    SBIDSong s;
    s._songID   =dal->retrieveLastInsertedKey();
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
    SBIDSong song;
    int i=0;

    song._songID                      =Common::parseIntFieldDB(&r,i++);
    song._songTitle                   =r.value(i++).toString();
    song._notes                       =r.value(i++).toString();
    song._lyrics                      =r.value(i++).toString();
    song._originalSongPerformanceID   =r.value(i++).toInt();

    return std::make_shared<SBIDSong>(song);
}

void
SBIDSong::mergeFrom(SBIDSongPtr &fromPtr)
{
    SBIDSongPerformanceMgr *spMgr=Context::instance()->getSongPerformanceMgr();
    _loadSongPerformances();	//	make sure list is loaded.
    QMapIterator<int,SBIDSongPerformancePtr> it(fromPtr->songPerformances());
    while(it.hasNext())
    {
        it.next();
        int performerID=it.key();
        SBIDSongPerformancePtr spPtr=it.value();

        if(_songPerformances.contains(performerID))
        {
            //	Remove
            spPtr->setDeletedFlag();
        }
        else
        {
            //	Merge to current
            spPtr->setSongID(this->songID());
            _songPerformances[performerID]=spPtr;

        }
        spMgr->setChanged(spPtr);
    }
}

void
SBIDSong::openKey(const QString &key, int& songID)
{
    QStringList l=key.split(":");
    songID=l.count()==2?l[1].toInt():-1;
}

void
SBIDSong::postInstantiate(SBIDSongPtr &ptr)
{
    Q_UNUSED(ptr);
}

SBSqlQueryModel*
SBIDSong::retrieveSQL(const QString& key)
{
    int songID=-1;
    openKey(key,songID);

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
        .arg(key.length()==0?"":QString("WHERE s.song_id=%1").arg(songID))
    ;

    return new SBSqlQueryModel(q);
}

QStringList
SBIDSong::updateSQL(const Common::db_change db_change) const
{
    QStringList SQL;

    if(!mergedFlag() && !deletedFlag() && changedFlag() && db_change==Common::db_update)
    {
        SQL.append(QString
        (
            "UPDATE ___SB_SCHEMA_NAME___song "
            "SET "
                "original_performance_id=%1, "
                "title='%2', "
                "notes='%3' "
            "WHERE "
                "song_id=%4 "
        )
            .arg(_originalSongPerformanceID)
            .arg(Common::escapeSingleQuotes(this->_songTitle))
            .arg(Common::escapeSingleQuotes(this->_notes))
            .arg(this->_songID)
        );
    }
    return SQL;
}

Common::result
SBIDSong::userMatch(const Common::sb_parameters& p, SBIDSongPtr exclude, SBIDSongPtr& found, bool showAllChoicesFlag)
{
    SBIDSongMgr* smgr=Context::instance()->getSongMgr();
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
                    found=std::dynamic_pointer_cast<SBIDSong>(selected);
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

///	Private methods
void
SBIDSong::_copy(const SBIDSong &c)
{
    _songID                    =c._songID;
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
    _sb_item_type=SBIDBase::sb_type_song;

    _songID=-1;
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
    SBIDAlbumPerformanceMgr* apmgr=Context::instance()->getAlbumPerformanceMgr();

    //	Load performances including dependents, this will set its internal pointers
    _albumPerformances=apmgr->retrieveSet(qm,SBIDManagerTemplate<SBIDAlbumPerformance,SBIDBase>::open_flag_default);

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
    SBIDSongPerformanceMgr* spmgr=Context::instance()->getSongPerformanceMgr();

    //	Load performances including dependents, this will set its internal pointers
    QMap<int,SBIDSongPerformancePtr> songPerformances=spmgr->retrieveMap(qm,SBIDManagerTemplate<SBIDSongPerformance,SBIDBase>::open_flag_default);
    delete qm;
    return songPerformances;
}

