#include "Common.h"
#include "Controller.h"
#include "Context.h"
#include "DataAccessLayer.h"
#include "SBSqlQueryModel.h"
#include "DataEntitySong.h"

#include <QMessageBox>
#include <QStringList>

SBID
DataEntitySong::getDetail(const SBID& id)
{
    SBID result=id;
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());

    QString q=QString
    (
        "SELECT DISTINCT "
            "s.title, "
            "s.notes, "
            "a.artist_id, "
            "a.name, "
            "p.year, "
            "l.lyrics, "
            "CASE WHEN p.role_id=0 THEN 1 ELSE 0 END "
        "FROM "
            "___SB_SCHEMA_NAME___song s "
                "JOIN ___SB_SCHEMA_NAME___performance p ON "
                    "s.song_id=p.song_id and "
                    "p.role_id=0 "
                "JOIN ___SB_SCHEMA_NAME___artist a ON "
                    "p.artist_id=a.artist_id "
                "LEFT JOIN ___SB_SCHEMA_NAME___lyrics l ON "
                    "s.song_id=l.song_id "
        "WHERE "
             "s.song_id=%1 AND "
             "( "
                "p.artist_id=%2 OR "
                "%2=%2 "
             ") "
    )
        .arg(id.sb_song_id)
        .arg(id.sb_performer_id)
    ;
    dal->customize(q);

    qDebug() << SB_DEBUG_INFO << q;
    QSqlQuery query(q,db);

    if(query.next())
    {
        result.assign(SBID::sb_type_song,id.sb_song_id);
        result.sb_performer_id    =query.value(2).toInt();
        result.performerName      =query.value(3).toString();
        result.songTitle          =query.value(0).toString();
        result.year               =query.value(4).toInt();
        result.lyrics             =query.value(5).toString();
        result.notes              =query.value(1).toString();
        result.isOriginalPerformer=query.value(6).toBool();
    }

    return result;
}

SBSqlQueryModel*
DataEntitySong::findSong(const SBID& id)
{
    QString q=QString
    (
        "SELECT DISTINCT "
            "s.song_id, "
            "s.title, "
            "s.notes, "
            "a.artist_id, "
            "a.name, "
            "p.year, "
            "l.lyrics, "
            "CASE WHEN p.role_id=0 THEN 1 ELSE 0 END "
        "FROM "
            "___SB_SCHEMA_NAME___song s "
                "JOIN ___SB_SCHEMA_NAME___performance p ON "
                    "s.song_id=p.song_id and "
                    "p.role_id=0 "
                "JOIN ___SB_SCHEMA_NAME___artist a ON "
                    "p.artist_id=a.artist_id "
                "LEFT JOIN ___SB_SCHEMA_NAME___lyrics l ON "
                    "s.song_id=l.song_id "
        "WHERE "
             "s.song_id!=%1 AND "
             "s.title='%2' AND "
             "a.name='%3' "
    )
        .arg(id.sb_song_id)
        .arg(id.songTitle)
        .arg(Common::escapeSingleQuotes(id.performerName))
    ;

    return new SBSqlQueryModel(q);
}

SBSqlQueryModel*
DataEntitySong::getAllSongs()
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
            "SB_POSITION_ID "
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
            ") a "
        "ORDER BY 4,7,10 "
    ).
        arg(SBID::sb_type_song).
        arg(SBID::sb_type_performer).
        arg(SBID::sb_type_album).
        arg(SBID::sb_type_position)
    ;

    return new SBSqlQueryModel(q);
}

int
DataEntitySong::getMaxSongID()
{
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
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

    return select.value(0).toInt();
}

SBSqlQueryModel*
DataEntitySong::getPerformedByListBySong(const SBID& id)
{
    QString q=QString
    (
        "SELECT "
            "%1 AS SB_ITEM_TYPE, "
            "a.artist_id AS SB_ITEM_ID, "
            "a.name AS \"performer\", "
            "p.year AS \"year released \" "
        "FROM "
            "___SB_SCHEMA_NAME___performance p "
                "JOIN ___SB_SCHEMA_NAME___artist a ON "
                    "a.artist_id=p.artist_id "
        "WHERE "
            "p.role_id=1 AND "
            "p.song_id=%2 "
        "ORDER BY "
            "a.name"
    )
        .arg(SBID::sb_type_performer)
        .arg(id.sb_song_id)
    ;

    return new SBSqlQueryModel(q);
}

SBSqlQueryModel*
DataEntitySong::getOnAlbumListBySong(const SBID& id)
{
    QString q=QString
    (
        "SELECT DISTINCT "
            "%1 AS SB_ITEM_TYPE1, "
            "r.record_id AS SB_RECORD_ID, "
            "r.title AS \"album title\", "
            "r.year AS \"year released\", "
            "%2 AS SB_ITEM_TYPE2, "
            "a.artist_id AS SB_PERFORMER_ID, "
            "a.name AS \"performer\",  "
            "%3 AS SB_ITEM_TYPE3, "
            "rp.record_position AS SB_POSITION_ID "
        "FROM "
            "___SB_SCHEMA_NAME___record_performance rp "
                "JOIN ___SB_SCHEMA_NAME___record r ON "
                    "rp.record_id=r.record_id "
                "JOIN ___SB_SCHEMA_NAME___artist a ON "
                    "rp.artist_id=a.artist_id "
        "WHERE "
            "rp.song_id=%3 "
        "ORDER BY "
            "r.title "
    )
        .arg(SBID::sb_type_album)
        .arg(SBID::sb_type_performer)
        .arg(id.sb_song_id)
        .arg(SBID::sb_type_position)
    ;

    return new SBSqlQueryModel(q);
}

SBSqlQueryModel*
DataEntitySong::getOnChartListBySong(const SBID& id)
{
    QString q=QString
    (
        "SELECT "
            "%1 AS SB_ITEM_TYPE, "
            "cp.chart_position AS \"position\", "
            "c.chart_id AS SB_ITEM_ID, "
            "c.name AS \"chart\", "
            "a.artist_id AS SB_PERFORMER_ID, "
            "a.name AS \"performer\" "
        "FROM "
            "___SB_SCHEMA_NAME___chart_performance cp "
                "JOIN ___SB_SCHEMA_NAME___chart c ON "
                    "c.chart_id=cp.chart_id "
                "JOIN ___SB_SCHEMA_NAME___artist a ON "
                    "cp.artist_id=a.artist_id "
            "WHERE "
                "cp.song_id=%2"
    )
        .arg(SBID::sb_type_chart)
        .arg(id.sb_song_id)
    ;

    return new SBSqlQueryModel(q);
}

SBSqlQueryModel*
DataEntitySong::getOnPlaylistListBySong(const SBID& id)
{
    QString q=QString
    (
        "SELECT DISTINCT "
            "%1 AS SB_ITEM_TYPE1, "
            "p.playlist_id AS SB_PLAYLIST_ID, "
            "p.name AS \"playlist\", "
            "%2 AS SB_ITEM_TYPE2, "
            "a.artist_id AS SB_PERFORMER_ID, "
            "a.name AS \"performer\", "
            "rp.duration AS \"duration\", "
            "%3 AS SB_ITEM_TYPE3, "
            "r.record_id AS SB_ALBUM_ID, "
            "r.title AS \"album title\" "
        "FROM "
            "___SB_SCHEMA_NAME___playlist_performance pp "
                "JOIN ___SB_SCHEMA_NAME___playlist p ON "
                    "p.playlist_id=pp.playlist_id "
                "JOIN ___SB_SCHEMA_NAME___artist a ON "
                    "pp.artist_id=a.artist_id "
                "JOIN ___SB_SCHEMA_NAME___record_performance rp ON "
                    "pp.artist_id=rp.artist_id AND "
                    "pp.song_id=rp.song_id AND "
                    "pp.record_id=rp.record_id AND "
                    "pp.record_position=rp.record_position "
                "JOIN ___SB_SCHEMA_NAME___record r ON "
                    "pp.record_id=r.record_id "
        "WHERE "
            "pp.song_id=%4 "
        "ORDER BY "
            "p.name "
    )
        .arg(SBID::sb_type_playlist)
        .arg(SBID::sb_type_performer)
        .arg(SBID::sb_type_album)
        .arg(id.sb_song_id)
    ;

    return new SBSqlQueryModel(q);
}

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
SBSqlQueryModel*
DataEntitySong::matchSong(const SBID &newSongID)
{
    qDebug() << SB_DEBUG_INFO;

    QString newSoundex=Common::soundex(newSongID.songTitle);

    //	MatchRank:
    //	0	-	edited value (always one in data set).
    //	1	-	exact match with specified artist (0 or 1 in data set).
    //	2	-	exact match with any other artist (0 or more in data set).
    //	3	-	soundex match with any other artist (0 or more in data set).
    QString q=QString
    (
        "SELECT "
            "0 AS matchRank, "
            "-1 AS song_id, "
            "'%1' AS title, "
            "%3 AS artist_id, "
            "'%2' AS artistName "
        "UNION "
        "SELECT "
            "CASE WHEN a.artist_id=%3 THEN 1 ELSE 2 END AS matchRank, "
            "s.song_id, "
            "s.title, "
            "a.artist_id, "
            "a.name "
        "FROM "
            "___SB_SCHEMA_NAME___performance p "
                "JOIN ___SB_SCHEMA_NAME___song s ON "
                    "p.song_id=s.song_id "
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
                "JOIN ___SB_SCHEMA_NAME___artist a ON "
                    "p.artist_id=a.artist_id "
        "WHERE "
            "p. role_id=0 AND "
            "( "
                "SUBSTR(s.soundex,1,LENGTH('%4'))='%4' OR "
                "SUBSTR('%4',1,LENGTH(s.soundex))=s.soundex "
            ") "
        "ORDER BY "
            "1,3 "

    )
        .arg(Common::escapeSingleQuotes(newSongID.songTitle))
        .arg(Common::escapeSingleQuotes(newSongID.performerName))
        .arg(newSongID.sb_performer_id)
        .arg(newSoundex)
    ;

    return new SBSqlQueryModel(q);
}

///
/// \brief DataEntitySong::matchSongWithinPerformer
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
SBSqlQueryModel*
DataEntitySong::matchSongWithinPerformer(const SBID& newSongID, const QString& newSongTitle)
{
    //	Matches a song by artist
    qDebug() << SB_DEBUG_INFO;

    QString newSoundex=Common::soundex(newSongTitle);

    //	MatchRank:
    //	0	-	edited value (always one in data set).
    //	1	-	exact match with new artist artist (0 or 1 in data set).
    //	2	-	soundex match with new artist (0 or more in data set).
    QString q=QString
    (
        "SELECT "
            "0 AS matchRank, "
            "-1 AS song_id, "
            "'%1' AS title, "
            "%3 AS artist_id, "
            "'%2' AS artistName "
        "UNION "
        "SELECT "
            "1 AS matchRank, "
            "s.song_id, "
            "s.title, "
            "p.artist_id, "
            "a.name "
        "FROM "
            "___SB_SCHEMA_NAME___song s "
                "JOIN ___SB_SCHEMA_NAME___performance p ON "
                    "s.song_id=p.song_id "
                "JOIN ___SB_SCHEMA_NAME___artist a ON "
                    "p.artist_id=a.artist_id AND "
                    "a.artist_id IN (%3) "
        "WHERE "
            "s.song_id!=%4 AND "
            "REPLACE(LOWER(s.title),' ','') = REPLACE(LOWER('%1'),' ','') "
        "UNION "
        "SELECT "
            "2 AS matchRank, "
            "s.song_id, "
            "s.title, "
            "p.artist_id, "
            "a.name "
        "FROM "
            "___SB_SCHEMA_NAME___song s "
                "JOIN ___SB_SCHEMA_NAME___performance p ON "
                    "s.song_id=p.song_id "
                "JOIN ___SB_SCHEMA_NAME___artist a ON "
                    "p.artist_id=a.artist_id AND "
                    "a.artist_id=%3 "
        "WHERE "
            "s.title!='%1' AND "
            "( "
                "SUBSTR(s.soundex,1,LENGTH('%5'))='%5' OR "
                "SUBSTR('%5',1,LENGTH(s.soundex))=s.soundex "
            ") "
        "ORDER BY "
            "1,3 "
    )
        .arg(Common::escapeSingleQuotes(newSongTitle))
        .arg(Common::escapeSingleQuotes(newSongID.performerName))
        .arg(newSongID.sb_performer_id)
        .arg(newSongID.sb_song_id)
        .arg(newSoundex)
    ;

    return new SBSqlQueryModel(q);
}

///
/// \brief DataEntitySong::saveNewSong
/// \param id
/// \return
///
/// Creates new entry in song and performance. Assigns new ID
/// back to parameter. Assumption is that performer already
/// exists.
bool
DataEntitySong::saveNewSong(SBID &id)
{
    bool resultCode=1;

    qDebug() << SB_DEBUG_INFO;
    if(id.sb_song_id==-1)
    {
        DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
        QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
        QString newSoundex=Common::soundex(id.songTitle);
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

        id.sb_song_id=select.value(0).toInt();

        //	Insert new song
        q=QString
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
            .arg(id.sb_song_id)
            .arg(Common::escapeSingleQuotes(id.songTitle))
            .arg(newSoundex)
        ;

        dal->customize(q);
        qDebug() << SB_DEBUG_INFO << q;
        QSqlQuery insertSong(q,db);
        Q_UNUSED(insertSong);

        //	Insert new performance
        q=QString
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
                "%1, "
                "%2, "
                "0, "	//	0: original performer, 1: non-original performer
                "%3, "
                "'%4' "
        )
            .arg(id.sb_song_id)
            .arg(id.sb_performer_id)
            .arg(id.year)
            .arg(Common::escapeSingleQuotes(id.notes))
        ;

        dal->customize(q);
        qDebug() << SB_DEBUG_INFO << q;
        QSqlQuery insertPerformance(q,db);
        Q_UNUSED(insertPerformance);

    }
    return resultCode;
}

bool
DataEntitySong::updateLastPlayDate(const SBID &id)
{
    qDebug() << SB_DEBUG_INFO << id;
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());

    QString q=QString
    (
        "UPDATE "
            "___SB_SCHEMA_NAME___online_performance "
        "SET "
            "last_play_date=%1 "
        "WHERE "
            "song_id=%2 AND "
            "artist_id=%3 AND "
            "record_id=%4 AND "
            "record_position=%5 "
    )
        .arg(dal->getGetDateTime())
        .arg(id.sb_song_id)
        .arg(id.sb_performer_id)
        .arg(id.sb_album_id)
        .arg(id.sb_position)
    ;
    dal->customize(q);

    qDebug() << SB_DEBUG_INFO << q;
    QSqlQuery query(q,db);
    query.exec();

    return 1;	//	CWIP: need proper error handling
}

bool
DataEntitySong::updateExistingSong(const SBID &oldSongID, SBID &newSongID, const QStringList& extraSQL,bool commitFlag)
{
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QStringList allQueries;
    QString q;
    bool resultFlag=1;

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
        << ":sb_song_id=" << oldSongID.sb_song_id
        << ":sb_performer_id=" << oldSongID.sb_performer_id
        << ":isOriginalPerformer=" << oldSongID.isOriginalPerformer
    ;
    qDebug() << SB_DEBUG_INFO << "new"
        << ":sb_song_id=" << newSongID.sb_song_id
        << ":sb_performer_id=" << newSongID.sb_performer_id
        << ":isOriginalPerformer=" << newSongID.isOriginalPerformer
    ;

    //	1.	Set attribute flags
    if(oldSongID.year!=newSongID.year)
    {
        yearOfReleaseChangedFlag=1;
    }
    if(oldSongID.notes!=newSongID.notes)
    {
        notesChangedFlag=1;
    }
    if(oldSongID.lyrics!=newSongID.lyrics)
    {
        lyricsChangedFlag=1;
    }

    //	2.	Determine what need to be done.
    if(newSongID.sb_song_id==-1)
    {
        //	New song does NOT exists
        if(oldSongID.sb_performer_id!=newSongID.sb_performer_id)
        {
            //	Different performer
            mergeToNewSongFlag=1;
            qDebug() << SB_DEBUG_INFO << "mergeToNewSong";
        }
        else
        {
            //	Same performer
            titleRenameFlag=1;
            newSongID.sb_song_id=oldSongID.sb_song_id;
            qDebug() << SB_DEBUG_INFO << "titleRename";
        }
    }
    else
    {
        //	New song exists
        if(oldSongID.sb_song_id!=newSongID.sb_song_id)
        {
            //	Songs are not the same -> merge
            mergeToExistingSongFlag=1;
            qDebug() << SB_DEBUG_INFO << "mergeToExistingSong";
        }
        else if(oldSongID.sb_performer_id!=newSongID.sb_performer_id)
        {
            //	Songs are the same, update performer
            updatePerformerFlag=1;
            qDebug() << SB_DEBUG_INFO << "updatePerformer";
        }
    }

    if(extraSQL.count()>0)
    {
        extraSQLFlag=1;
    }

    qDebug() << SB_DEBUG_INFO << "titleRenameFlag" << titleRenameFlag;
    qDebug() << SB_DEBUG_INFO << "mergeToNewSongFlag" << mergeToNewSongFlag;
    qDebug() << SB_DEBUG_INFO << "mergeToExistingSongFlag" << mergeToExistingSongFlag;
    qDebug() << SB_DEBUG_INFO << "updatePerformerFlag" << updatePerformerFlag;
    qDebug() << SB_DEBUG_INFO << "lyricsChangedFlag" << lyricsChangedFlag;

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
        QMessageBox msgBox;
        msgBox.setText("No flags are set in saveSong");
        msgBox.exec();
        return 0;
    }

    if((int)titleRenameFlag+(int)mergeToNewSongFlag+(int)mergeToExistingSongFlag+(int)updatePerformerFlag>1)
    {
        QMessageBox msgBox;
        msgBox.setText("SaveSong: multiple flags set!");
        msgBox.exec();
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
        qDebug() << SB_DEBUG_INFO;
        allQueries.append(extraSQL);
    }

    //		A.	Attribute changes
    if(lyricsChangedFlag==1)
    {
        qDebug() << SB_DEBUG_INFO << "Update lyrics";

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
            .arg(newSongID.sb_song_id)
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
            .arg(Common::escapeSingleQuotes(newSongID.lyrics))
            .arg(newSongID.sb_song_id)
        ;
        allQueries.append(q);
    }

    if(notesChangedFlag==1)
    {
        qDebug() << SB_DEBUG_INFO << "Update notes";
        q=QString
        (
            "UPDATE ___SB_SCHEMA_NAME___song "
            "SET "
                "notes='%1' "
            "WHERE "
                "song_id=%2 "
        )
            .arg(Common::escapeSingleQuotes(newSongID.notes))
            .arg(newSongID.sb_song_id)
        ;
        allQueries.append(q);
    }

    if(yearOfReleaseChangedFlag==1)
    {
        qDebug() << SB_DEBUG_INFO << "Update year of release";
        q=QString
        (
            "UPDATE ___SB_SCHEMA_NAME___performance "
            "SET "
                "year='%1' "
            "WHERE "
                "song_id=%2 AND "
                "artist_id=%3 "
        )
            .arg(newSongID.year)
            .arg(newSongID.sb_song_id)
            .arg(newSongID.sb_performer_id)
        ;
        allQueries.append(q);
    }

    if(titleRenameFlag==1 || mergeToNewSongFlag==1)
    {
        qDebug() << SB_DEBUG_INFO << "Update song title";
        q=QString
        (
            "UPDATE ___SB_SCHEMA_NAME___song "
            "SET "
                "title='%1', "
                "soundex='%2' "
            "WHERE "
                "song_id=%3 "
        )
            .arg(Common::escapeSingleQuotes(newSongID.songTitle))
            .arg(Common::soundex(newSongID.songTitle))
            .arg(oldSongID.sb_song_id)
        ;
        allQueries.append(q);
        newSongID.sb_song_id=oldSongID.sb_song_id;
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
            .arg(newSongID.sb_song_id)
            .arg(newSongID.sb_performer_id)
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
            .arg(newSongID.sb_song_id)
            .arg(oldSongID.sb_song_id)
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
            .arg(newSongID.sb_performer_id)
            .arg(newSongID.sb_song_id)
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
                .arg(newSongID.sb_song_id)
                .arg(newSongID.sb_performer_id)
                .arg(oldSongID.sb_song_id)
                .arg(oldSongID.sb_performer_id)
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
            .arg(newSongID.sb_song_id)
            .arg(newSongID.sb_performer_id)
            .arg(oldSongID.sb_song_id)
            .arg(oldSongID.sb_performer_id)
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
            .arg(newSongID.sb_song_id)
            .arg(newSongID.sb_performer_id)
            .arg(oldSongID.sb_song_id)
            .arg(oldSongID.sb_performer_id)
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
            .arg(newSongID.sb_song_id)
            .arg(oldSongID.sb_song_id)
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
            .arg(oldSongID.sb_song_id)
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
            .arg(oldSongID.sb_song_id)
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
            .arg(oldSongID.sb_song_id)
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
            .arg(oldSongID.sb_song_id)
            //.arg(oldSongID.sb_performer_id)
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
            .arg(oldSongID.sb_song_id)
        ;
        allQueries.append(q);
    }

    resultFlag=dal->executeBatch(allQueries,commitFlag);
    qDebug() << SB_DEBUG_INFO << resultFlag;

    return resultFlag;
}

void
DataEntitySong::updateSoundexFields()
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
