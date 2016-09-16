#include <QMessageBox>

#include "Context.h"
#include "DataAccessLayer.h"
#include "DataEntityAlbum.h"
#include "SBSqlQueryModel.h"

//	CWIP: move to SBIDAlbum:
//	either add to a list of songs in SBIDAlbum (preferred)
//	or as saveSongToAlbum().
QStringList
DataEntityAlbum::addSongToAlbum(const SBIDSong &song)
{
    QStringList SQL;

    //	Insert performance if not exists
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
                "song_id, "
                "%1, "	//	artist_id
                "1, "	//	role_id,
                "%3, "
                "'' "
            "FROM "
                "___SB_SCHEMA_NAME___song s "
            "WHERE "
                "song_id=%2 AND "
                "NOT EXISTS "
                "( "
                    "SELECT "
                        "1 "
                    "FROM "
                        "___SB_SCHEMA_NAME___performance "
                    "WHERE "
                        "song_id=%2 AND "
                        "artist_id=%1 "
                ") "
        )
            .arg(song.songPerformerID())
            .arg(song.songID())
            .arg(song.year())
    );

    //	Insert record performance
    SQL.append
    (
        QString
        (
            "INSERT INTO ___SB_SCHEMA_NAME___record_performance "
            "( "
                "song_id, "
                "artist_id, "
                "record_id, "
                "record_position, "
//                "op_song_id, "
//                "op_artist_id, "
//                "op_record_id, "
//                "op_record_position, "
                "duration "
            ") "

            "VALUES "
            "( "
                "%1, "
                "%2, "
                "%3, "
                "%4, "
//                "%1, "
//                "%2, "
//                "%3, "
//                "%4, "
                "'00:00:00' "
             ") "
        )
            .arg(song.songID())
            .arg(song.songPerformerID())
            .arg(song.albumID())
            .arg(song.albumPosition())
    );
    return SQL;
}

//	CWIP: move to SBIDAlbum
SBIDAlbum
DataEntityAlbum::getDetail(const SBIDBase& id)
{
    SBIDAlbum result=id;	//	CWIP: this should *NOT* be done. Assign result with query results *ONLY*
    result.getDetail(0);
    return result;
}

SBSqlQueryModel*
DataEntityAlbum::getAllSongs(const SBIDBase& id)
{
    QString q=QString
    (
        "SELECT "
            "%1 AS SB_MAIN_ITEM, "
            "rp.record_position AS \"#\", "
            "%2 AS SB_ITEM_TYPE1, "
            "%3 AS SB_ALBUM_ID, "
            "%1 AS SB_ITEM_TYPE2, "
            "s.song_id AS SB_SONG_ID , "
            "s.title AS \"song\", "
            "rp.duration AS \"duration\", "
            "%4 AS SB_ITEM_TYPE3, "
            "a.artist_id AS SB_PERFORMER_ID, "
            "a.name AS \"performer\", "
            "%5 AS SB_POSITION, "
            "rp.record_position AS SB_ALBUM_POSITION, "
            "op.path AS SB_PATH, "
            "r.title AS album_title "
        "FROM "
            "___SB_SCHEMA_NAME___record_performance rp "
                "JOIN ___SB_SCHEMA_NAME___song s ON "
                    "rp.song_id=s.song_id "
                "JOIN ___SB_SCHEMA_NAME___artist a ON "
                    "rp.artist_id=a.artist_id "
                "JOIN ___SB_SCHEMA_NAME___online_performance op ON "
                    "rp.op_song_id=op.song_id AND "
                    "rp.op_artist_id=op.artist_id AND "
                    "rp.op_record_id=op.record_id AND "
                    "rp.op_record_position=op.record_position "
                "JOIN ___SB_SCHEMA_NAME___record r ON "
                    "rp.record_id=r.record_id "
        "WHERE "
            "rp.record_id=%3 "
        "ORDER BY 2"
    )
        .arg(Common::sb_field_song_id)
        .arg(Common::sb_field_album_id)
        .arg(id.albumID())
        .arg(Common::sb_field_performer_id)
        .arg(Common::sb_field_album_position)
    ;

    return new SBSqlQueryModel(q);
}

SBSqlQueryModel*
DataEntityAlbum::matchAlbum(const SBIDBase &newAlbum)
{
    //	MatchRank:
    //	0	-	edited value (always one in data set).
    //	1	-	exact match with specified artist (0 or 1 in data set).
    //	2	-	exact match with any other artist (0 or more in data set).
    QString q=QString
    (
        "SELECT "
            "0 AS matchRank, "
            "-1 AS album_id, "
            "'%1' AS title, "
            "%3 AS artist_id, "
            "'%2' AS artistName "
        "UNION "
        "SELECT "
            "CASE WHEN a.artist_id=%3 THEN 1 ELSE 2 END AS matchRank, "
            "p.record_id, "
            "p.title, "
            "a.artist_id, "
            "a.name "
        "FROM "
            "___SB_SCHEMA_NAME___record p "
                "JOIN ___SB_SCHEMA_NAME___artist a ON "
                    "p.artist_id=a.artist_id "
        "WHERE "
            "REPLACE(LOWER(p.title),' ','') = REPLACE(LOWER('%1'),' ','') AND "
            "p.record_id!=%4 "
        "ORDER BY "
            "1 "
    )
        .arg(Common::escapeSingleQuotes(newAlbum.albumTitle()))
        .arg(Common::escapeSingleQuotes(newAlbum.albumPerformerName()))
        .arg(newAlbum.albumPerformerID())
        .arg(newAlbum.albumID())
    ;

    return new SBSqlQueryModel(q);

}

QStringList
DataEntityAlbum::mergeAlbum(const SBIDBase& from, const SBIDBase& to)
{
    QStringList SQL;
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
            .arg(from.albumID())
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
            .arg(from.albumID())
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
            .arg(from.albumID())
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
            .arg(from.albumID())
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
            .arg(from.albumID())
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
            .arg(from.albumID())
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
            .arg(from.albumID())
    );

    return SQL;
}

QStringList
DataEntityAlbum::mergeSongInAlbum(const SBIDBase& album, int newPosition, const SBIDBase& song)
{
    QStringList SQL;

    //	Move any performances from merged song to alt table
    SQL.append
    (
        QString
        (
            "INSERT INTO ___SB_SCHEMA_NAME___online_performance_alt "
            "SELECT "
                "song_id, "
                "artist_id, "
                "record_id, "
                "%1, "
                "format_id, "
                "path, "
                "source_id, "
                "last_play_date, "
                "play_order, "
                "insert_order "
            "FROM "
                "___SB_SCHEMA_NAME___online_performance op "
            "WHERE"
                "record_id=%2 AND "
                "record_position=%3 "
        )
            .arg(newPosition)
            .arg(album.albumID())
            .arg(song.albumPosition())
    );

    //	Delete performance from op table
    SQL.append
    (
        QString
        (
            "DELETE ___SB_SCHEMA_NAME___online_performance op "
            "WHERE"
                "record_id=%1 AND "
                "record_position=%2 "
        )
            .arg(album.albumID())
            .arg(song.albumPosition())
    );

    //	Update toplay
    SQL.append
    (
        QString
        (
            "UPDATE "
                "___SB_SCHEMA_NAME___toplay "
            "SET "
                "record_position=%1 "
            "WHERE "
                "record_id=%2 AND "
                "record_position=%3 "
        )
            .arg(newPosition)
            .arg(album.albumID())
            .arg(song.albumPosition())
    );

    //	Update playlist_performance
    SQL.append
    (
        QString
        (
            "UPDATE "
                "___SB_SCHEMA_NAME___playlist_performance "
            "SET "
                "record_position=%1 "
            "WHERE "
                "record_id=%2 AND "
                "record_position=%3 "
        )
            .arg(newPosition)
            .arg(album.albumID())
            .arg(song.albumPosition())
    );

    //	Delete song from record_performance
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
            .arg(album.albumID())
            .arg(song.albumPosition())
    );


    return SQL;
}

QStringList
DataEntityAlbum::removeAlbum(const SBIDBase& album)
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
            .arg(album.albumID())
    );

    SQL.append
    (
        QString
        (
            "UPDATE "
                "___SB_SCHEMA_NAME___record_performance "
            "SET "
                "op_song_id=NULL, "
                "op_artist_id=NULL, "
                "op_record_id=NULL, "
                "op_record_position=NULL "
            "WHERE "
                "record_id=%1 "
        )
            .arg(album.albumID())
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
            .arg(album.albumID())
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
            .arg(album.albumID())
    );

    SQL.append
    (
        QString
        (
            "DELETE FROM "
                "___SB_SCHEMA_NAME___playlist_performance "
            "WHERE "
                "record_id=%1 "
        )
            .arg(album.albumID())
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
            .arg(album.albumID())
    );

    return SQL;
}

QStringList
DataEntityAlbum::removeSongFromAlbum(const SBIDBase& album, int position)
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
            .arg(album.albumID())
            .arg(position)
    );

    SQL.append
    (
        QString
        (
            "UPDATE "
                "___SB_SCHEMA_NAME___record_performance "
            "SET "
                "op_song_id=NULL, "
                "op_artist_id=NULL, "
                "op_record_id=NULL, "
                "op_record_position=NULL "
            "WHERE "
                "record_id=%1 AND "
                "record_position=%2 "
        )
            .arg(album.albumID())
            .arg(position)
    );

    SQL.append
    (
        QString
        (
            "DELETE FROM "
                "___SB_SCHEMA_NAME___online_performance "
            "WHERE "
                "record_id=%1 AND "
                "record_position=%2 "
        )
            .arg(album.albumID())
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
            .arg(album.albumID())
            .arg(position)
    );

    SQL.append
    (
        QString
        (
            "DELETE FROM "
                "___SB_SCHEMA_NAME___playlist_performance "
            "WHERE "
                "record_id=%1 AND "
                "record_position=%2 "
        )
            .arg(album.albumID())
            .arg(position)
    );

    return SQL;
}

QStringList
DataEntityAlbum::repositionSongOnAlbum(int albumID, int fromPosition, int toPosition)
{
    QStringList SQL;

    //	Update rock_performance.non_op fields
    SQL.append
    (
        QString
        (
            "UPDATE "
                "___SB_SCHEMA_NAME___record_performance "
            "SET "
                "op_record_position=%1 "
            "WHERE "
                "op_record_id=%2 AND "
                "op_record_position=%3 "
        )
            .arg(toPosition)
            .arg(albumID)
            .arg(fromPosition)
    );

    //	Update rock_performance.op fields
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
            .arg(albumID)
            .arg(fromPosition)
    );

    //	Update online_performance
    SQL.append
    (
        QString
        (
            "UPDATE "
                "___SB_SCHEMA_NAME___online_performance "
            "SET "
                "record_position=%1 "
            "WHERE "
                "record_id=%2 AND "
                "record_position=%3 "
        )
            .arg(toPosition)
            .arg(albumID)
            .arg(fromPosition)
    );

    //	Update toplay
    SQL.append
    (
        QString
        (
            "UPDATE "
                "___SB_SCHEMA_NAME___toplay "
            "SET "
                "record_position=%1 "
            "WHERE "
                "record_id=%2 AND "
                "record_position=%3 "
        )
            .arg(toPosition)
            .arg(albumID)
            .arg(fromPosition)
    );

    //	Update playlist_performance
    SQL.append
    (
        QString
        (
            "UPDATE "
                "___SB_SCHEMA_NAME___playlist_performance "
            "SET "
                "record_position=%1 "
            "WHERE "
                "record_id=%2 AND "
                "record_position=%3 "
        )
            .arg(toPosition)
            .arg(albumID)
            .arg(fromPosition)
    );

    return SQL;
}

bool
DataEntityAlbum::updateExistingAlbum(const SBIDBase& orgAlbum, const SBIDBase& newAlbum, const QStringList &extraSQL,bool commitFlag)
{
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();

    QStringList allQueries;
    QString q;
    bool resultFlag=1;

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

    return resultFlag;
}

QStringList
DataEntityAlbum::updateSongOnAlbum(int albumID, const SBIDSong &song)
{
    qDebug() << SB_DEBUG_INFO
             << song.songID()
             << song.songPerformerID()
             << song.albumID()
             << song.albumPosition()
             << song.notes()
    ;
    QStringList SQL;

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
            .arg(albumID)
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
            .arg(albumID)
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
            .arg(albumID)
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
            .arg(albumID)
            .arg(song.albumPosition())
    );
    return SQL;
}
