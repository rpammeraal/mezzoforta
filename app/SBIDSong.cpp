#include <QDebug>

#include "SBIDSong.h"

#include "Common.h"
#include "Context.h"
#include "DataAccessLayer.h"
#include "SBIDPerformance.h"
#include "SBMessageBox.h"
#include "SBSqlQueryModel.h"

///	Ctors
SBIDSong::SBIDSong(const SBIDSong &c):SBIDBase(c)
{
    this->_performances=c._performances;
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
    qDebug() << SB_DEBUG_INFO;
    return QString("Song %1")
        .arg(this->text())
    ;
}

QString
SBIDSong::iconResourceLocation() const
{
    return QString(":/images/SongIcon.png");
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

    //	Send the first performance where orginalPerformerFlag is set.
    for(int i=0;i<_performances.size();i++)
    {
        if(_performances.at(i)->originalPerformerFlag()==1)
        {
            list[list.count()]=performancePtr;
        }
    }

    //	If still empty, take the first available performance
    if(list.count()==0 && _performances.count())
    {
        list[list.count()]=_performances.at(0);
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
    tm->populateAlbumsBySong(_performances);
    return tm;
}

//QVector<int>
//SBIDSong::albumIDList() const
//{
//    QVector<int> list;

//    for(int i=0;i<_performances.size();i++)
//    {
//        int albumID=_performances.at(i)->albumID();
//        if(!list.contains(albumID))
//        {
//            list.append(albumID);
//        }
//    }
//    return list;
//}

QVector<SBIDPerformancePtr>
SBIDSong::allPerformances() const
{
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

SBSqlQueryModel*
SBIDSong::getAllSongs()
{
    //	Main query
    QString q=QString
    (
        "SELECT DISTINCT "
            "SB_KEYWORDS, "
            "%1 AS SB_ITEM_TYPE1, "
            "SB_SONG_ID, "
            "songTitle AS \"song title\", "
            "%2 AS SB_ITEM_TYPE2, "
            "SB_PERFORMER_ID, "
            "artistName AS \"performer\", "
            "%3 AS SB_ITEM_TYPE3, "
            "SB_ALBUM_ID, "
            "recordTitle AS \"album title\", "
            "%4 AS SB_ITEM_TYPE4, "
            "SB_POSITION_ID, "
            "path AS SB_PATH, "
            "duration AS SB_DURATION "
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
                    "s.title || ' ' || a.name || ' ' || r.title  AS SB_KEYWORDS, "
                    "op.path, "
                    "rp.duration "
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
    ).
        arg(Common::sb_field_song_id).
        arg(Common::sb_field_performer_id).
        arg(Common::sb_field_album_id).
        arg(Common::sb_field_album_position)
    ;

    return new SBSqlQueryModel(q);
}

SBTableModel*
SBIDSong::playlistList()
{
    if(!_performance2playlistID.count())
    {
        //	Playlists may not be loaded -- retrieve again
        this->_loadPlaylists();
    }
    SBTableModel* tm=new SBTableModel();
    tm->populatePlaylists(_performance2playlistID);
    return tm;
}

SBIDPerformancePtr
SBIDSong::performance(int albumID, int albumPosition) const
{
    SBIDPerformancePtr null;

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
    qDebug() << SB_DEBUG_INFO;
    QVector<int> list;

    for(int i=0;i<_performances.size();i++)
    {
        int performerID=_performances.at(i)->songPerformerID();
        qDebug() << SB_DEBUG_INFO << _performances.at(i)->key() << performerID;
        if(!list.contains(performerID))
        {
            list.append(performerID);
        }
    }
    qDebug() << SB_DEBUG_INFO << list.count();
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


void
SBIDSong::setPlaylistPosition(int playlistPosition)
{
    _sb_playlist_position=playlistPosition;
    setChangedFlag();
}

void
SBIDSong::setSongTitle(const QString &songTitle)
{
    _songTitle=songTitle;
    setChangedFlag();
}

bool
SBIDSong::updateExistingSong(const SBIDBase &oldSongID, SBIDSong &newSongID, const QStringList& extraSQL,bool commitFlag)
{
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QStringList allQueries;
    QString q;
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
bool
SBIDSong::operator ==(const SBIDSong& i) const
{
    if(
        i._sb_song_id==this->_sb_song_id &&
        i._sb_song_performer_id==this->_sb_song_performer_id &&
        i._sb_album_id==this->_sb_album_id &&
        i._sb_album_position==this->_sb_album_position)
    {
        return 1;
    }
    return 0;
}

SBIDSong::operator QString() const
{
    QString songTitle=this->_songTitle.length() ? this->_songTitle : "<N/A>";
    QString songPerformerName=this->_songPerformerName.length() ? this->_songPerformerName : "<N/A>";
    QString albumTitle=this->_albumTitle.length() ? this->_albumTitle : "<N/A>";

    return QString("SBIDSong:%1,%2:t=%3:p=%4 %5,%6:a=%7 %8,%9")
            .arg(this->_sb_song_id)
            .arg(this->_sb_tmp_item_id)
            .arg(songTitle)
            .arg(songPerformerName)
            .arg(this->_sb_song_performer_id)
            .arg(this->_sb_tmp_performer_id)
            .arg(albumTitle)
            .arg(this->_sb_album_id)
            .arg(this->_sb_tmp_album_id)
    ;
}

//	Methods required by SBIDManagerTemplate
QString
SBIDSong::key() const
{
    return createKey(this->songID());
}

//	Static methods
SBIDSongPtr
SBIDSong::retrieveSong(int songID,bool noDependentsFlag)
{
        qDebug() << SB_DEBUG_INFO << songID;
    SBIDSongMgr* smgr=Context::instance()->getSongMgr();
    return smgr->retrieve(createKey(songID),(noDependentsFlag==1?SBIDManagerTemplate<SBIDSong>::open_flag_parentonly:SBIDManagerTemplate<SBIDSong>::open_flag_default));
}

///	Protected methods
SBIDSong::SBIDSong():SBIDBase()
{
    _init();
}

QString
SBIDSong::createKey(int songID)
{
    return QString("%1:%2")
        .arg(SBIDBase::sb_type_song)
        .arg(songID)
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

    //	Instantiate
    SBIDSong song;
    song._sb_song_id=qID.value(0).toInt();
    song._songTitle="Song1";

    //	Give new playlist unique name
    int maxNum=1;
    q=QString("SELECT title FROM ___SB_SCHEMA_NAME___song WHERE name %1 \"New Song%\"").arg(dal->getILike());
    dal->customize(q);
    qDebug() << SB_DEBUG_INFO << q;
    QSqlQuery qName(q,db);

    while(qName.next())
    {
        QString existing=qName.value(0).toString();
        existing.replace("New Song","");
        int i=existing.toInt();
        if(i>=maxNum)
        {
            maxNum=i+1;
        }
    }
    song._songTitle=QString("New Song%1").arg(maxNum);

    //	Insert
    q=QString
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
            "%2, "
            "'', "
            "'%3' "
    )
        .arg(song._sb_song_id)
        .arg(song._songTitle)
        .arg(Common::soundex(song._songTitle))
    ;

    dal->customize(q);
    qDebug() << SB_DEBUG_INFO << q;
    QSqlQuery insert(q,db);
    Q_UNUSED(insert);

    //	Done
    return std::make_shared<SBIDSong>(song);
}

SBSqlQueryModel*
SBIDSong::find(const QString &tobeFound, int excludeItemID, QString secondaryParameter)
{
    QString newSoundex=Common::soundex(tobeFound);

    //	MatchRank:
    //	0	-	edited value (always one in data set).
    //	1	-	exact match with specified artist (0 or 1 in data set).
    //	2	-	exact match with any other artist (0 or more in data set).
    //	3	-	soundex match with any other artist (0 or more in data set).
    QString q=QString
    (
        "SELECT "
            "CASE WHEN a.artist_id=%2 THEN 1 ELSE 2 END AS matchRank, "
            "s.song_id, "
            "s.title, "
            "a.artist_id, "
            "a.name "
        "FROM "
            "___SB_SCHEMA_NAME___performance p "
                "JOIN ___SB_SCHEMA_NAME___song s ON "
                    "p.song_id=s.song_id "
                    "%4 "
                "JOIN ___SB_SCHEMA_NAME___artist a ON "
                    "p.artist_id=a.artist_id "
        "WHERE "
            "REPLACE(LOWER(s.title),' ','') = REPLACE(LOWER('%1'),' ','') "
        "UNION "
        "SELECT "
            "3 AS matchRank, "
            "s.song_id, "
            "s.title, "
            "a.artist_id, "
            "a.name "
        "FROM "
            "___SB_SCHEMA_NAME___performance p "
                "JOIN ___SB_SCHEMA_NAME___song s ON "
                    "p.song_id=s.song_id "
                    "%4 "
                "JOIN ___SB_SCHEMA_NAME___artist a ON "
                    "p.artist_id=a.artist_id "
        "WHERE "
            "p. role_id=0 AND "
            "( "
                "SUBSTR(s.soundex,1,LENGTH('%3'))='%3' OR "
                "SUBSTR('%3',1,LENGTH(s.soundex))=s.soundex "
            ") "
        "ORDER BY "
            "1,3 "

    )
        .arg(Common::simplified(tobeFound))
        .arg(secondaryParameter.toInt())
        .arg(newSoundex)
        .arg(excludeItemID==-1?"":QString(" AND s.song_id=%1").arg(excludeItemID))
    ;
    return new SBSqlQueryModel(q);
}

SBIDSongPtr
SBIDSong::instantiate(const QSqlRecord &r, bool noDependentsFlag)
{
    SBIDSong song;
    song._sb_song_id           =r.value(0).toInt();
    song._songTitle            =r.value(1).toString();
    song._notes                =r.value(2).toString();
    song._sb_song_performer_id =r.value(3).toInt();
    song._songPerformerName    =r.value(4).toString();
    song._year                 =r.value(5).toInt();
    song._lyrics               =r.value(6).toString();
    song._originalPerformerFlag=r.value(7).toBool();

    if(!noDependentsFlag)
    {
        song._loadPlaylists();
        song._loadPerformances();
    }

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
            "a.artist_id, "
            "a.name, "
            "p.year, "
            "l.lyrics, "
            "CASE WHEN p.role_id=0 THEN 1 ELSE 0 END "
        "FROM "
            "___SB_SCHEMA_NAME___song s "
                "LEFT JOIN ___SB_SCHEMA_NAME___performance p ON "
                    "s.song_id=p.song_id "
                "LEFT JOIN ___SB_SCHEMA_NAME___artist a ON "
                    "p.artist_id=a.artist_id "
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

    return SQL;
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
    _performances=pemgr->retrieveSet(qm);
}

void
SBIDSong::_loadPlaylists()
{
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

    SBIDPerformancePtr performancePtr;
    while(q1.next())
    {
        qDebug() << SB_DEBUG_INFO << q1.value(0) << q1.value(1) << q1.value(2);
        performancePtr=performance(q1.value(1).toInt(),q1.value(2).toInt());

        if(performancePtr)
        {
            if(!_performance2playlistID.contains(performancePtr))
            {
                _performance2playlistID[performancePtr]=q1.value(0).toInt();
            }
        }
    }
    qDebug() << SB_DEBUG_INFO;
}
