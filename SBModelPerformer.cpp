#include "Context.h"
#include "DataAccessLayer.h"
#include "SBID.h"
#include "SBModelPerformer.h"
#include "SBModelSonglist.h"

SBModelPerformer::SBModelPerformer()
{
}

SBModelPerformer::~SBModelPerformer()
{
}

SBID
SBModelPerformer::getDetail(const SBID& id)
{
    SBID result=id;

    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());

    QString q=QString
    (
        "SELECT "
            "a.name, "
            "a.www, "
            "a.notes, "
            "a.mbid, "
            "COALESCE(r.record_count,0) AS record_count, "
            "COALESCE(s.song_count,0) AS song_count "
        "FROM "
                "___SB_SCHEMA_NAME___artist a "
                "LEFT JOIN "
                    "( "
                        "SELECT r.artist_id,COUNT(*) as record_count "
                        "FROM ___SB_SCHEMA_NAME___record r  "
                        "WHERE r.artist_id=%1 "
                        "GROUP BY r.artist_id "
                    ") r ON a.artist_id=r.artist_id "
                "LEFT JOIN "
                    "( "
                        "SELECT rp.artist_id,COUNT(DISTINCT song_id) as song_count "
                        "FROM ___SB_SCHEMA_NAME___record_performance rp  "
                        "WHERE rp.artist_id=%1 "
                        "GROUP BY rp.artist_id "
                    ") s ON a.artist_id=s.artist_id "
        "WHERE "
            "a.artist_id=%1"
    ).arg(id.sb_item_id);
    dal->customize(q);

    QSqlQuery query(q,db);
    query.next();

    result.sb_item_type    =SBID::sb_type_performer;
    result.sb_item_id      =id.sb_item_id;
    result.sb_mbid         =query.value(3).toString();
    result.sb_performer_id1=id.sb_item_id;
    result.performerName   =query.value(0).toString();
    result.url             =query.value(1).toString();
    result.notes           =query.value(2).toString();
    result.count1          =query.value(4).toInt();
    result.count2          =query.value(5).toInt();

    if(result.url.length()>0 && result.url.toLower().left(7)!="http://")
    {
        result.url="http://"+result.url;
    }
    return result;
}

SBModelSonglist*
SBModelPerformer::getAllAlbums(const SBID& id)
{
    QString q=QString
    (
        "SELECT "
            "r.record_id AS SB_RECORD_ID, "
            "r.title AS \"title\", "
            "r.year AS \"year released\", "
            "a.artist_id AS SB_PERFORMER_ID, "
            "a.name \"performer\" "
        "FROM "
            "___SB_SCHEMA_NAME___artist a "
                "JOIN ___SB_SCHEMA_NAME___record r ON "
                    "a.artist_id=r.artist_id "
        "WHERE "
            "a.artist_id=%1 "
        "UNION "
        "SELECT "
            "r.record_id AS SB_RECORD_ID, "
            "r.title AS \"title\", "
            "r.year AS \"year released\", "
            "a1.artist_id AS SB_PERFORMER_ID, "
            "a1.name AS \"performer\" "
        "FROM "
            "___SB_SCHEMA_NAME___artist a "
                "JOIN ___SB_SCHEMA_NAME___record_performance rp ON "
                    "a.artist_id=rp.artist_id "
                "JOIN ___SB_SCHEMA_NAME___record r ON "
                    "rp.record_id=r.record_id "
                "JOIN ___SB_SCHEMA_NAME___artist a1 ON "
                    "r.artist_id=a1.artist_id "
        "WHERE "
            "a.artist_id=%1 "
        "ORDER BY  "
            "1 "
    ).arg(id.sb_item_id);

    return new SBModelSonglist(q);
}

SBModelSonglist*
SBModelPerformer::getAllCharts(const SBID& id)
{
    QString q=QString
    (
        "SELECT "
            "cp.chart_position AS \"position\", "
            "s.song_id AS SB_SONG_ID, "
            "s.title AS \"song\" , "
            "c.chart_id AS SB_CHART_ID, "
            "c.name AS \"chart\" "
        "FROM "
            "___SB_SCHEMA_NAME___chart c "
                "JOIN ___SB_SCHEMA_NAME___chart_performance cp ON "
                    "c.chart_id=cp.chart_id "
                "JOIN ___SB_SCHEMA_NAME___song s ON "
                    "cp.song_id=s.song_id "
        "WHERE "
            "cp.artist_id=%1 "
        "ORDER BY 1"
    ).arg(id.sb_item_id);

    return new SBModelSonglist(q);
}

SBModelSonglist*
SBModelPerformer::getAllSongs(const SBID& id)
{
    QString q=QString
    (
        "SELECT "
            "s.song_id AS SB_SONG_ID, "
            "s.title AS \"title\", "
            "p.year AS \"year released\" "
        "FROM "
            "___SB_SCHEMA_NAME___song s "
                "JOIN ___SB_SCHEMA_NAME___performance p ON "
                    "s.song_id=p.song_id "
                "JOIN ___SB_SCHEMA_NAME___artist a ON "
                    "p.artist_id=a.artist_id "
        "WHERE "
            "a.artist_id=%1 "
        "ORDER BY "
            "s.title "
    ).arg(id.sb_item_id);

    return new SBModelSonglist(q);
}

void
SBModelPerformer::updateHomePage(const SBID &id)
{
    qDebug() << SB_DEBUG_INFO << id;
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());

    QString q=QString
    (
        "UPDATE "
            "___SB_SCHEMA_NAME___artist "
        "SET "
            "www='%1' "
        "WHERE "
            "artist_id=%2"
    ).arg(id.url).arg(id.sb_item_id);
    dal->customize(q);

    qDebug() << SB_DEBUG_INFO << q;
    QSqlQuery query(q,db);
    query.exec();
}

void
SBModelPerformer::updateMBID(const SBID &id)
{
    qDebug() << SB_DEBUG_INFO << id;
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());

    QString q=QString
    (
        "UPDATE "
            "___SB_SCHEMA_NAME___artist "
        "SET "
            "mbid='%1' "
        "WHERE "
            "artist_id=%2"
    ).arg(id.sb_mbid).arg(id.sb_item_id);
    dal->customize(q);

    qDebug() << SB_DEBUG_INFO << q;
    QSqlQuery query(q,db);
    query.exec();
}
