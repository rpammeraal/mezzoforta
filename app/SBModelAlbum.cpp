#include <QMessageBox>

#include "Context.h"
#include "DataAccessLayer.h"
#include "SBModelAlbum.h"
#include "SBSqlQueryModel.h"

QStringList
SBModelAlbum::addSongToAlbum(const SBID &album, const SBID &song)
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
                "comments "
            ") "
            "SELECT "
                "song_id, "
                "%1, "	//	artist_id
                "1, "	//	role_id,
                "year, "
                "'' "
            "FROM "
                "___SB_SCHEMA_NAME___song s "
            "WHERE"
                "song_id=%2 AND "
                "NOT EXISTS "
                "( "
                    "SELECT "
                        "1"
                    "FROM "
                        "___SB_SCHEMA_NAME___performance "
                    "WHERE "
                        "song_id=%2 AND "
                        "artist_id=%1 "
                ") "
        )
            .arg(song.sb_performer_id)
            .arg(song.sb_song_id)
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
                "op_song_id, "
                "op_artist_id, "
                "op_record_id, "
                "op_record_position, "
                "duration "
            ") "

            "VALUES "
            "( "
                "%1, "
                "%2, "
                "%3, "
                "%4, "
                "%1, "
                "%2, "
                "%3, "
                "%4, "
                "'00:00:00' "
             ") "
        )
            .arg(song.sb_song_id)
            .arg(song.sb_performer_id)
            .arg(song.sb_album_id)
            .arg(song.sb_position)
    );
    return SQL;
}

SBID
SBModelAlbum::getDetail(const SBID& id)
{
    SBID result=id;
    qDebug() << SB_DEBUG_INFO << "id.wiki=" << id.wiki;

    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());

    QString q=QString
    (
        "SELECT DISTINCT "
            "r.record_id, "
            "r.title , "
            "r.genre, "
            "r.year, "
            "r.notes, "
            "a.name, "
            "a.artist_id, "
            "a.mbid "
        "FROM "
            "___SB_SCHEMA_NAME___record r "
                "JOIN ___SB_SCHEMA_NAME___artist a ON "
                    "a.artist_id=r.artist_id "
        "WHERE "
            "r.record_id=%1 OR "
            "( "
                "r.title='%2' AND "
                "a.name='%3' "
            ") "
    )
        .arg(id.sb_item_id)
        .arg(Common::escapeSingleQuotes(id.albumTitle))
        .arg(Common::escapeSingleQuotes(id.performerName))
    ;
    dal->customize(q);

    qDebug() << SB_DEBUG_INFO << q;

    QSqlQuery query(q,db);

    result.sb_item_type   =SBID::sb_type_album;

    if(query.next())
    {
        qDebug() << SB_DEBUG_INFO;
        result.sb_item_id     =query.value(0).toInt();
        result.sb_album_id    =query.value(0).toInt();
        result.performerName  =query.value(5).toString();
        result.albumTitle     =query.value(1).toString();
        result.year           =query.value(3).toInt();
        result.genre          =query.value(2).toString();
        result.notes          =query.value(4).toString();
        result.sb_performer_id=query.value(6).toInt();
        result.sb_mbid        =query.value(7).toString();
    }
    else
    {
        qDebug() << SB_DEBUG_INFO;
        result.sb_item_id=-1;
    }

    qDebug() << SB_DEBUG_INFO << result.sb_item_id;
    qDebug() << SB_DEBUG_INFO << "result.wiki=" << result.wiki;
    return result;
}


SBSqlQueryModel*
SBModelAlbum::getAllSongs(const SBID& id)
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
            "rp.record_position "
        "FROM "
            "___SB_SCHEMA_NAME___record_performance rp "
                "join ___SB_SCHEMA_NAME___song s on "
                    "rp.song_id=s.song_id "
                "join ___SB_SCHEMA_NAME___artist a on "
                    "rp.artist_id=a.artist_id "
        "WHERE "
            "record_id=%3 "
        "ORDER BY 2"
    )
        .arg(SBID::sb_type_song)
        .arg(SBID::sb_type_album)
        .arg(id.sb_item_id)
        .arg(SBID::sb_type_performer)
        .arg(SBID::sb_type_position)
    ;

    return new SBSqlQueryModel(q);
}

SBSqlQueryModel*
SBModelAlbum::getAllSongsOLD(const SBID& id)
{
    QString q=QString
    (
        "SELECT "
            "%1 AS SB_ALBUM_ID, "
            "rp.record_position AS \"#\", "
            "%2 AS SB_ITEM_TYPE1, "
            "s.song_id AS SB_SONG_ID , "
            "s.title AS \"song\", "
            "rp.duration AS \"duration\", "
            "%3 AS SB_ITEM_TYPE2, "
            "a.artist_id AS SB_PERFORMER_ID, "
            "a.name AS \"performer\" "
        "FROM "
            "___SB_SCHEMA_NAME___record_performance rp "
                "join ___SB_SCHEMA_NAME___song s on "
                    "rp.song_id=s.song_id "
                "join ___SB_SCHEMA_NAME___artist a on "
                    "rp.artist_id=a.artist_id "
        "WHERE "
            "record_id=%1 "
        "ORDER BY 2"
    )
        .arg(id.sb_item_id)
        .arg(SBID::sb_type_song)
        .arg(SBID::sb_type_performer);

    return new SBSqlQueryModel(q);
}

SBSqlQueryModel*
SBModelAlbum::matchAlbum(const SBID &newAlbum)
{
    qDebug() << SB_DEBUG_INFO;

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
            "p.record_id!=%4"
    )
        .arg(Common::escapeSingleQuotes(newAlbum.albumTitle))
        .arg(Common::escapeSingleQuotes(newAlbum.performerName))
        .arg(newAlbum.sb_performer_id)
        .arg(newAlbum.sb_item_id)
    ;

    return new SBSqlQueryModel(q);

}

QStringList
SBModelAlbum::mergeAlbum(const SBID& from, const SBID& to)
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
        .arg(to.sb_item_id)
    ;
    dal->customize(q);

    qDebug() << SB_DEBUG_INFO << q;

    QSqlQuery query(q,db);

    if(query.next())
    {
        maxPosition=query.value(0).toInt();
        maxPosition++;
    }
    qDebug() << SB_DEBUG_INFO << maxPosition;

    //	Update rock_performance.non_op fields
    SQL.append
    (
        QString
        (
            "UPDATE "
                "___SB_SCHEMA_NAME___record_performance "
            "SET "
                "op_record=%1, "
                "op_record_position=op_record_position+%2 "
            "WHERE "
                "op_record_id=%3 "
        )
            .arg(to.sb_album_id)
            .arg(maxPosition)
            .arg(from.sb_album_id)
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
            .arg(to.sb_album_id)
            .arg(maxPosition)
            .arg(from.sb_album_id)
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
            .arg(to.sb_album_id)
            .arg(maxPosition)
            .arg(from.sb_album_id)
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
            .arg(to.sb_album_id)
            .arg(maxPosition)
            .arg(from.sb_album_id)
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
            .arg(to.sb_album_id)
            .arg(maxPosition)
            .arg(from.sb_album_id)
    );

    return SQL;
}

QStringList
SBModelAlbum::mergeSongInAlbum(const SBID& album, int newPosition, const SBID& song)
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
            .arg(album.sb_album_id)
            .arg(song.sb_position)
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
            .arg(album.sb_album_id)
            .arg(song.sb_position)
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
            .arg(album.sb_album_id)
            .arg(song.sb_position)
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
            .arg(album.sb_album_id)
            .arg(song.sb_position)
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
            .arg(album.sb_album_id)
            .arg(song.sb_position)
    );


    return SQL;
}

QStringList
SBModelAlbum::removeAlbum(const SBID& album)
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
            .arg(album.sb_album_id)
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
            .arg(album.sb_album_id)
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
            .arg(album.sb_album_id)
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
            .arg(album.sb_album_id)
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
            .arg(album.sb_album_id)
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
            .arg(album.sb_album_id)
    );

    return SQL;
}

QStringList
SBModelAlbum::removeSongFromAlbum(const SBID& album, int position)
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
            .arg(album.sb_album_id)
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
            .arg(album.sb_album_id)
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
            .arg(album.sb_album_id)
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
            .arg(album.sb_album_id)
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
            .arg(album.sb_album_id)
            .arg(position)
    );

    return SQL;
}

QStringList
SBModelAlbum::repositionSongOnAlbum(int albumID, int fromPosition, int toPosition)
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
SBModelAlbum::updateExistingAlbum(const SBID& orgAlbum, const SBID& newAlbum, const QStringList &extraSQL,bool commitFlag)
{
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();

    QStringList allQueries;
    QString q;
    bool resultFlag=1;

    //	artist
    if(orgAlbum.sb_performer_id!=newAlbum.sb_performer_id)
    {
        q=QString
        (
            "UPDATE ___SB_SCHEMA_NAME___record "
            "SET "
                "artist_id=%1 "
            "WHERE "
                "record_id=%2 "
        )
            .arg(newAlbum.sb_performer_id)
            .arg(newAlbum.sb_album_id)
        ;
        allQueries.append(q);
    }

    //	title
    if(orgAlbum.albumTitle!=newAlbum.albumTitle)
    {
        q=QString
        (
            "UPDATE ___SB_SCHEMA_NAME___record "
            "SET "
                "title='%1' "
            "WHERE "
                "record_id=%2 "
        )
            .arg(Common::escapeSingleQuotes(newAlbum.albumTitle))
            .arg(newAlbum.sb_album_id)
        ;
        allQueries.append(q);
    }

    //	year
    if(orgAlbum.year!=newAlbum.year)
    {
        q=QString
        (
            "UPDATE ___SB_SCHEMA_NAME___record "
            "SET "
                "year=%1 "
            "WHERE "
                "record_id=%2 "
        )
            .arg(newAlbum.year)
            .arg(newAlbum.sb_album_id)
        ;
        allQueries.append(q);
    }

    allQueries.append(extraSQL);

    resultFlag=dal->executeBatch(allQueries,commitFlag);

    return resultFlag;
}
