#include "DataEntityCurrentPlaylist.h"

#include "Common.h"
#include "Controller.h"
#include "Context.h"
#include "DataAccessLayer.h"
#include "SBSqlQueryModel.h"

#include <QMessageBox>
#include <QStringList>

void
DataEntityCurrentPlaylist::clearPlaylist()
{
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());

    QString q="TRUNCATE TABLE ___SB_SCHEMA_NAME___current_playlist ";
    dal->customize(q);

    qDebug() << SB_DEBUG_INFO << q;

    QSqlQuery q2(q,db);
    q2.exec();
}

SBSqlQueryModel*
DataEntityCurrentPlaylist::getAllOnlineSongs()
{
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();

    //	Main query
    QString q=QString
    (
        "SELECT DISTINCT "
            "SB_SONG_ID, "
            "songTitle AS \"song title\", "
            "SB_PERFORMER_ID, "
            "artistName AS \"performer\", "
            "SB_ALBUM_ID, "
            "recordTitle AS \"album title\", "
            "SB_POSITION_ID, "
            "SB_PATH, "
            "duration, "
            "SB_PLAY_ORDER "
        "FROM "
            "( "
                "SELECT "
                    "s.song_id AS SB_SONG_ID, "
                    "s.title AS songTitle, "
                    "a.artist_id AS SB_PERFORMER_ID, "
                    "a.name AS artistName, "
                    "r.record_id AS SB_ALBUM_ID, "
                    "r.title AS recordTitle, "
                    "op.record_position AS SB_POSITION_ID, "
                    "op.path AS SB_PATH, "
                    "rp.duration, "
                    "%1(op.last_play_date,'1/1/1900') AS SB_PLAY_ORDER "
                "FROM "
                    "___SB_SCHEMA_NAME___online_performance op "
                        "JOIN ___SB_SCHEMA_NAME___artist a ON  "
                            "op.artist_id=a.artist_id "
                        "JOIN ___SB_SCHEMA_NAME___record r ON  "
                            "op.record_id=r.record_id "
                        "JOIN ___SB_SCHEMA_NAME___song s ON  "
                            "op.song_id=s.song_id "
                        "JOIN ___SB_SCHEMA_NAME___record_performance rp ON "
                            "op.song_id=rp.song_id AND "
                            "op.artist_id=rp.artist_id AND "
                            "op.record_id=rp.record_id AND "
                            "op.record_position=rp.record_position "
            ") a "
        "ORDER BY "
            "SB_PLAY_ORDER "
        "LIMIT 100 "
    )
            .arg(dal->getIsNull())
    ;

    return new SBSqlQueryModel(q);
}

SBSqlQueryModel*
DataEntityCurrentPlaylist::getAllSongs()
{
    //	Main query
    QString q=QString
    (
        "SELECT DISTINCT "
            "play_position, "
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
            "SB_PATH, "
            "duration "
        "FROM "
            "( "
                "SELECT "
                    "cp.play_position, "
                    "s.song_id AS SB_SONG_ID, "
                    "s.title AS songTitle, "
                    "a.artist_id AS SB_PERFORMER_ID, "
                    "a.name AS artistName, "
                    "r.record_id AS SB_ALBUM_ID, "
                    "r.title AS recordTitle, "
                    "cp.record_position AS SB_POSITION_ID, "
                    "op.path AS SB_PATH, "
                    "rp.duration "
                "FROM "
                    "___SB_SCHEMA_NAME___current_playlist cp "
                        "JOIN ___SB_SCHEMA_NAME___artist a ON  "
                            "cp.artist_id=a.artist_id "
                        "JOIN ___SB_SCHEMA_NAME___record r ON  "
                            "cp.record_id=r.record_id "
                        "JOIN ___SB_SCHEMA_NAME___song s ON  "
                            "cp.song_id=s.song_id "
                        "JOIN ___SB_SCHEMA_NAME___record_performance rp ON "
                            "cp.song_id=rp.song_id AND "
                            "cp.artist_id=rp.artist_id AND "
                            "cp.record_id=rp.record_id AND "
                            "cp.record_position=rp.record_position "
                        "JOIN ___SB_SCHEMA_NAME___online_performance op ON "
                            "cp.song_id=op.song_id AND "
                            "cp.artist_id=op.artist_id AND "
                            "cp.record_id=op.record_id AND "
                            "cp.record_position=op.record_position "
            ") a "
        "ORDER BY "
            "a.play_position "
    ).
        arg(SBID::sb_type_song).
        arg(SBID::sb_type_performer).
        arg(SBID::sb_type_album).
        arg(SBID::sb_type_position)
    ;

    return new SBSqlQueryModel(q);
}

void
DataEntityCurrentPlaylist::populateFromPlaylist(const SBID &playlistID)
{
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());

    QString q=QString
    (
        "INSERT INTO ___SB_SCHEMA_NAME___current_playlist "
        "("
            "current_playlist_id, "
            "play_position, "
            "has_played_flag, "
            "active_flag, "
            "song_id, "
            "artist_id, "
            "record_id, "
            "record_position "
        ") "
        "SELECT "
            "playlist_position, "
            "playlist_position, "
            "'F', "
            "'F', "
            "song_id, "
            "artist_id, "
            "record_id, "
            "record_position "
        "FROM "
            "___SB_SCHEMA_NAME___playlist_performance "
        "WHERE "
            "playlist_id=%1 "
    ).arg(playlistID.sb_playlist_id);
    dal->customize(q);

    qDebug() << SB_DEBUG_INFO << q;

    QSqlQuery q2(q,db);
    q2.exec();
}

void
DataEntityCurrentPlaylist::resetPlaylist()
{
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());

    QString q=QString
    (
        "UPDATE "
            "___SB_SCHEMA_NAME___current_playlist "
        "SET "
            "has_played_flag=0"
    );
    dal->customize(q);

    qDebug() << SB_DEBUG_INFO << q;

    QSqlQuery q2(q,db);
    q2.exec();
}


void
DataEntityCurrentPlaylist::setSongAttributes(int playID,bool activeFlag, bool hasPlayedFlag)
{
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());

    QString q=QString
    (
        "UPDATE "
            "___SB_SCHEMA_NAME___current_playlist "
        "SET "
            "active_flag= %2, "
            "has_played_flag=%3 "
        "WHERE "
            "current_playlist_id=%1 "
    )
        .arg(playID)
        .arg(activeFlag)
        .arg(hasPlayedFlag)
    ;
    dal->customize(q);

    qDebug() << SB_DEBUG_INFO << q;

    QSqlQuery q2(q,db);
    q2.exec();
}
