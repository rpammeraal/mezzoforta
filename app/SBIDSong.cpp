#include <QDebug>

#include "SBIDSong.h"

#include "Common.h"
#include "Context.h"
#include "DataAccessLayer.h"
#include "SBDialogSelectItem.h"
#include "SBIDPerformance.h"
#include "SBMessageBox.h"
#include "SBSqlQueryModel.h"

///	Ctors
SBIDSong::SBIDSong(const SBIDSong &c):SBIDBase(c)
{
    _lyrics                    =c._lyrics;
    _notes                     =c._notes;
    _sb_song_id                =c._sb_song_id;
    _sb_song_performer_id      =c._sb_song_performer_id;
    _songTitle                 =c._songTitle;
    _year                      =c._year;

    _performerPtr              =c._performerPtr;

    _playlistKey2performanceKey=c._playlistKey2performanceKey;
    _performances              =c._performances;
}

SBIDSong::~SBIDSong()
{

}

///	Public methods
int
SBIDSong::commonPerformerID() const
{
    return this->songPerformerID();
}

QString
SBIDSong::commonPerformerName() const
{
    return this->songPerformerName();
}

QString
SBIDSong::genericDescription() const
{
    return QString("Song - %1 / %2 (Not Available)")
        .arg(this->text())
        .arg(this->songPerformerName())
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
    return this->_sb_song_id;
}

SBIDBase::sb_type
SBIDSong::itemType() const
{
    return SBIDBase::sb_type_song;
}

bool
SBIDSong::save()
{
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
    QStringList SQL;

    if(this->_sb_song_id==-1)
    {
        QString newSoundex=Common::soundex(this->_songTitle);
        QString q;

        //	Find out new songID
        q=QString
        (
            "SELECT "
                "%1(MAX(song_id)+1,0) AS MaxSongID "
            "FROM "
                "___SB_SCHEMA_NAME___song "
        )
            .arg(dal->getIsNull())
        ;

        dal->customize(q);
        qDebug() << SB_DEBUG_INFO << q;

        QSqlQuery select(q,db);
        select.next();

        this->_sb_song_id=select.value(0).toInt();

        //	Last minute cleanup of title
        this->_songTitle=this->_songTitle.simplified();

        //	Insert new song
        SQL.append
        (
            QString
            (
                "INSERT INTO ___SB_SCHEMA_NAME___song "
                "( "
                    "song_id, "
                    "title, "
                    "soundex "
                ") "
                "SELECT "
                    "%1, "
                    "'%2', "
                    "'%3' "
            )
                .arg(this->_sb_song_id)
                .arg(Common::escapeSingleQuotes(this->_songTitle))
                .arg(newSoundex)
        );

        //	Upsert new performance
        SQL.append
        (
            QString
            (
                "INSERT INTO ___SB_SCHEMA_NAME___performance "
                "( "
                    "song_id, "
                    "artist_id, "
                    "role_id, "
                    "year, "
                    "notes "
                ") "
                "SELECT "
                    "d.song_id, "
                    "d.artist_id, "
                    "COALESCE(MIN(p.role_id)+1,d.role_id), "
                    "NULLIF(d.year,0), "
                    "d.notes "
                "FROM "
                    "( "
                        "SELECT "
                            "%1 as song_id, "
                            "%2 as artist_id, "
                            "0 as role_id, "
                            "%3 as year, "
                            "CAST(E'%4' AS VARCHAR) as notes "
                    ") d "
                        "LEFT JOIN ___SB_SCHEMA_NAME___performance p ON "
                            "d.song_id=p.song_id "
                "WHERE "
                    "NOT EXISTS "
                    "( "
                        "SELECT NULL "
                        "FROM ___SB_SCHEMA_NAME___performance p "
                        "WHERE d.song_id=p.song_id AND d.artist_id=p.artist_id "
                    ") "
                "GROUP BY "
                    "d.song_id, "
                    "d.artist_id, "
                    "d.year, "
                    "d.notes, "
                    "d.role_id "
            )
                .arg(this->_sb_song_id)
                .arg(this->_sb_song_performer_id)
                .arg(this->_year)
                .arg(Common::escapeSingleQuotes(this->_notes))
        );
    }
    else
    {
        //	Update existing
    }
    return dal->executeBatch(SQL);
}

void
SBIDSong::sendToPlayQueue(bool enqueueFlag)
{
    QMap<int,SBIDPerformancePtr> list;
    SBIDPerformancePtr performancePtr;

    if(_performances.count()==0 && !_performerPtr)
    {
        const_cast<SBIDSong *>(this)->refreshDependents();
    }

    //	Send the first performance where orginalPerformerFlag is set.
    int originalPerformerID=songPerformerID();
    for(int i=0;i<_performances.size() && list.count()==0;i++)
    {
        performancePtr=_performances.at(i);
        if(performancePtr->songPerformerID()==originalPerformerID && performancePtr->path().length()>0)
        {
            list[list.count()]=performancePtr;
        }
    }

    //	If still empty, take the first available performance
    if(list.count()==0)
    {
        for(int i=0;i<_performances.size() && list.count()==0;i++)
        {
            performancePtr=_performances.at(i);
            if(performancePtr->path().length()>0)
            {
                list[list.count()]=performancePtr;
            }
        }
    }

    SBModelQueuedSongs* mqs=Context::instance()->getSBModelQueuedSongs();
    SB_DEBUG_IF_NULL(mqs);
    mqs->populate(list,enqueueFlag);
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
    if(_performances.count()==0)
    {
        const_cast<SBIDSong *>(this)->refreshDependents();
    }
    tm->populateAlbumsBySong(_performances);
    return tm;
}

QVector<SBIDPerformancePtr>
SBIDSong::allPerformances() const
{
    if(_performances.count()==0)
    {
        const_cast<SBIDSong *>(this)->refreshDependents();
    }
    return _performances;
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
        q+=QString("(SELECT COUNT(*) FROM ___SB_SCHEMA_NAME___%1_performance WHERE song_id=%2) ").arg(t).arg(this->_sb_song_id);
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
            SQL.append(QString("DELETE FROM ___SB_SCHEMA_NAME___%1 WHERE song_id=%2").arg(t).arg(_sb_song_id));
        }
        dal->executeBatch(SQL);
    }
}

int
SBIDSong::numPerformances() const
{
    if(_performances.count()==0)
    {
        const_cast<SBIDSong *>(this)->refreshDependents();
    }
    return _performances.count();
}

SBTableModel*
SBIDSong::playlistList()
{
    if(!_playlistKey2performanceKey.count())
    {
        //	Playlists may not be loaded -- retrieve again
        this->refreshDependents();
    }
    SBTableModel* tm=new SBTableModel();
    tm->populatePlaylists(_playlistKey2performanceKey);
    return tm;
}

SBIDPerformancePtr
SBIDSong::performance(int albumID, int albumPosition) const
{
    SBIDPerformancePtr null;

    if(_performances.count()==0)
    {
        const_cast<SBIDSong *>(this)->refreshDependents();
    }

    for(int i=0;i<_performances.size();i++)
    {
        int currentAlbumID=_performances.at(i)->albumID();
        int currentAlbumPosition=_performances.at(i)->albumPosition();

        if(currentAlbumID==albumID && currentAlbumPosition==albumPosition)
        {
            return _performances.at(i);
        }
    }

    return null;
}

QVector<int>
SBIDSong::performerIDList() const
{
    QVector<int> list;

    if(_performances.count()==0)
    {
        const_cast<SBIDSong *>(this)->refreshDependents();
    }

    for(int i=0;i<_performances.size();i++)
    {
        int performerID=_performances.at(i)->songPerformerID();
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
//            "p. role_id=0 AND "
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

int
SBIDSong::songPerformerID() const
{
    return _sb_song_performer_id;
}

QString
SBIDSong::songPerformerName() const
{
    if(!_performerPtr)
    {
        const_cast<SBIDSong *>(this)->_setPerformerPtr();
    }
    return _performerPtr?_performerPtr->performerName():"SBIDPerformance::songPerformerName()::performerPtr null";
}

bool
SBIDSong::updateExistingSong(const SBIDBase &oldSongID, SBIDSong &newSongID, const QStringList& extraSQL,bool commitFlag)
{
    Q_UNUSED(oldSongID);
    Q_UNUSED(newSongID);
    Q_UNUSED(extraSQL);
    Q_UNUSED(commitFlag);

    //DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    //QStringList allQueries;
    //QString q;
    bool resultFlag=1;

    /*
    //	The following flags should be mutually exclusive.
    bool titleRenameFlag=0;
    bool mergeToNewSongFlag=0;
    bool mergeToExistingSongFlag=0;
    bool updatePerformerFlag=0;

    //	The following flags can be set independently from eachother.
    //	However, they can be turned of by detecting any of the flags above.
    bool yearOfReleaseChangedFlag=0;
    bool notesChangedFlag=0;
    bool lyricsChangedFlag=0;
    bool extraSQLFlag=0;

    qDebug() << SB_DEBUG_INFO << "old"
        << ":sb_song_id=" << oldSongID.songID()
        << ":sb_song_performer_id=" << oldSongID.songPerformerID()
        << ":isOriginalPerformerFlag=" << oldSongID.originalPerformerFlag()
    ;
    qDebug() << SB_DEBUG_INFO << "new"
        << ":sb_song_id=" << newSongID.songID()
        << ":sb_song_performer_id=" << newSongID.songPerformerID()
        << ":isOriginalPerformerFlag=" << newSongID.originalPerformerFlag()
    ;

    //	1.	Set attribute flags
    if(oldSongID.year()!=newSongID.year())
    {
        yearOfReleaseChangedFlag=1;
    }
    if(oldSongID.notes()!=newSongID.notes())
    {
        notesChangedFlag=1;
    }
    if(oldSongID.lyrics()!=newSongID.lyrics())
    {
        lyricsChangedFlag=1;
    }

    //	2.	Determine what need to be done.
    if(newSongID.songID()==-1)
    {
        //	New song does NOT exists
        if(oldSongID.songPerformerID()!=newSongID.songPerformerID())
        {
            //	Different performer
            mergeToNewSongFlag=1;
        }
        else
        {
            //	Same performer
            titleRenameFlag=1;
            newSongID.setSongID(oldSongID.songID());
        }
    }
    else
    {
        //	New song exists
        if(oldSongID.songID()!=newSongID.songID())
        {
            //	Songs are not the same -> merge
            mergeToExistingSongFlag=1;
        }
        else if(oldSongID.songPerformerID()!=newSongID.songPerformerID())
        {
            //	Songs are the same, update performer
            updatePerformerFlag=1;
        }
    }

    if(extraSQL.count()>0)
    {
        extraSQLFlag=1;
    }

    //	3.	Sanity check on flags
    if(
        titleRenameFlag==0 &&
        mergeToNewSongFlag==0 &&
        mergeToExistingSongFlag==0 &&
        updatePerformerFlag==0 &&

        yearOfReleaseChangedFlag==0 &&
        notesChangedFlag==0 &&
        lyricsChangedFlag==0 &&

        extraSQLFlag==0
    )
    {
        SBMessageBox::standardWarningBox("No flags are set in saveSong");
        return 0;
    }

    if((int)titleRenameFlag+(int)mergeToNewSongFlag+(int)mergeToExistingSongFlag+(int)updatePerformerFlag>1)
    {
        SBMessageBox::standardWarningBox("SaveSong: multiple flags set!");
        return 0;
    }

    //	Discard attribute changes when merging
    if(mergeToExistingSongFlag || mergeToNewSongFlag)
    {
        yearOfReleaseChangedFlag=0;
        notesChangedFlag=0;
        lyricsChangedFlag=0;
        extraSQLFlag=0;
    }

    //	4.	Collect work to be done.
    if(extraSQLFlag==1)
    {
        allQueries.append(extraSQL);
    }

    //		A.	Attribute changes
    if(lyricsChangedFlag==1)
    {
        //	Insert record if not exists.
        q=QString
        (
            "INSERT INTO "
                "___SB_SCHEMA_NAME___lyrics "
            "SELECT DISTINCT "
                "%1,'' "
            "FROM "
                "___SB_SCHEMA_NAME___lyrics "
            "WHERE "
                "NOT EXISTS "
                "( "
                    "SELECT "
                        "NULL "
                    "FROM "
                        "___SB_SCHEMA_NAME___lyrics "
                    "WHERE "
                        "song_id=%1 "
                ")"
        )
            .arg(newSongID.songID())
        ;
        allQueries.append(q);

        //	Now do the update
        q=QString
        (
            "UPDATE ___SB_SCHEMA_NAME___lyrics "
            "SET "
                "lyrics='%1' "
            "WHERE "
                "song_id=%2 "
        )
            .arg(Common::escapeSingleQuotes(newSongID.lyrics()))
            .arg(newSongID.songID())
        ;
        allQueries.append(q);
    }

    if(notesChangedFlag==1)
    {
        q=QString
        (
            "UPDATE ___SB_SCHEMA_NAME___song "
            "SET "
                "notes='%1' "
            "WHERE "
                "song_id=%2 "
        )
            .arg(Common::escapeSingleQuotes(newSongID.notes()))
            .arg(newSongID.songID())
        ;
        allQueries.append(q);
    }

    if(yearOfReleaseChangedFlag==1)
    {
        q=QString
        (
            "UPDATE ___SB_SCHEMA_NAME___performance "
            "SET "
                "year='%1' "
            "WHERE "
                "song_id=%2 AND "
                "artist_id=%3 "
        )
            .arg(newSongID.year())
            .arg(newSongID.songID())
            .arg(newSongID.songPerformerID())
        ;
        allQueries.append(q);
    }

    if(titleRenameFlag==1 || mergeToNewSongFlag==1)
    {
        q=QString
        (
            "UPDATE ___SB_SCHEMA_NAME___song "
            "SET "
                "title='%1', "
                "soundex='%2' "
            "WHERE "
                "song_id=%3 "
        )
            .arg(Common::escapeSingleQuotes(newSongID.songTitle()))
            .arg(Common::soundex(newSongID.songTitle()))
            .arg(oldSongID.songID())
        ;
        allQueries.append(q);
        newSongID.setSongID(oldSongID.songID());
    }

    //		B.	Non-attribute changes
    //			A.	Create
    if(updatePerformerFlag==1 || mergeToNewSongFlag==1 || mergeToExistingSongFlag==1)
    {
        //	Create performance if it does not exists.
        q=QString
        (
            "INSERT INTO ___SB_SCHEMA_NAME___performance "
            "( "
                "song_id, "
                "artist_id, "
                "role_id, "
                "year "
            ") "
            "SELECT DISTINCT "
                "%1, "
                "%2, "
                "0, "
                "year "
            "FROM "
                "___SB_SCHEMA_NAME___performance "
            "WHERE "
                "song_id=%1 AND "
                "role_id=0 AND "
                "NOT EXISTS "
                "( "
                    "SELECT "
                        "NULL "
                    "FROM "
                        "___SB_SCHEMA_NAME___performance "
                    "WHERE "
                        "song_id=%1 AND "
                        "artist_id=%2 "
                ") "
        )
            .arg(newSongID.songID())
            .arg(newSongID.songPerformerID())
        ;
        allQueries.append(q);
    }

    //			B.	Update
    if(updatePerformerFlag==1 || mergeToNewSongFlag==1)
    {
        //	Update non-original performances
        q=QString
        (
            "UPDATE "
                "___SB_SCHEMA_NAME___performance "
            "SET "
                "song_id=%2 "
            "WHERE "
                "song_id=%1 "
        )
            .arg(newSongID.songID())
            .arg(oldSongID.songID())
        ;
        allQueries.append(q);

        //	Switch flag
        q=QString
        (
            "UPDATE "
                "___SB_SCHEMA_NAME___performance "
            "SET "
                "role_id=CASE WHEN artist_id=%1 THEN 0 ELSE 1 END "
            "WHERE "
                "song_id=%2 "
        )
            .arg(newSongID.songPerformerID())
            .arg(newSongID.songID())
        ;
        allQueries.append(q);
    }

    if(mergeToExistingSongFlag==1 || mergeToNewSongFlag==1)
    {
        //	Merge old with new.

        //	1.	Update performance tables
        QStringList performanceTable;
        performanceTable.append("chart_performance");
        performanceTable.append("collection_performance");
        performanceTable.append("online_performance");
        performanceTable.append("playlist_performance");

        for(int i=0;i<performanceTable.size();i++)
        {
            q=QString
            (
                "UPDATE "
                    "___SB_SCHEMA_NAME___%1 "
                "SET "
                    "song_id=%2, "
                    "artist_id=CASE WHEN artist_id=%5 THEN %3 ELSE artist_id END "
                "WHERE "
                    "song_id=%4 "
             )
                .arg(performanceTable.at(i))
                .arg(newSongID.songID())
                .arg(newSongID.songPerformerID())
                .arg(oldSongID.songID())
                .arg(oldSongID.songPerformerID())
            ;
            allQueries.append(q);
        }

        //	2.	Update record_performance for non-op_ fields
        q=QString
        (
            "UPDATE "
                "___SB_SCHEMA_NAME___record_performance "
            "SET     "
                "song_id=%1, "
                "artist_id=CASE WHEN artist_id=%4 THEN %2 ELSE artist_id END "
            "WHERE "
                "song_id=%3 "
         )
            .arg(newSongID.songID())
            .arg(newSongID.songPerformerID())
            .arg(oldSongID.songID())
            .arg(oldSongID.songPerformerID())
        ;
        allQueries.append(q);

        //	3.	Update record_performance for op_ fields
        q=QString
        (
            "UPDATE "
                "___SB_SCHEMA_NAME___record_performance "
            "SET     "
                "op_song_id=%1, "
                "op_artist_id=CASE WHEN op_artist_id=%4 THEN %2 ELSE artist_id END "
            "WHERE "
                "op_song_id=%3 "
         )
            .arg(newSongID.songID())
            .arg(newSongID.songPerformerID())
            .arg(oldSongID.songID())
            .arg(oldSongID.songPerformerID())
        ;
        allQueries.append(q);
    }

    if(mergeToNewSongFlag==1)
    {
        //	1.	Update lyrics to point to new song
        q=QString
        (
            "UPDATE "
                "___SB_SCHEMA_NAME___lyrics "
            "SET     "
                "song_id=%1 "
            "WHERE "
                "song_id=%2 "
         )
            .arg(newSongID.songID())
            .arg(oldSongID.songID())
        ;
        allQueries.append(q);
    }

    //			C.	Remove
    if(mergeToExistingSongFlag==1)
    {
        //	1.	Remove lyrics
        q=QString
        (
            "DELETE FROM "
                "___SB_SCHEMA_NAME___lyrics "
            "WHERE "
                "song_id=%1 "
        )
            .arg(oldSongID.songID())
        ;
        allQueries.append(q);

        //	2.	Remove online_performance
        q=QString
        (
            "DELETE FROM "
                "___SB_SCHEMA_NAME___online_performance "
            "WHERE "
                "song_id=%1 "
        )
            .arg(oldSongID.songID())
        ;
        allQueries.append(q);

        //	3.	Remove toplay
        q=QString
        (
            "DELETE FROM "
                "___SB_SCHEMA_NAME___toplay "
            "WHERE "
                "song_id=%1 "
        )
            .arg(oldSongID.songID())
        ;
        allQueries.append(q);
    }

    if(mergeToExistingSongFlag==1 || mergeToNewSongFlag==1)
    {
        //	Remove original performance
        q=QString
        (
            "DELETE FROM "
                "___SB_SCHEMA_NAME___performance "
            "WHERE "
                "song_id=%1 "
                //"artist_id=%2 "
        )
            .arg(oldSongID.songID())
            //.arg(oldSongID.songPerformerID())
        ;
        allQueries.append(q);
    }

    if(mergeToExistingSongFlag==1)
    {
        //	Remove original song
        q=QString
        (
            "DELETE FROM ___SB_SCHEMA_NAME___song "
            "WHERE song_id=%1 "
        )
            .arg(oldSongID.songID())
        ;
        allQueries.append(q);
    }

    resultFlag=dal->executeBatch(allQueries,commitFlag);
    */

    return resultFlag;
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

///	Operators
SBIDSong::operator QString() const
{
    QString songTitle=this->_songTitle.length() ? this->_songTitle:"<N/A>";
    QString songPerformerName=_performerPtr?this->songPerformerName():"not retrieved yet";

    return QString("SBIDSong:%1:t=%2:p=%3 %4")
            .arg(this->_sb_song_id)
            .arg(songTitle)
            .arg(songPerformerName)
            .arg(this->_sb_song_performer_id)
    ;
}

//	Methods required by SBIDManagerTemplate
QString
SBIDSong::key() const
{
    return createKey(this->songID());
}

void
SBIDSong::refreshDependents(bool showProgressDialogFlag,bool forcedFlag)
{
    Q_UNUSED(showProgressDialogFlag);

    if(forcedFlag==1 || _performances.count()>=0)
    {
        _loadPerformances();
    }
    if(forcedFlag==1 || _playlistKey2performanceKey.count()==0)
    {
        _loadPlaylists();
    }
    _setPerformerPtr();
}


//	Static methods
SBSqlQueryModel*
SBIDSong::retrieveAllSongs()
{
    //	List songs with actual online performance only
    QString q=QString
    (
        "SELECT DISTINCT "
            "SB_KEYWORDS, "
            "-1 AS SB_ITEM_TYPE1, "
            "CAST(%1 AS VARCHAR)||':'||CAST(SB_ALBUM_ID AS VARCHAR)||':'||CAST(SB_POSITION_ID AS VARCHAR) AS SB_ITEM_KEY1, "
            "songTitle AS \"song title\", "
            "-1 AS SB_ITEM_TYPE2, "
            "CAST(%2 AS VARCHAR)||':'||CAST(SB_PERFORMER_ID AS VARCHAR) AS SB_ITEM_KEY2, "
            "artistName AS \"performer\", "
            "-1 AS SB_ITEM_TYPE3, "
            "CAST(%3 AS VARCHAR)||':'||CAST(SB_ALBUM_ID AS VARCHAR) AS SB_ITEM_KEY3, "
            "recordTitle AS \"album title\" "
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
                    "s.title || ' ' || a.name || ' ' || r.title  AS SB_KEYWORDS "
                "FROM "
                    "___SB_SCHEMA_NAME___record_performance rp  "
                        "JOIN ___SB_SCHEMA_NAME___artist a ON  "
                            "rp.artist_id=a.artist_id "
                        "JOIN ___SB_SCHEMA_NAME___record r ON  "
                            "rp.record_id=r.record_id "
                        "JOIN ___SB_SCHEMA_NAME___song s ON  "
                            "rp.song_id=s.song_id "
                        "JOIN ___SB_SCHEMA_NAME___online_performance op ON "
                            "rp.op_song_id=op.song_id AND "
                            "rp.op_artist_id=op.artist_id AND "
                            "rp.op_record_id=op.record_id AND "
                            "rp.op_record_position=op.record_position "
            ") a "
        "ORDER BY 4,7,10 "
    )
        .arg(SBIDBase::sb_type_performance)
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

QString
SBIDSong::createKey(int songID)
{
    return songID>=0?QString("%1:%2")
        .arg(SBIDBase::sb_type_song)
        .arg(songID):QString("x:x")	//	return invalid key if songID<0
    ;
}

SBIDSongPtr
SBIDSong::createInDB()
{
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
    QString q;

    //	Get next ID available
    q=QString("SELECT %1(MAX(song_id),0)+1 FROM ___SB_SCHEMA_NAME___song ").arg(dal->getIsNull());
    dal->customize(q);
    qDebug() << SB_DEBUG_INFO << q;
    QSqlQuery qID(q,db);
    qID.next();

    //	Give new song unique name
    int maxNum=1;
    QString newSongTitle="New Song";
    q=QString("SELECT title FROM ___SB_SCHEMA_NAME___song WHERE name %1 \"%2%\"")
        .arg(dal->getILike())
        .arg(newSongTitle)
    ;
    dal->customize(q);
    qDebug() << SB_DEBUG_INFO << q;
    QSqlQuery qName(q,db);

    while(qName.next())
    {
        QString existing=qName.value(0).toString();
        existing.replace(newSongTitle,"");
        int i=existing.toInt();
        if(i>=maxNum)
        {
            maxNum=i+1;
        }
    }

    //	Instantiate
    SBIDPerformerPtr variousPerformerPtr=SBIDPerformer::retrieveVariousArtists();
    SBIDSong song;
    song._sb_song_id=qID.value(0).toInt();
    song._songTitle=QString("%1%2").arg(newSongTitle).arg(maxNum);
    song._sb_song_performer_id=variousPerformerPtr->performerID();
    song._year=QDate(QDate::currentDate()).year();
    song._notes="Inserted by us!";

    QStringList SQL;

    //	Insert
    SQL.append(QString
    (
        "INSERT INTO ___SB_SCHEMA_NAME___song "
        "( "
            "song_id, "
            "title, "
            "notes, "
            "soundex "
        ") "
        "SELECT "
            "%1, "
            "'%2', "
            "'%3', "
            "'%4' "
    )
        .arg(song._sb_song_id)
        .arg(song._songTitle)
        .arg(song._notes)
        .arg(Common::soundex(song._songTitle))
    );

    SQL.append(QString
    (
        "INSERT INTO ___SB_SCHEMA_NAME___performance "
        "( "
            "song_id, "
            "artist_id, "
            "role_id, "
            "year, "
            "notes "
        ") "
        "SELECT "
            "%1 as song_id, "
            "%2 as artist_id, "
            "0 as role_id, "
            "%3 as year, "
            "CAST(E'%4' AS VARCHAR) as notes "
    )
        .arg(song._sb_song_id)
        .arg(song._sb_song_performer_id)
        .arg(song._year)
        .arg(Common::escapeSingleQuotes(song._notes))
    );

    dal->customize(q);
    qDebug() << SB_DEBUG_INFO << q;
    QSqlQuery insert(q,db);
    Q_UNUSED(insert);

    bool successFlag=dal->executeBatch(SQL);
    if(successFlag)
    {
        return std::make_shared<SBIDSong>(song);
    }
    return SBIDSongPtr();
}

SBSqlQueryModel*
SBIDSong::find(const Common::sb_parameters& tobeFound,SBIDSongPtr existingSongPtr)
{
    QString newSoundex=Common::soundex(tobeFound.songTitle);
    int excludeID=(existingSongPtr?existingSongPtr->songID():-1);

    //	MatchRank:
    //	0	-	exact match with specified artist (0 or 1 in data set).
    //	1	-	exact match with any other artist (0 or more in data set).
    //	2	-	soundex match with any other artist (0 or more in data set).
    QString q=QString
    (
        "SELECT "
            "CASE WHEN p.artist_id=%2 THEN 0 ELSE 1 END AS matchRank, "
            "s.song_id, "
            "s.title, "
            "s.notes, "
            "p.artist_id, "
            "p.year, "
            "l.lyrics "
        "FROM "
            "___SB_SCHEMA_NAME___performance p "
                "JOIN ___SB_SCHEMA_NAME___song s ON "
                    "p.song_id=s.song_id "
                    "%4 "
                "LEFT JOIN ___SB_SCHEMA_NAME___lyrics l ON "
                    "s.song_id=l.song_id "
        "WHERE "
            "REPLACE(LOWER(s.title),' ','') = REPLACE(LOWER('%1'),' ','') "
        "UNION "
        "SELECT "
            "2 AS matchRank, "
            "s.song_id, "
            "s.title, "
            "s.notes, "
            "p.artist_id, "
            "p.year, "
            "l.lyrics "
        "FROM "
            "___SB_SCHEMA_NAME___performance p "
                "JOIN ___SB_SCHEMA_NAME___song s ON "
                    "p.song_id=s.song_id "
                    "%4 "
                "LEFT JOIN ___SB_SCHEMA_NAME___lyrics l ON "
                    "s.song_id=l.song_id "
        "WHERE "
            "p. role_id=0 AND "
            "( "
                "SUBSTR(s.soundex,1,LENGTH('%3'))='%3' OR "
                "SUBSTR('%3',1,LENGTH(s.soundex))=s.soundex "
            ") "
        "ORDER BY "
            "1,3 "

    )
        .arg(Common::simplified(tobeFound.songTitle))
        .arg(tobeFound.performerID)
        .arg(newSoundex)
        .arg(excludeID==-1?"":QString(" AND s.song_id=%1").arg(excludeID))
    ;
    return new SBSqlQueryModel(q);
}

SBIDSongPtr
SBIDSong::instantiate(const QSqlRecord &r)
{
    SBIDSong song;
    song._sb_song_id           =r.value(0).toInt();
    song._songTitle            =r.value(1).toString();
    song._notes                =r.value(2).toString();
    song._sb_song_performer_id =r.value(3).toInt();
    song._year                 =r.value(4).toInt();
    song._lyrics               =r.value(5).toString();

    return std::make_shared<SBIDSong>(song);
}

void
SBIDSong::mergeTo(SBIDSongPtr &to)
{
    Q_UNUSED(to);
}

void
SBIDSong::openKey(const QString &key, int &songID)
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
            "p.artist_id, "
            "p.year, "
            "l.lyrics "
        "FROM "
            "___SB_SCHEMA_NAME___song s "
                "LEFT JOIN ___SB_SCHEMA_NAME___performance p ON "
                    "s.song_id=p.song_id AND "
                    "p.role_id=0 "
                "LEFT JOIN ___SB_SCHEMA_NAME___lyrics l ON "
                    "s.song_id=l.song_id "
        "%1  "
        "ORDER BY "
            "s.title "
    )
        .arg(key.length()==0?"":QString("WHERE s.song_id=%1").arg(songID))
    ;

    qDebug() << SB_DEBUG_INFO << q;
    return new SBSqlQueryModel(q);
}

QStringList
SBIDSong::updateSQL() const
{
    QStringList SQL;

    if(changedFlag())
    {
        SQL.append(QString
        (
            "UPDATE ___SB_SCHEMA_NAME___song "
            "SET "
                "title='%2', "
                "notes='%3' "
            "WHERE "
                "song_id=%1 "
        )
            .arg(this->songID()))
        ;

        SQL.append(QString
        (
            "UPDATE ___SB_SCHEMA_NAME___performance "
            "SET "
                "year=%3, "
                "notes='%4' "
            "WHERE "
                "song_id=%1 AND "
                "artist_id=%2 "
        )
            .arg(this->songID())
            .arg(this->songPerformerID())
        );
    }

    return SQL;
}

SBIDSongPtr
SBIDSong::userMatch(const Common::sb_parameters& tobeMatched, SBIDSongPtr existingSongPtr)
{
    qDebug() << SB_DEBUG_INFO << tobeMatched.songTitle << tobeMatched.performerID;
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    SBIDSongPtr selectedSongPtr;
    SBIDSongMgr* smgr=Context::instance()->getSongMgr();
    bool resultCode=1;
    QMap<int,QList<SBIDSongPtr>> matches;

    int findCount=smgr->find(tobeMatched,existingSongPtr,matches);

    if(findCount)
    {
        if(matches[0].count()==1)
        {
            //	Dataset indicates an exact match if the 2nd record identifies an exact match.
            selectedSongPtr=matches[0][0];
            resultCode=1;
        }
        else
        {
            //	Dataset has at least two records, of which the 2nd one is an soundex match,
            //	display pop-up
            SBDialogSelectItem* pu=SBDialogSelectItem::selectSong(tobeMatched,existingSongPtr,matches);
            pu->exec();

            //	Go back to screen if no item has been selected
            if(pu->hasSelectedItem()==0)
            {
                qDebug() << SB_DEBUG_INFO << "none selected";
                return selectedSongPtr;
            }
            else
            {
                qDebug() << SB_DEBUG_INFO;
                SBIDPtr selected=pu->getSelected();
                if(selected)
                {
                    //	Existing song is choosen
                    qDebug() << SB_DEBUG_INFO << "EXISTING";
                    selectedSongPtr=std::dynamic_pointer_cast<SBIDSong>(selected);
                }
                else
                {
                    //	New song has been choosen -- create.
                    qDebug() << SB_DEBUG_INFO << "NEW";
                    selectedSongPtr=smgr->createInDB();
                    selectedSongPtr->_setSongTitle(tobeMatched.performerName);
                    selectedSongPtr->_setNotes("populated by us");
                    smgr->commit(selectedSongPtr,dal,0);
                }
            }
        }
    }
    return selectedSongPtr;
}

///	Private methods
void
SBIDSong::_init()
{
    _sb_item_type=SBIDBase::sb_type_song;
    _sb_song_id=-1;
    _performances.clear();
}

void
SBIDSong::_loadPerformances()
{
    SBSqlQueryModel* qm=SBIDPerformance::performancesBySong(songID());
    SBIDPerformanceMgr* pemgr=Context::instance()->getPerformanceMgr();

    //	Load performances including dependents, this will set its internal pointers
    _performances=pemgr->retrieveSet(qm,SBIDManagerTemplate<SBIDPerformance,SBIDBase>::open_flag_default);

    delete qm;
}

void
SBIDSong::_loadPlaylists()
{
    //	Create a map between performance and playlist where this performance exists.
    //	This will not be put in SBIDManagerTemplate
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
    QString q=QString
    (
        "SELECT DISTINCT "
            "pp.playlist_id, "
            "pp.record_id, "
            "pp.record_position "
        "FROM "
            "___SB_SCHEMA_NAME___playlist_performance pp "
        "WHERE "
            "pp.song_id=%4 "
    )
        .arg(this->songID())
    ;
    qDebug() << SB_DEBUG_INFO << q;

    QSqlQuery q1(db);
    q1.exec(dal->customize(q));

    _playlistKey2performanceKey.clear();
    while(q1.next())
    {
        QString performanceKey=SBIDPerformance::createKey(q1.value(1).toInt(),q1.value(2).toInt());
        QString playlistKey=SBIDPlaylist::createKey(q1.value(0).toInt());

        if(!_playlistKey2performanceKey.contains(playlistKey))
        {
            _playlistKey2performanceKey[playlistKey]=performanceKey;
        }
    }
}

void
SBIDSong::_setPerformerPtr()
{
    _performerPtr=SBIDPerformer::retrievePerformer(_sb_song_performer_id,1);
}

void
SBIDSong::_setSongTitle(const QString &songTitle)
{
    _songTitle=songTitle;
    setChangedFlag();
}
