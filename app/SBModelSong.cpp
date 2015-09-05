#include "Common.h"
#include "Controller.h"
#include "Context.h"
#include "DataAccessLayer.h"
#include "SBSqlQueryModel.h"
#include "SBModelSong.h"

#include <QMessageBox>
#include <QStringList>

SBID
SBModelSong::getDetail(const SBID& id)
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
        .arg(id.sb_item_id)
        .arg(id.sb_performer_id)
    ;
    dal->customize(q);

    QSqlQuery query(q,db);

    result.sb_item_type       =SBID::sb_type_song;

    if(query.next())
    {
        result.sb_performer_id    =query.value(2).toInt();
        result.sb_item_id         =id.sb_item_id;
        result.sb_song_id         =id.sb_item_id;
        result.performerName      =query.value(3).toString();
        result.songTitle          =query.value(0).toString();
        result.year               =query.value(4).toInt();
        result.lyrics             =query.value(5).toString();
        result.notes              =query.value(1).toString();
        result.isOriginalPerformer=query.value(6).toBool();
    }
    else
    {
        result.sb_item_id=-1;	//	no records
    }

    return result;
}

SBSqlQueryModel*
SBModelSong::getAllSongs()
{
    //	Main query
    QString q=QString
    (
        "SELECT  "
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
        "ORDER BY 4 "
    ).arg(SBID::sb_type_song).arg(SBID::sb_type_performer).arg(SBID::sb_type_album).arg(SBID::sb_type_position);

    return new SBSqlQueryModel(q);
}

SBSqlQueryModel*
SBModelSong::getPerformedByListBySong(const SBID& id)
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
        .arg(id.sb_item_id);

    return new SBSqlQueryModel(q);
}

SBSqlQueryModel*
SBModelSong::getOnAlbumListBySong(const SBID& id)
{
    QString q=QString
    (
        "SELECT "
            "%1 AS SB_ITEM_TYPE1, "
            "r.record_id AS SB_RECORD_ID, "
            "r.title AS \"album title\", "
            "r.year AS \"year released\", "
            "%2 AS SB_ITEM_TYPE2, "
            "a.artist_id AS SB_PERFORMER_ID, "
            "a.name AS \"performer\" , "
            "rp.duration \"duration\", "
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
        .arg(id.sb_item_id);

    return new SBSqlQueryModel(q);
}

SBSqlQueryModel*
SBModelSong::getOnChartListBySong(const SBID& id)
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
        .arg(id.sb_item_id);

    return new SBSqlQueryModel(q);
}

SBSqlQueryModel*
SBModelSong::getOnPlaylistListBySong(const SBID& id)
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
        .arg(id.sb_item_id);

    return new SBSqlQueryModel(q);
}


SBSqlQueryModel*
SBModelSong::matchSongByPerformer(const SBID& newSongID, const QString& newSongTitle)
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

bool
SBModelSong::saveSong(const SBID &oldSongID, SBID &newSongID)
{
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QStringList allQueries;
    QString q;

    //	The following flags should be mutually exclusive.
    bool updateSongTitleFlag=0;
    bool updateOriginalPerformerFlag=0;
    bool mergeToNewSongFlag=0;
    bool updateToExistingPerformerFlag=0;

    //	The following flags can be set independently from eachother.
    //	However, they can be turned of by detecting any of the flags above.
    bool yearOfReleaseChangedFlag=0;
    bool notesChangedFlag=0;
    bool lyricsChangedFlag=0;

    qDebug() << SB_DEBUG_INFO << "old"
        << ":sb_item_id=" << oldSongID.sb_item_id
        << ":sb_performer_id=" << oldSongID.sb_performer_id
        << ":isOriginalPerformer=" << oldSongID.isOriginalPerformer
    ;
    qDebug() << SB_DEBUG_INFO << "new"
        << ":sb_item_id=" << newSongID.sb_item_id
        << ":sb_performer_id=" << newSongID.sb_performer_id
        << ":isOriginalPerformer=" << newSongID.isOriginalPerformer
    ;

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

    if(oldSongID.sb_performer_id==newSongID.sb_performer_id)
    {
        //	Update song title
        if(newSongID.sb_item_id==-1)
        {
            updateSongTitleFlag=1;
        }
        else if(oldSongID.sb_item_id!=newSongID.sb_item_id)
        {
            //	Merge
            mergeToNewSongFlag=1;
        }
        //	else: attribute changed.
    }
    else	//	org performer != new performer
    {
        if(oldSongID.sb_item_id==newSongID.sb_item_id &&
            oldSongID.sb_performer_id!=newSongID.sb_performer_id &&
            oldSongID.sb_item_id!=-1)
        {
            //	Performance records exist for both old performer and new performer.
            //	(each with different role_id's). This means that the original
            //	performer is switched.
            updateOriginalPerformerFlag=1;
        }
        else
        {
            if(newSongID.sb_item_id==-1)
            {
                //	performer has changed, title stays the same.
                //	newSongID.sb_item_id==-1 signals that song for new artist does not exist.
                updateToExistingPerformerFlag=1;
                newSongID.sb_item_id=oldSongID.sb_item_id;
            }
            else
            {
                //	Performer has changed. Merge to another existing song.
                mergeToNewSongFlag=1;
            }
        }
    }

    //	Sanity check on flags
    if(
        updateSongTitleFlag==0 &&
        updateOriginalPerformerFlag==0 &&
        mergeToNewSongFlag==0 &&
        updateToExistingPerformerFlag==0 &&
        yearOfReleaseChangedFlag==0 &&
        notesChangedFlag==0 &&
        lyricsChangedFlag==0
    )
    {
        QMessageBox msgBox;
        msgBox.setText("No flags are set in saveSong");
        msgBox.exec();
        return 0;
    }

    qDebug() << SB_DEBUG_INFO << "updateSongTitleFlag" << updateSongTitleFlag;
    qDebug() << SB_DEBUG_INFO << "updateOriginalPerformerFlag" << updateOriginalPerformerFlag;
    qDebug() << SB_DEBUG_INFO << "mergeToNewSongFlag" << mergeToNewSongFlag;
    qDebug() << SB_DEBUG_INFO << "updateToExistingPerformerFlag" << updateToExistingPerformerFlag;

    if((int)updateSongTitleFlag+(int)updateOriginalPerformerFlag+(int)mergeToNewSongFlag+(int)updateToExistingPerformerFlag>1)
    {
        QMessageBox msgBox;
        msgBox.setText("Multiple flags set!");
        msgBox.exec();
        return 0;
    }

    if(mergeToNewSongFlag==1)
    {
        //	Honor year of release of song being merged to.
        yearOfReleaseChangedFlag=0;
        notesChangedFlag=0;
        lyricsChangedFlag=0;
    }


    //	Collect work to be done.
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
            .arg(newSongID.sb_item_id)
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
            .arg(newSongID.lyrics)
            .arg(newSongID.sb_item_id)
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
            .arg(newSongID.notes)
            .arg(newSongID.sb_item_id)
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
            .arg(newSongID.sb_item_id)
            .arg(newSongID.sb_performer_id)
        ;
        allQueries.append(q);
    }

    if(updateSongTitleFlag==1)
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
            .arg(oldSongID.sb_item_id)
        ;
        allQueries.append(q);

        //	Reassign org sb_item_id to newSongID as only title change has happened
        newSongID.sb_item_id=oldSongID.sb_item_id;
    }

    if(updateOriginalPerformerFlag==1)
    {
        qDebug() << SB_DEBUG_INFO << "Update original performer";

        q=QString
        (
            "UPDATE "
                "___SB_SCHEMA_NAME___performance "
            "SET "
                "role_id=CASE WHEN artist_id=%2 THEN 0 ELSE 1 END "
            "WHERE "
                "artist_id IN (%1,%2) AND "
                "song_id=%3 "
        )
            .arg(oldSongID.sb_performer_id)
            .arg(newSongID.sb_performer_id)
            .arg(newSongID.sb_item_id)
        ;
        allQueries.append(q);
    }

    if(mergeToNewSongFlag==1 || updateToExistingPerformerFlag==1)
    {
        qDebug() << SB_DEBUG_INFO << "Merge to new song";
        //	Merge old with new.

        //	A.	Update performance tables
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
                    "artist_id=%3 "
                "WHERE "
                    "song_id=%4 AND "
                    "artist_id=%5 "
             )
                .arg(performanceTable.at(i))
                .arg(newSongID.sb_item_id)
                .arg(newSongID.sb_performer_id)
                .arg(oldSongID.sb_item_id)
                .arg(oldSongID.sb_performer_id)
            ;
            allQueries.append(q);
        }
    }

    if(updateToExistingPerformerFlag==1)
    {
        //	Create a new performance
        q=QString
        (
            "INSERT INTO ___SB_SCHEMA_NAME___performance "
            "( song_id, artist_id, role_id, year, notes ) "
            "VALUES ( %1,%2,0,%3,'') "
        )
            .arg(newSongID.sb_item_id)
            .arg(newSongID.sb_performer_id)
            .arg(newSongID.year)
        ;
        allQueries.append(q);
    }

    if(mergeToNewSongFlag==1 || updateToExistingPerformerFlag==1)
    {
        //	A.	Update record_performance for non-op_ fields
        q=QString
        (
            "UPDATE "
                "___SB_SCHEMA_NAME___record_performance "
            "SET     "
                "song_id=%1, "
                "artist_id=%2 "
            "WHERE "
                "song_id=%3 AND "
                "artist_id=%4 "
         )
            .arg(newSongID.sb_item_id)
            .arg(newSongID.sb_performer_id)
            .arg(oldSongID.sb_item_id)
            .arg(oldSongID.sb_performer_id)
        ;
        allQueries.append(q);

        //	B.	Update record_performance for op_ fields
        q=QString
        (
            "UPDATE "
                "___SB_SCHEMA_NAME___record_performance "
            "SET     "
                "op_song_id=%1, "
                "op_artist_id=%2 "
            "WHERE "
                "op_song_id=%3 AND "
                "op_artist_id=%4 "
         )
            .arg(newSongID.sb_item_id)
            .arg(newSongID.sb_performer_id)
            .arg(oldSongID.sb_item_id)
            .arg(oldSongID.sb_performer_id)
        ;
        allQueries.append(q);
    }

    if(mergeToNewSongFlag==1)
    {
        //	1.	Remove lyrics
        q=QString
        (
            "DELETE FROM "
                "___SB_SCHEMA_NAME___lyrics "
            "WHERE "
                "song_id=%1 "
        )
            .arg(oldSongID.sb_item_id)
            .arg(oldSongID.sb_performer_id)
        ;
        allQueries.append(q);

        //	2.	Remove online_performance
        q=QString
        (
            "DELETE FROM "
                "___SB_SCHEMA_NAME___online_performance "
            "WHERE "
                "song_id=%1 AND "
                "artist_id=%2 "
        )
            .arg(oldSongID.sb_item_id)
            .arg(oldSongID.sb_performer_id)
        ;
        allQueries.append(q);
    }

    if(mergeToNewSongFlag==1 || updateToExistingPerformerFlag==1)
    {
        //	1.	Remove original performance
        q=QString
        (
            "DELETE FROM "
                "___SB_SCHEMA_NAME___performance "
            "WHERE "
                "song_id=%1 AND "
                "artist_id=%2 "
        )
            .arg(oldSongID.sb_item_id)
            .arg(oldSongID.sb_performer_id)
        ;
        allQueries.append(q);
    }

    if(mergeToNewSongFlag==1)
    {
        //	1.	Remove original song
        q=QString
        (
            "DELETE FROM ___SB_SCHEMA_NAME___song "
            "WHERE song_id=%1 "
        )
            .arg(oldSongID.sb_item_id)
        ;
        allQueries.append(q);
    }
    qDebug() << SB_DEBUG_INFO << "End";

    return dal->executeBatch(allQueries);
}

void
SBModelSong::updateSoundexFields()
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
