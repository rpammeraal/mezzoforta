#include "Context.h"
#include "DataAccessLayer.h"
#include "SBModelAlbum.h"
#include "SBSqlQueryModel.h"

SBID
SBModelAlbum::getDetail(const SBID& id)
{
    SBID result=id;
    qDebug() << SB_DEBUG_INFO << "id.wiki=" << id.wiki;

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
    result.sb_album_id  =id.sb_item_id;
    result.performerName=query.value(4).toString();
    result.albumTitle   =query.value(0).toString();
    result.year         =query.value(2).toInt();
    result.genre        =query.value(1).toString();
    result.notes        =query.value(3).toString();

    qDebug() << SB_DEBUG_INFO << "result.wiki=" << result.wiki;
    return result;
}

SBSqlQueryModel*
SBModelAlbum::getAllSongs(const SBID& id)
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
