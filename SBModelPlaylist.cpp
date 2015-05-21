#include "Common.h"
#include "Context.h"
#include "Controller.h"
#include "DataAccessLayer.h"
#include "SBModelPlaylist.h"
#include "SBModelSonglist.h"

//	NEW
SBID
SBModelPlaylist::getDetail(const SBID& id)
{
    SBID result=id;
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());

    QString q=QString
    (
        "SELECT "
            "p.name, "
            "p.duration, "
            "count(*) "
        "FROM "
            "___SB_SCHEMA_NAME___playlist p "
                "LEFT JOIN ___SB_SCHEMA_NAME___playlist_performance pp on "
                    "p.playlist_id=pp.playlist_id "
                "LEFT JOIN ___SB_SCHEMA_NAME___playlist_composite pc on "
                    "p.playlist_id=pc.playlist_id "
        "WHERE "
            "p.playlist_id=%1 "
        "GROUP BY "
            "p.name, "
            "p.duration"
    ).arg(id.sb_item_id);
    dal->customize(q);

    qDebug() << SB_DEBUG_INFO << q;

    QSqlQuery query(q,db);
    query.next();

    result.sb_item_type   =SBID::sb_type_playlist;
    result.sb_item_id     =id.sb_item_id;
    result.sb_playlist_id1=id.sb_item_id;
    result.playlistName   =query.value(0).toString();
    result.duration       =query.value(1).toString();
    result.count1         =query.value(2).toInt();

    return result;
}

SBModelSonglist*
SBModelPlaylist::getAllItemsByPlaylist(const SBID& id)
{
    //	Main query
    QString q=QString
    (
        "SELECT "
            "pc.playlist_position as \"#\", "
            "CASE "
                "WHEN pc.playlist_playlist_id IS NOT NULL THEN 'SB_PLAYLIST_TYPE' "
                "WHEN pc.playlist_chart_id    IS NOT NULL THEN 'SB_CHART_TYPE' "
                "WHEN pc.playlist_record_id   IS NOT NULL THEN 'SB_ALBUM_TYPE' "
                "WHEN pc.playlist_artist_id   IS NOT NULL THEN 'SB_PERFORMER_TYPE' "
            "END AS SB_TYPE_ID, "
            "COALESCE(pc.playlist_playlist_id,pc.playlist_chart_id,pc.playlist_record_id,pc.playlist_artist_id) AS SB_ITEM_ID, "
            "CASE "
                "WHEN pc.playlist_playlist_id IS NOT NULL THEN 'playlist' "
                "WHEN pc.playlist_chart_id    IS NOT NULL THEN 'chart' "
                "WHEN pc.playlist_record_id   IS NOT NULL THEN 'album' "
                "WHEN pc.playlist_artist_id   IS NOT NULL THEN 'artist' "
            "END || ': ' || "
            "COALESCE(p.name,c.name,r.title,a.name) || "
            "CASE "
                "WHEN pc.playlist_record_id   IS NOT NULL THEN ' - ' || ra.name "
                "WHEN pc.playlist_artist_id   IS NOT NULL THEN ' - ' || a.name "
                "ELSE '' "
            "END  as item "
            //"COALESCE(ra.artist_id,a.artist_id) AS SB_ARTIST_ID, "
            //"COALESCE(ra.name,a.name) as \"performer\" "
        "FROM "
            "___SB_SCHEMA_NAME___playlist_composite pc "
                "LEFT JOIN ___SB_SCHEMA_NAME___playlist p ON "
                    "pc.playlist_playlist_id=p.playlist_id "
                "LEFT JOIN ___SB_SCHEMA_NAME___chart c ON "
                    "pc.playlist_chart_id=c.chart_id "
                "LEFT JOIN ___SB_SCHEMA_NAME___record r ON "
                    "pc.playlist_record_id=r.record_id "
                "LEFT JOIN ___SB_SCHEMA_NAME___artist ra ON "
                    "r.artist_id=ra.artist_id "
                "LEFT JOIN ___SB_SCHEMA_NAME___artist a ON "
                    "pc.playlist_artist_id=a.artist_id "
        "WHERE "
            "pc.playlist_id=%1 "
        "UNION "
        "SELECT "
            "pp.playlist_position, "
            "'SB_SONG_TYPE' AS SB_TYPE_ID, "
            "s.song_id AS SB_ITEM_ID, "
            "'song - ' || s.title || ' by ' || a.name "
            //"a.artist_id, "
            //"a.name "
        "FROM "
            "___SB_SCHEMA_NAME___playlist_performance pp  "
                "JOIN ___SB_SCHEMA_NAME___song s ON "
                    "pp.song_id=s.song_id "
                "JOIN ___SB_SCHEMA_NAME___artist a ON "
                    "pp.artist_id=a.artist_id "
                "JOIN ___SB_SCHEMA_NAME___record_performance rp ON "
                    "pp.song_id=rp.song_id AND "
                    "pp.artist_id=rp.artist_id AND "
                    "pp.record_id=rp.record_id AND "
                    "pp.record_position=rp.record_position "
                "JOIN ___SB_SCHEMA_NAME___record r ON "
                    "rp.record_id=r.record_id "
        "WHERE "
            "pp.playlist_id=%1 "
    ).arg(id.sb_item_id);

    return new SBModelSonglist(q);
}

SBModelSonglist*
SBModelPlaylist::getAllPlaylists()
{
    //	Main query
    QString q=QString
    (
        "SELECT "
            "p.playlist_id AS SB_PLAYLIST_ID, "
            "p.name "
        "FROM "
            "___SB_SCHEMA_NAME___playlist p "
        "ORDER BY "
            "p.name "
    );

    return new SBModelSonglist(q);
}
