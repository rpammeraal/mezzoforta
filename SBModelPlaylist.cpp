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


//	OLD

SBModelPlaylist::SBModelPlaylist() : SBModel()
{
    applyFilter(QString(),0);
}

SBModelPlaylist::~SBModelPlaylist()
{
}

void
SBModelPlaylist::applyFilter(const QString& filter, const bool doExactSearch)
{
    Q_UNUSED(filter);
    Q_UNUSED(doExactSearch);

    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();

    QString q=QString("SELECT playlist_id AS \"sb_playlist_id\", name AS \"title\", duration as \"duration\" FROM ___SB_SCHEMA_NAME___playlist ORDER BY name");

    QSqlQueryModel::setQuery(dal->customize(q),QSqlDatabase::database(dal->getConnectionName()));
    while(QSqlQueryModel::canFetchMore())
    {
        QSqlQueryModel::fetchMore();
    }
}

bool
SBModelPlaylist::assign(const QString& dstID, const SBID& id)
{
    qDebug() << SB_DEBUG_INFO << dstID << id;
    QString q;
    QString s;

    switch(id.sb_item_type)
    {
    case SBID::sb_type_song:
        q=QString
            (
                "INSERT INTO ___SB_SCHEMA_NAME___playlist_performance "
                "(playlist_id,playlist_position,song_id,artist_id,record_id,record_position,timestamp) "
                "SELECT %1, ___SB_DB_ISNULL___(MAX(playlist_position)+1,0), %2, %3, %4, %5, ___SB_DB_GETDATE___ "
                "FROM ___SB_SCHEMA_NAME___playlist_performance "
                "WHERE playlist_id=%1 AND "
                "NOT EXISTS "
                "( SELECT 1 FROM ___SB_SCHEMA_NAME___playlist WHERE playlist_id=%1 AND song_id=%2 AND artist_id=%3 AND record_id=%4 AND record_position=%5 ) "
            ).
                arg(dstID.toInt()).
                arg(id.sb_song_id1).
                arg(id.sb_performer_id1).
                arg(id.sb_album_id1).
                arg(id.sb_album_position)
            ;
        s=QString("Assigned %1 [%2] by %3").arg(id.songTitle).arg(id.albumTitle).arg(id.performerName);
        break;
    }

    if(q.length()>0)
    {
        DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
        dal->customize(q);
        qDebug() << SB_DEBUG_INFO << q;
        QSqlQuery c1(QSqlDatabase::database(dal->getConnectionName()));

        c1.exec(q);
        handleSQLError();

        Context::instance()->getController()->updateStatusBar(s);
    }

    return 0;
}

SBID::sb_type
SBModelPlaylist::getSBType(int column) const
{
    Q_UNUSED(column);
    return SBID::sb_type_playlist;
}

void
SBModelPlaylist::resetFilter()
{
    applyFilter(QString(),0);
}

//bool
//SBModelPlaylist::setData(const QModelIndex &index, const QVariant &value, int role)
//{
//    qDebug() << "SBModelPlaylist:setData called:index=" << index << ":value=" << value << ":role=" << role;
//    return 0;
//}

const char*
SBModelPlaylist::whoami() const
{
    return "SBModelPlaylist";
}

///	PROTECTED
SBID
SBModelPlaylist::getSBID(const QModelIndex &i) const
{
    Q_UNUSED(i);
    qDebug() << SB_DEBUG_INFO;
    SBID id;
    return id;
//    QByteArray encodedData;
//    QDataStream ds(&encodedData, QIODevice::WriteOnly);
//
//    ds << headerData(i.column()-1,Qt::Horizontal,Qt::DisplayRole).toString();
//    const QModelIndex n=this->index(i.row(),i.column()-1);
//    ds << data(n, Qt::DisplayRole).toString();
//
//    return encodedData;
}

