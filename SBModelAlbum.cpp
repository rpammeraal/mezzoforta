#include "Context.h"
#include "DataAccessLayer.h"
#include "SBModelAlbum.h"
#include "SBModelSonglist.h"

SBID
SBModelAlbum::getDetail(const SBID& id)
{
    SBID result=id;

    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());

    QString q=QString
    (
        "SELECT "
            "r.title , "
            "r.genre, "
            "r.year, "
            "r.notes, "
            "a.name "
        "FROM "
            "___SB_SCHEMA_NAME___record r "
                "JOIN ___SB_SCHEMA_NAME___artist a ON "
                    "a.artist_id=r.artist_id "
        "WHERE "
            "r.record_id=%1"
    ).arg(id.sb_item_id);
    dal->customize(q);

    QSqlQuery query(q,db);
    query.next();

    result.sb_item_type =SBID::sb_type_album;
    result.sb_item_id   =id.sb_item_id;
    result.sb_album_id1 =id.sb_item_id;
    result.performerName=query.value(4).toString();
    result.albumTitle   =query.value(0).toString();
    result.year         =query.value(2).toInt();
    result.genre        =query.value(1).toString();
    result.notes        =query.value(3).toString();

    return result;
}

SBModelSonglist*
SBModelAlbum::getAllSongs(const SBID& id)
{
    QString q=QString
    (
        "SELECT "
            "rp.record_position AS \"#\", "
            "s.song_id AS SB_SONG_ID, "
            "s.title AS \"song\", "
            "rp.duration AS \"duration\", "
            "a.artist_id AS SB_ARTIST_ID, "
            "a.name AS \"performer\" "
        "FROM "
            "___SB_SCHEMA_NAME___record_performance rp "
                "join ___SB_SCHEMA_NAME___song s on "
                    "rp.song_id=s.song_id "
                "join ___SB_SCHEMA_NAME___artist a on "
                    "rp.artist_id=a.artist_id "
        "WHERE "
            "record_id=%1 "
        "ORDER BY 1"
    ).arg(id.sb_item_id);

    return new SBModelSonglist(q);
}
