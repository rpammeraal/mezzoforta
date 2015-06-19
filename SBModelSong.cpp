#include "Common.h"
#include "Controller.h"
#include "Context.h"
#include "DataAccessLayer.h"
#include "SBModelList.h"
#include "SBModelSong.h"



SBID
SBModelSong::getDetail(const SBID& id)
{
    SBID result=id;
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());

    QString q=QString
    (
        "SELECT "
            "s.title, "
            "s.notes, "
            "a.artist_id, "
            "a.name, "
            "p.year, "
            "l.lyrics "
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
             "s.song_id=%1"
    ).arg(id.sb_item_id);
    dal->customize(q);

    QSqlQuery query(q,db);
    query.next();

    result.sb_performer_id1=query.value(2).toInt();
    result.sb_item_id      =id.sb_item_id;
    result.sb_song_id1     =id.sb_item_id;
    result.sb_item_type    =SBID::sb_type_song;
    result.performerName   =query.value(3).toString();
    result.songTitle       =query.value(0).toString();
    result.year            =query.value(4).toInt();
    result.lyrics          =query.value(5).toString();
    result.notes           =query.value(1).toString();

    return result;
}

SBModelList*
SBModelSong::getAllSongs()
{
    //	Main query
    QString q=
        "SELECT  "
            "SB_KEYWORDS, "
            "SB_SONG_ID, "
            "songTitle AS \"song title\", "
            "SB_PERFORMER_ID, "
            "artistName AS \"performer\", "
            "SB_ALBUM_ID, "
            "recordTitle AS \"album title\", "
            "SB_ALBUM_POSITION_ID "
        "FROM "
            "( "
                "SELECT "
                    "s.song_id AS SB_SONG_ID, "
                    "s.title AS songTitle, "
                    "a.artist_id AS SB_PERFORMER_ID, "
                    "a.name AS artistName, "
                    "r.record_id AS SB_ALBUM_ID, "
                    "r.title AS recordTitle, "
                    "rp.record_position AS SB_ALBUM_POSITION_ID, "
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
        "ORDER BY 1 ";

    return new SBModelList(q);
}

SBModelList*
SBModelSong::getPerformedByListBySong(const SBID& id)
{
    QString q=QString
    (
        "SELECT "
            "a.artist_id AS SB_PERFORMER_ID, "
            "a.name AS \"performer\", "
            "p.year AS \"year released \" "
        "FROM "
            "___SB_SCHEMA_NAME___performance p "
                "JOIN ___SB_SCHEMA_NAME___artist a ON "
                    "a.artist_id=p.artist_id "
        "WHERE "
            "p.role_id=1 AND "
            "p.song_id=%1 "
        "ORDER BY "
            "a.name"
    ).arg(id.sb_item_id);

    return new SBModelList(q);
}

SBModelList*
SBModelSong::getOnAlbumListBySong(const SBID& id)
{
    QString q=QString
    (
        "SELECT "
            "r.record_id AS SB_ALBUM_ID, "
            "r.title AS \"album title\", "
            "r.year AS \"year released\", "
            "a.artist_id AS SB_PERFORMER_ID, "
            "a.name AS \"performer\" , "
            "rp.duration \"duration\" "
        "FROM "
            "___SB_SCHEMA_NAME___record_performance rp "
                "JOIN ___SB_SCHEMA_NAME___record r ON "
                    "rp.record_id=r.record_id "
                "JOIN ___SB_SCHEMA_NAME___artist a ON "
                    "rp.artist_id=a.artist_id "
        "WHERE "
            "rp.song_id=%1"
    ).arg(id.sb_item_id);

    return new SBModelList(q);
}

SBModelList*
SBModelSong::getOnChartListBySong(const SBID& id)
{
    QString q=QString
    (
        "SELECT "
            "cp.chart_position AS \"position\", "
            "c.chart_id AS SB_CHART_ID, "
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
                "cp.song_id=%1"
    ).arg(id.sb_item_id);

    return new SBModelList(q);
}

SBModelList*
SBModelSong::getOnPlaylistListBySong(const SBID& id)
{
    QString q=QString
    (
        "SELECT DISTINCT "
            "p.playlist_id AS SB_PLAYLIST_ID, "
            "p.name AS \"playlist\", "
            "a.artist_id AS SB_PERFORMER_ID, "
            "a.name AS \"performer\", "
            "rp.duration AS \"duration\" "
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
        "WHERE "
            "pp.song_id=%1"
    ).arg(id.sb_item_id);

    return new SBModelList(q);
}
