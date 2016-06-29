#include "BackgroundThread.h"
#include "Common.h"
#include "Context.h"
#include "Controller.h"
#include "DataAccessLayer.h"
#include "SBID.h"
#include "SBSqlQueryModel.h"
#include "DataEntityPlaylist.h"

#include <QMessageBox>

///	PUBLIC METHODS
DataEntityPlaylist::DataEntityPlaylist()
{
    qDebug() << SB_DEBUG_INFO << "ctor";
    init();
}

DataEntityPlaylist::~DataEntityPlaylist()
{
    qDebug() << SB_DEBUG_INFO << "dtor";
}

void
DataEntityPlaylist::assignPlaylistItem(const SBID &toBeAssignedID, const SBID &toPlaylistID) const
{
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
    QString q;

    qDebug() << SB_DEBUG_INFO << "toBeAssignedID" << toBeAssignedID;
    qDebug() << SB_DEBUG_INFO << "toPlaylistID" << toPlaylistID;

    switch(toBeAssignedID.sb_item_type())
    {
    case SBID::sb_type_song:
        qDebug() << SB_DEBUG_INFO;
        if(toBeAssignedID.sb_album_id==-1 || toBeAssignedID.sb_playlist_position==-1)
        {
            qDebug() << "****************************************************************************************";
            qDebug() << "****************************************************************************************";
            qDebug() << "****************************************************************************************";
            qDebug() << "****************************************************************************************";
            qDebug() << "****************************************************************************************";
            qDebug() << "****************************************************************************************";
            qDebug() << "****************************************************************************************";
            qDebug() << "****************************************************************************************";
            qDebug() << "assignment of song without album";
            qDebug() << "****************************************************************************************";
            qDebug() << "****************************************************************************************";
            qDebug() << "****************************************************************************************";
            qDebug() << "****************************************************************************************";
            qDebug() << "****************************************************************************************";
            qDebug() << "****************************************************************************************";
            qDebug() << "****************************************************************************************";
            qDebug() << "****************************************************************************************";

        }
        q=QString
          (
            "INSERT INTO ___SB_SCHEMA_NAME___playlist_performance "
                "(playlist_id, playlist_position, song_id, artist_id, record_id, record_position, timestamp) "
            "SELECT "
                "%1, %7(a.playlist_position,0)+1, %2, %3, %4, %5, %6 "
            "FROM "
                "( "
                    "SELECT MAX(playlist_position) AS playlist_position "
                    "FROM "
                    "( "
                        "SELECT MAX(playlist_position) AS playlist_position "
                        "FROM ___SB_SCHEMA_NAME___playlist_performance "
                        "WHERE playlist_id=%1 "
                        "UNION "
                        "SELECT MAX(playlist_position) AS playlist_composite "
                        "FROM ___SB_SCHEMA_NAME___playlist_composite "
                        "WHERE playlist_id=%1 "
                    ") b "
                ") a "
            "WHERE "
                "NOT EXISTS "
                "( "
                    "SELECT NULL FROM ___SB_SCHEMA_NAME___playlist_performance pp "
                    "WHERE "
                        "pp.playlist_id=%1 AND "
                        "pp.song_id=%2 AND "
                        "pp.artist_id=%3 AND "
                        "pp.record_id=%4 AND "
                        "pp.record_position=%5 "
                ") "
          ).arg(toPlaylistID.sb_playlist_id)
           .arg(toBeAssignedID.sb_song_id)
           .arg(toBeAssignedID.sb_performer_id)
           .arg(toBeAssignedID.sb_album_id)
           .arg(toBeAssignedID.sb_position)
           .arg(dal->getGetDate())
           .arg(dal->getIsNull());
        break;

    case SBID::sb_type_performer:
        qDebug() << SB_DEBUG_INFO;
        q=QString
          (
            "INSERT INTO ___SB_SCHEMA_NAME___playlist_composite "
                "(playlist_id, timestamp, playlist_position, playlist_artist_id) "
            "SELECT "
                "%1, %2, %3(playlist_position,0)+1, %4 "
            "FROM "
                "( "
                    "SELECT MAX(playlist_position) AS playlist_position "
                    "FROM "
                    "( "
                        "SELECT MAX(playlist_position) AS playlist_position "
                        "FROM ___SB_SCHEMA_NAME___playlist_performance "
                        "WHERE playlist_id=%1 "
                        "UNION "
                        "SELECT MAX(playlist_position) "
                        "FROM ___SB_SCHEMA_NAME___playlist_composite "
                        "WHERE playlist_id=%1 "
                    ") b "
                ") a "
            "WHERE "
                "NOT EXISTS "
                "( "
                    "SELECT NULL FROM ___SB_SCHEMA_NAME___playlist_composite pp "
                    "WHERE "
                        "pp.playlist_id=%1 AND "
                        "pp.playlist_artist_id=%4 "
                ") "
          ).arg(toPlaylistID.sb_playlist_id)
           .arg(dal->getGetDate())
           .arg(dal->getIsNull())
           .arg(toBeAssignedID.sb_performer_id)
        ;
        break;

    case SBID::sb_type_album:
        qDebug() << SB_DEBUG_INFO;
        q=QString
          (
            "INSERT INTO ___SB_SCHEMA_NAME___playlist_composite "
                "(playlist_id, timestamp, playlist_position, playlist_record_id) "
            "SELECT "
                "%1, %2, %3(playlist_position,0)+1, %4 "
            "FROM "
                "( "
                    "SELECT MAX(playlist_position) AS playlist_position "
                    "FROM "
                    "( "
                        "SELECT MAX(playlist_position) AS playlist_position "
                        "FROM ___SB_SCHEMA_NAME___playlist_performance "
                        "WHERE playlist_id=%1 "
                        "UNION "
                        "SELECT MAX(playlist_position) "
                        "FROM ___SB_SCHEMA_NAME___playlist_composite "
                        "WHERE playlist_id=%1 "
                    ") b "
                ") a "
            "WHERE "
                "NOT EXISTS "
                "( "
                    "SELECT NULL FROM ___SB_SCHEMA_NAME___playlist_composite pp "
                    "WHERE "
                        "pp.playlist_id=%1 AND "
                        "pp.playlist_record_id=%4 "
                ") "
          ).arg(toPlaylistID.sb_playlist_id)
           .arg(dal->getGetDate())
           .arg(dal->getIsNull())
           .arg(toBeAssignedID.sb_album_id)
        ;
        break;

    case SBID::sb_type_chart:
        qDebug() << SB_DEBUG_INFO;
        break;

    case SBID::sb_type_playlist:
        qDebug() << SB_DEBUG_INFO;
        q=QString
          (
            "INSERT INTO ___SB_SCHEMA_NAME___playlist_composite "
                "(playlist_id, timestamp, playlist_position, playlist_playlist_id) "
            "SELECT "
                "%1, %2, %3(playlist_position,0)+1, %4 "
            "FROM "
                "( "
                    "SELECT MAX(playlist_position) AS playlist_position "
                    "FROM "
                    "( "
                        "SELECT MAX(playlist_position) AS playlist_position "
                        "FROM ___SB_SCHEMA_NAME___playlist_performance "
                        "WHERE playlist_id=%1 "
                        "UNION "
                        "SELECT MAX(playlist_position) "
                        "FROM ___SB_SCHEMA_NAME___playlist_composite "
                        "WHERE playlist_id=%1 "
                    ") b "
                ") a "
            "WHERE "
                "NOT EXISTS "
                "( "
                    "SELECT NULL FROM ___SB_SCHEMA_NAME___playlist_composite pp "
                    "WHERE "
                        "pp.playlist_id=%1 AND "
                        "pp.playlist_playlist_id=%4 "
                ") "
          ).arg(toPlaylistID.sb_playlist_id)
           .arg(dal->getGetDate())
           .arg(dal->getIsNull())
           .arg(toBeAssignedID.sb_playlist_id)
        ;
        break;

    case SBID::sb_type_allsongs:
    case SBID::sb_type_songsearch:
    case SBID::sb_type_invalid:
    case SBID::sb_type_current_playlist:
    case SBID::sb_type_position:
        qDebug() << SB_DEBUG_INFO;
        break;
    }

    dal->customize(q);
    if(q.length()>0)
    {
        QSqlQuery insert(q,db);
        Q_UNUSED(insert);
        qDebug() << SB_DEBUG_INFO << q;
        recalculatePlaylistDuration(toPlaylistID);
        qDebug() << SB_DEBUG_INFO;
    }
    qDebug() << SB_DEBUG_INFO;
}

SBID
DataEntityPlaylist::createNewPlaylist() const
{
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
    SBID result;
    QString q;

    //	Get next ID available
    q=QString("SELECT %1(MAX(playlist_id),0)+1 FROM ___SB_SCHEMA_NAME___playlist ").arg(dal->getIsNull());
    dal->customize(q);
    qDebug() << SB_DEBUG_INFO << q;
    QSqlQuery qID(q,db);
    qID.next();

    result.assign(SBID::sb_type_playlist,qID.value(0).toInt());

    //	Figure out name of next playlist
    QString playlistName;
    int maxNum=1;
    q=QString("SELECT name FROM ___SB_SCHEMA_NAME___playlist WHERE name %1 \"New Playlist%\"").arg(dal->getILike());
    dal->customize(q);
    qDebug() << SB_DEBUG_INFO << q;
    QSqlQuery qName(q,db);
    while(qName.next())
    {
        QString existing=qName.value(0).toString();
        qDebug() << SB_DEBUG_INFO << existing;
        existing.replace("New Playlist","");
        int i=existing.toInt();
        qDebug() << SB_DEBUG_INFO << i << maxNum;
        if(i>=maxNum)
        {
            maxNum=i+1;
        }
    }
    result.playlistName=QString("New Playlist%1").arg(maxNum);

    //	Insert
    q=QString("INSERT INTO ___SB_SCHEMA_NAME___playlist (playlist_id, name,created,play_mode) VALUES(%1,'%2',%3,1)")
            .arg(result.sb_playlist_id)
            .arg(result.playlistName)
            .arg(dal->getGetDate());
    dal->customize(q);
    qDebug() << SB_DEBUG_INFO << q;
    QSqlQuery insert(q,db);
    Q_UNUSED(insert);
    //	insert.exec();	-- no need to run on insert statements

    qDebug() << SB_DEBUG_INFO << result;
    return result;
}

void
DataEntityPlaylist::deletePlaylistItem(const SBID &toBeDeleted, const SBID &fromID) const
{
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
    QString q;

    qDebug() << SB_DEBUG_INFO << toBeDeleted;

    switch(toBeDeleted.sb_item_type())
    {
    case SBID::sb_type_song:
        q=QString
          (
            "DELETE FROM ___SB_SCHEMA_NAME___playlist_performance "
            "WHERE "
                "playlist_id=%1 AND "
                "playlist_position=%2 "
          ).arg(fromID.sb_playlist_id)
           .arg(toBeDeleted.sb_playlist_position);
        break;

    case SBID::sb_type_performer:
    case SBID::sb_type_album:
    case SBID::sb_type_chart:
    case SBID::sb_type_playlist:
        q=QString
          (
            "DELETE FROM ___SB_SCHEMA_NAME___playlist_composite "
            "WHERE "
                "playlist_id=%1 AND "
                "playlist_position=%2 "
          ).arg(fromID.sb_playlist_id)
           .arg(toBeDeleted.sb_playlist_position)
        ;
        break;

    case SBID::sb_type_allsongs:
    case SBID::sb_type_songsearch:
    case SBID::sb_type_invalid:
    default:
        break;
    }

    dal->customize(q);
    if(q.length()>0)
    {
        qDebug() << SB_DEBUG_INFO << q;
        QSqlQuery remove(q,db);
        Q_UNUSED(remove);
        reorderPlaylistPositions(fromID);
        recalculatePlaylistDuration(fromID);
    }
}

SBID
DataEntityPlaylist::getDetailPlaylistItemSong(const SBID &id) const
{
    qDebug() << SB_DEBUG_INFO
             << "song_id=" << id.sb_song_id
             << "artist_id=" << id.sb_performer_id
             << "record_id=" << id.sb_album_id
             << "record_position=" << id.sb_position
             << "playlist_id=" << id.sb_playlist_id
             << "playlist_position" << id.sb_playlist_position
    ;
    SBID result=id;
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());

    QString q=QString
    (
        "SELECT DISTINCT "
            "s.song_id, "
            "a.artist_id, "
            "r.record_id, "
            "rp.record_position, "
            "rp.duration, "
            "s.title, "
            "a.name, "
            "r.title, "
            "op.path "
        "FROM "
            "___SB_SCHEMA_NAME___playlist_performance pp "
                "JOIN ___SB_SCHEMA_NAME___song s ON "
                    "pp.song_id=s.song_id "
                "JOIN ___SB_SCHEMA_NAME___artist a ON "
                    "pp.artist_id=a.artist_id "
                "JOIN ___SB_SCHEMA_NAME___record r ON "
                    "pp.record_id=r.record_id "
                "JOIN ___SB_SCHEMA_NAME___record_performance rp ON "
                    "pp.song_id=rp.song_id AND "
                    "pp.artist_id=rp.artist_id AND "
                    "pp.record_id=rp.record_id AND "
                    "pp.record_position=rp.record_position "
                "JOIN ___SB_SCHEMA_NAME___online_performance op ON "
                    "pp.song_id=op.song_id AND "
                    "pp.artist_id=op.artist_id AND "
                    "pp.record_id=op.record_id AND "
                    "pp.record_position=op.record_position "
        "WHERE "
            "pp.playlist_id=%1 AND "
            "pp.playlist_position=%2 "
    )
        .arg(id.sb_playlist_id)
        .arg(id.sb_playlist_position)
    ;
    dal->customize(q);

    qDebug() << SB_DEBUG_INFO << q;
    QSqlQuery query(q,db);

    if(query.next())
    {
        result.assign(SBID::sb_type_song,query.value(0).toInt());
        result.sb_performer_id=query.value(1).toInt();
        result.sb_album_id    =query.value(2).toInt();
        result.sb_position    =query.value(3).toInt();
        result.duration       =query.value(4).toTime();
        result.songTitle      =query.value(5).toString();
        result.performerName  =query.value(6).toString();
        result.albumTitle     =query.value(7).toString();
        result.path           =query.value(8).toString();
    }
    qDebug() << SB_DEBUG_INFO
             << "song_id=" << result.sb_song_id
             << "artist_id=" << result.sb_performer_id
             << "record_id=" << result.sb_album_id
             << "record_position=" << result.sb_position
             << "playlist_id=" << result.sb_playlist_id
             << "playlist_position" << result.sb_playlist_position
             << "path" << result.path
             << "duration" << result.duration
    ;
    return result;
}

void
DataEntityPlaylist::deletePlaylist(const SBID &id) const
{
    qDebug() << SB_DEBUG_INFO;
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
    QString qTemplate=QString("DELETE FROM ___SB_SCHEMA_NAME___%1 WHERE playlist_id=%2");

    QStringList l;
    l.append("playlist_performance");
    l.append("playlist_composite");
    l.append("playlist");

    for(int i=0;i<l.count();i++)
    {
        QString q=qTemplate.arg(l.at(i)).arg(id.sb_playlist_id);
        dal->customize(q);
        qDebug() << SB_DEBUG_INFO << q;
        QSqlQuery toExec(q,db);
        Q_UNUSED(toExec);
    }
}

SBID
DataEntityPlaylist::getDetail(const SBID& id) const
{
    SBID result=id;
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());

    QString q=QString
    (
        "SELECT DISTINCT "
            "p.name, "
            "p.duration, "
            "COALESCE(a.num,0)+COALESCE(b.num,0)  "
        "FROM "
            "___SB_SCHEMA_NAME___playlist p "
                "LEFT JOIN "
                    "( "
                        "SELECT "
                            "pp.playlist_id, "
                            "COUNT(*) AS num "
                        "FROM "
                            "___SB_SCHEMA_NAME___playlist_performance pp  "
                        "WHERE "
                            "pp.playlist_id=%1 "
                        "GROUP BY "
                            "pp.playlist_id "
                    ") a ON a.playlist_id=p.playlist_id "
                "LEFT JOIN "
                    "( "
                        "SELECT "
                            "pp.playlist_id, "
                            "COUNT(*) AS num "
                        "FROM "
                            "___SB_SCHEMA_NAME___playlist_composite pp  "
                        "WHERE "
                            "pp.playlist_id=%1 "
                        "GROUP BY "
                            "pp.playlist_id "
                    ") b ON b.playlist_id=p.playlist_id "
        "WHERE "
            "p.playlist_id=%1 "
    ).arg(id.sb_playlist_id);
    dal->customize(q);

    qDebug() << SB_DEBUG_INFO << q;

    QSqlQuery query(q,db);

    if(query.next())
    {
        result.assign(SBID::sb_type_playlist,id.sb_playlist_id);
        result.playlistName  =query.value(0).toString();
        result.duration      =query.value(1).toString();
        result.count1        =query.value(2).toInt();
    }

    return result;
}

SBSqlQueryModel*
DataEntityPlaylist::getAllItemsByPlaylist(const SBID& id) const
{
    this->reorderPlaylistPositions(id);

    //	Main query
    QString q=QString
    (
        "SELECT "
            "pc.playlist_position as \"#\", "
            "CASE "
                "WHEN pc.playlist_playlist_id IS NOT NULL THEN %2 "
                "WHEN pc.playlist_chart_id    IS NOT NULL THEN %3 "
                "WHEN pc.playlist_record_id   IS NOT NULL THEN %4 "
                "WHEN pc.playlist_artist_id   IS NOT NULL THEN %5 "
            "END AS SB_ITEM_TYPE, "
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
                "WHEN pc.playlist_artist_id   IS NOT NULL AND pc.playlist_record_id IS NOT NULL THEN ' - ' || a.name "
                "ELSE '' "
            "END  as item, "
            "0 AS SB_ITEM_TYPE1, "
            "0 AS SB_ALBUM_ID, "
            "0 AS SB_ITEM_TYPE2, "
            "0 AS SB_POSITION_ID, "
            "0 AS SB_ITEM_TYPE3, "
            "a.artist_id AS SB_PERFORMER_ID "
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
            "%6, "
            "s.song_id, "
            "'song - ' || s.title || ' / ' || a.name || ' - ' || r.title, "
            "%3 AS SB_ITEM_TYPE1, "
            "r.record_id AS SB_ALBUM_ID, "
            "%7 AS SB_ITEM_TYPE2, "
            "rp.record_position AS SB_POSITION_ID, "
            "%5 AS SB_ITEM_TYPE3, "
            "pp.artist_id AS SB_PERFORMER_ID "
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
        "ORDER BY 1"
    )
            .arg(id.sb_playlist_id)
            .arg(SBID::sb_type_playlist)
            .arg(SBID::sb_type_chart)
            .arg(SBID::sb_type_album)
            .arg(SBID::sb_type_performer)  //	5
            .arg(SBID::sb_type_song)
            .arg(SBID::sb_type_position);

    return new SBSqlQueryModel(q,0);
}

void
DataEntityPlaylist::getAllItemsByPlaylistRecursive(QList<SBID>& compositesTraversed,QList<SBID>& allSongs,const SBID &rootID) const
{
    this->reorderPlaylistPositions(rootID);
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
    QString q;

    qDebug() << SB_DEBUG_INFO << "retrieve " << rootID;
    if(compositesTraversed.contains(rootID))
    {
        return;
    }
    compositesTraversed.append(rootID);

    //	If rootID is a playlist, traverse trough all items within this playlist, recurse when neccessary.
    switch(rootID.sb_item_type())
    {
    case SBID::sb_type_playlist:
        q=QString
            (
                "SELECT "
                    "0 AS composite_flag, "
                    "pp.playlist_position, "
                    "0 AS playlist_id, "
                    "0 AS chart_id, "
                    "pp.record_id, "
                    "pp.artist_id, "
                    "pp.song_id, "
                    "pp.record_position, "
                    "op.path, "
                    "s.title, "
                    "a.name, "
                    "r.title, "
                    "rp.duration "
                "FROM "
                    "___SB_SCHEMA_NAME___playlist_performance pp "
                        "JOIN ___SB_SCHEMA_NAME___record_performance rp ON "
                            "pp.artist_id=rp.artist_id AND "
                            "pp.song_id=rp.song_id AND "
                            "pp.record_id=rp.record_id AND "
                            "pp.record_position=rp.record_position "
                        "JOIN ___SB_SCHEMA_NAME___online_performance op ON "
                            "op.artist_id=rp.op_artist_id AND "
                            "op.song_id=rp.op_song_id AND "
                            "op.record_id=rp.op_record_id AND "
                            "op.record_position=rp.op_record_position "
                        "JOIN ___SB_SCHEMA_NAME___song s ON "
                            "pp.song_id=s.song_id "
                        "JOIN ___SB_SCHEMA_NAME___artist a ON "
                            "pp.artist_id=a.artist_id "
                        "JOIN ___SB_SCHEMA_NAME___record r ON "
                            "pp.record_id=r.record_id "
                "WHERE "
                    "pp.playlist_id=%2 "
                "UNION "
                "SELECT "
                    "1 AS composite_flag,"
                    "pc.playlist_position, "
                    "%1(playlist_playlist_id,0) AS playlist_id, "
                    "%1(playlist_chart_id,0) AS chart_id, "
                    "%1(playlist_record_id,0) AS record_id, "
                    "%1(playlist_artist_id,0) AS artist_id, "
                    "0 AS song_id, "
                    "0 AS record_position, "
                    "'' AS path, "
                    "'' AS song_title, "
                    "'' AS performer_name, "
                    "'' AS record_title, "
                    "NULL AS duration "
                "FROM "
                    "___SB_SCHEMA_NAME___playlist_composite pc "
                "WHERE "
                    "pc.playlist_id=%2 "
                "ORDER BY "
                    "2 "
            )
                .arg(dal->getIsNull())
                .arg(rootID.sb_playlist_id)
            ;

        dal->customize(q);
        qDebug() << SB_DEBUG_INFO << q;
        {
            QSqlQuery allItems(q,db);

            while(allItems.next())
            {
                bool compositeFlag=allItems.value(0).toInt();
                int playlistID=allItems.value(2).toInt();
                int playlistPosition=allItems.value(1).toInt();
                int chartID=allItems.value(3).toInt();
                int albumID=allItems.value(4).toInt();
                int performerID=allItems.value(5).toInt();

                if(compositeFlag)
                {
                    SBID t;
                    if(playlistID!=0)
                    {
                        qDebug() << SB_DEBUG_INFO;
                        t.assign(SBID::sb_type_playlist,playlistID);
                    }
                    else if(chartID!=0)
                    {
                        qDebug() << SB_DEBUG_INFO;
                        t.assign(SBID::sb_type_chart,chartID);
                    }
                    else if(albumID!=0)
                    {
                        qDebug() << SB_DEBUG_INFO;
                        t.assign(SBID::sb_type_album,albumID);
                    }
                    else if(performerID!=0)
                    {
                        qDebug() << SB_DEBUG_INFO;
                        t.assign(SBID::sb_type_performer,performerID);
                    }
                    qDebug() << SB_DEBUG_INFO << t;

                    getAllItemsByPlaylistRecursive(compositesTraversed,allSongs,t);
                }
                else
                {
                int songID=allItems.value(6).toInt();
                    SBID song=SBID(SBID::sb_type_song,songID);
                    song.sb_album_id=albumID;
                    song.sb_performer_id=performerID;
                    song.sb_position=allItems.value(7).toInt();
                    song.path=allItems.value(8).toString();
                    song.songTitle=allItems.value(9).toString();
                    song.performerName=allItems.value(10).toString();
                    song.albumTitle=allItems.value(11).toString();
                    song.playPosition=allSongs.count()+1;
                    song.duration=allItems.value(12).toTime();
                    song.sb_playlist_position=playlistPosition;

                    if(allSongs.contains(song)==0)
                    {
                        allSongs.append(song);
                    }
                }
            }
        }
        //	We're now done for this item, if it is a playlist:
        //	-	playlist_performances were retrieved
        //	-	recursed through all playlist_composites
        //	Set q to <empty>, since other case statements will set this variable.
        q=QString();
        break;

    case SBID::sb_type_chart:
        q=QString
            (
                "SELECT "
                    "pp.artist_id, "
                    "pp.song_id, "
                    "pp.record_id, "
                    "pp.record_position, "
                    "rp.duration, "
                    "s.title, "
                    "a.name, "
                    "r.title, "
                    "op.path "
                "FROM "
                    "___SB_SCHEMA_NAME___chart_performance pp "
                        "JOIN ___SB_SCHEMA_NAME___record_performance rp ON "
                            "pp.artist_id=rp.artist_id AND "
                            "pp.song_id=rp.song_id AND "
                            "pp.record_id=rp.record_id AND "
                            "pp.record_position=rp.record_position "
                        "JOIN ___SB_SCHEMA_NAME___online_performance op ON "
                            "op.artist_id=rp.op_artist_id AND "
                            "op.song_id=rp.op_song_id AND "
                            "op.record_id=rp.op_record_id AND "
                            "op.record_position=rp.op_record_position "
                        "JOIN ___SB_SCHEMA_NAME___song s ON "
                            "pp.song_id=s.song_id "
                        "JOIN ___SB_SCHEMA_NAME___artist a ON "
                            "pp.artist_id=a.artist_id "
                        "JOIN ___SB_SCHEMA_NAME___record r ON "
                            "pp.record_id=r.record_id "
                "WHERE "
                    "pp.chart_id=%1 "
                "ORDER BY "
                    "pp.chart_position "
            )
                .arg(rootID.sb_playlist_id)
            ;
        break;

    case SBID::sb_type_album:
        q=QString
            (
                "SELECT "
                    "rp.artist_id, "
                    "rp.song_id, "
                    "rp.record_id, "
                    "rp.record_position, "
                    "rp.duration, "
                    "s.title, "
                    "a.name, "
                    "r.title, "
                    "op.path "
                "FROM "
                    "___SB_SCHEMA_NAME___record_performance rp "
                        "JOIN ___SB_SCHEMA_NAME___online_performance op ON "
                            "op.artist_id=rp.op_artist_id AND "
                            "op.song_id=rp.op_song_id AND "
                            "op.record_id=rp.op_record_id AND "
                            "op.record_position=rp.op_record_position "
                        "JOIN ___SB_SCHEMA_NAME___song s ON "
                            "rp.song_id=s.song_id "
                        "JOIN ___SB_SCHEMA_NAME___artist a ON "
                            "rp.artist_id=a.artist_id "
                        "JOIN ___SB_SCHEMA_NAME___record r ON "
                            "rp.record_id=r.record_id "
                "WHERE "
                    "rp.record_id=%1 "
                "ORDER BY "
                    "rp.record_position "
            )
                .arg(rootID.sb_album_id)
            ;
        break;

    case SBID::sb_type_performer:
        q=QString
            (
                "SELECT "
                    "rp.artist_id, "
                    "rp.song_id, "
                    "rp.record_id, "
                    "rp.record_position, "
                    "rp.duration, "
                    "s.title, "
                    "a.name, "
                    "r.title, "
                    "op.path "
                "FROM "
                    "___SB_SCHEMA_NAME___record_performance rp "
                        "JOIN ___SB_SCHEMA_NAME___online_performance op ON "
                            "op.artist_id=rp.op_artist_id AND "
                            "op.song_id=rp.op_song_id AND "
                            "op.record_id=rp.op_record_id AND "
                            "op.record_position=rp.op_record_position "
                        "JOIN ___SB_SCHEMA_NAME___song s ON "
                            "rp.song_id=s.song_id "
                        "JOIN ___SB_SCHEMA_NAME___artist a ON "
                            "rp.artist_id=a.artist_id "
                        "JOIN ___SB_SCHEMA_NAME___record r ON "
                            "rp.record_id=r.record_id "
                "WHERE "
                    "rp.artist_id=%2"
            )
                .arg(rootID.sb_performer_id)
            ;
        break;

    case SBID::sb_type_invalid:
    case SBID::sb_type_song:
    case SBID::sb_type_position:
    case SBID::sb_type_allsongs:
    case SBID::sb_type_songsearch:
    case SBID::sb_type_current_playlist:
        break;
    }

    if(q.length())
    {
        dal->customize(q);
        qDebug() << SB_DEBUG_INFO << q;
        QSqlQuery querySong(q,db);
        while(querySong.next())
        {
            SBID song(SBID::sb_type_song,querySong.value(1).toInt());
            song.sb_performer_id=querySong.value(0).toInt();
            song.sb_album_id=querySong.value(2).toInt();
            song.sb_position=querySong.value(3).toInt();
            song.duration=querySong.value(4).toTime();
            song.songTitle=querySong.value(5).toString();
            song.performerName=querySong.value(6).toString();
            song.albumTitle=querySong.value(7).toString();
            song.path=querySong.value(8).toString();
            song.playPosition=allSongs.count()+1;

            if(allSongs.contains(song)==0)
            {
                allSongs.append(song);
            }
        }
    }
    return;
}

SBSqlQueryModel*
DataEntityPlaylist::getAllPlaylists() const
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

    return new SBSqlQueryModel(q);
}

void
DataEntityPlaylist::recalculateAllPlaylistDurations() const
{
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());

    QString q=QString
        (
            "SELECT DISTINCT "
                "0,playlist_id "
            "FROM "
                "playlist_performance pp "
            "WHERE "
                "NOT EXISTS "
                "( "
                    "SELECT "
                        "NULL "
                    "FROM "
                        "playlist_composite pc "
                    "WHERE "
                        "pp.playlist_id=pc.playlist_id "
                ") "
            "UNION "
            "SELECT DISTINCT "
                "1,playlist_id "
            "FROM "
                "playlist_composite "
            "ORDER BY "
                "1,2 "
        );
    dal->customize(q);
    qDebug() << SB_DEBUG_INFO << q;

    QSqlQuery query(q,db);

    while(query.next())
    {
        SBID t(SBID::sb_type_playlist,query.value(1).toInt());

        recalculatePlaylistDuration(t);
    }
}

void
DataEntityPlaylist::recalculatePlaylistDuration(const SBID &id) const
{
    QList<SBID> compositesTraversed;
    QList<SBID> allSongs;

    //	Get all songs
    qDebug() << SB_DEBUG_INFO << id;
    compositesTraversed.clear();
    allSongs.clear();
    getAllItemsByPlaylistRecursive(compositesTraversed,allSongs,id);
    qDebug() << SB_DEBUG_INFO;

    //	Calculate duration
    Duration duration;
    qDebug() << SB_DEBUG_INFO << allSongs.count();
    for(int i=0;i<allSongs.count();i++)
    {
        qDebug() << SB_DEBUG_INFO << allSongs.at(i) << allSongs.at(i).duration << duration;
        duration+=allSongs.at(i).duration;
    }

    //	Store calculation
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());

    QString q=QString
    (
        "UPDATE "
            "___SB_SCHEMA_NAME___playlist "
        "SET "
            "duration='%1' "
        "WHERE "
            "playlist_id=%2 "
    )
        .arg(duration.toString(Duration::sb_hhmmss_format))
        .arg(id.sb_playlist_id)
    ;
    dal->customize(q);

    qDebug() << SB_DEBUG_INFO << q;

    QSqlQuery query(q,db);
    query.exec();
}

void
DataEntityPlaylist::renamePlaylist(const SBID &id) const
{
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());

    QString q=QString
    (
        "UPDATE "
            "___SB_SCHEMA_NAME___playlist "
        "SET "
            "name='%1' "
        "WHERE "
            "playlist_id=%2 "
    )
        .arg(id.playlistName)
        .arg(id.sb_playlist_id)
    ;
    dal->customize(q);

    qDebug() << SB_DEBUG_INFO << q;

    QSqlQuery query(q,db);
    query.exec();
}

void
DataEntityPlaylist::reorderItem(const SBID &playlistID, const SBID &fID, int row) const
{
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
    QString q;
    SBID fromID=fID;

    qDebug() << SB_DEBUG_INFO << "from"
        << "itm" << fromID.sb_item_id()
        << "typ" << fromID.sb_item_type()
        << "pfr" << fromID.sb_performer_id
        << "sng" << fromID.sb_song_id
        << "alb" << fromID.sb_album_id
        << "alp" << fromID.sb_position
        << "pll" << fromID.sb_playlist_id;
    qDebug() << SB_DEBUG_INFO << "row" << row;

    //	-1.	Discard plan
    q="DISCARD PLAN";
    QSqlQuery discardPlan(q,db);
    discardPlan.next();

    //	0.	Make sure ordering is sane
    reorderPlaylistPositions(playlistID);

    //	1.	Find max position in current playlist
    q=QString
    (
        "SELECT "
            "a.playlist_position "
        "FROM "
        "( "
            "SELECT "
                "MAX(pp.playlist_position) AS playlist_position "
            "FROM "
                "___SB_SCHEMA_NAME___playlist_performance pp "
            //"WHERE "
                //"pp.playlist_id=%1 "
            "UNION "
            "SELECT "
                "MAX(pc.playlist_position) "
            "FROM "
                "___SB_SCHEMA_NAME___playlist_composite pc "
            //"WHERE "
                //"pc.playlist_id=%1 "
        ") a "
        "ORDER BY 1 DESC "
        "LIMIT 1"
    )
        .arg(playlistID.sb_playlist_id)
    ;
    dal->customize(q);

    qDebug() << SB_DEBUG_INFO << q;

    QSqlQuery maxPosition(q,db);
    maxPosition.next();
    int tmpPosition=maxPosition.value(0).toInt();
    qDebug() << SB_DEBUG_INFO << "tmpPosition=" << tmpPosition;
    tmpPosition+=10;
    qDebug() << SB_DEBUG_INFO << "tmpPosition=" << tmpPosition;


    //	2.	Assign tmpPosition to fromID
    q=QString
    (
        "UPDATE "
            "___SB_SCHEMA_NAME___playlist_performance "
        "SET "
            "playlist_position=%1 "
        "WHERE "
            "playlist_id=%2 AND "
            "artist_id=%3 AND "
            "song_id=%4 AND "
            "record_id=%5 AND "
            "record_position=%6 "
    )
        .arg(tmpPosition)
        .arg(playlistID.sb_playlist_id)
        .arg(fromID.sb_performer_id)
        .arg(fromID.sb_song_id)
        .arg(fromID.sb_album_id)
        .arg(fromID.sb_position)
    ;
    dal->customize(q);

    qDebug() << SB_DEBUG_INFO << q;
    QSqlQuery assignMin1Position(q,db);
    assignMin1Position.next();

    q=QString
    (
        "UPDATE "
            "___SB_SCHEMA_NAME___playlist_composite "
        "SET "
            "playlist_position=%1 "
        "WHERE "
            "playlist_id=%2 AND "
            "( "
                "playlist_playlist_id=%3 OR "
                "playlist_chart_id=%3 OR "
                "playlist_record_id=%3 OR "
                "playlist_artist_id=%3 "
            ") "
    )
        .arg(tmpPosition)
        .arg(playlistID.sb_playlist_id)
        .arg(fromID.sb_item_id());	//	legitimate use of sb_item_id()!
    dal->customize(q);

    qDebug() << SB_DEBUG_INFO << q;
    QSqlQuery assignMin1Composite(q,db);
    assignMin1Composite.next();

    //	3.	Reorder with fromID 'gone'
    reorderPlaylistPositions(playlistID,tmpPosition);

    int newPosition=row;
    qDebug() << SB_DEBUG_INFO << "newPosition=" << newPosition;

    //	5.	Add 1 to all position from toID onwards
    q=QString
    (
        "UPDATE "
            "___SB_SCHEMA_NAME___playlist_performance "
        "SET "
            "playlist_position=playlist_position+1 "
        "WHERE "
            "playlist_id=%1 AND "
            "playlist_position>=%2 AND "
            "playlist_position<%3 "
    )
        .arg(playlistID.sb_playlist_id)
        .arg(newPosition)
        .arg(tmpPosition);
    dal->customize(q);

    qDebug() << SB_DEBUG_INFO << q;
    QSqlQuery updateToPositionPerformance(q,db);
    updateToPositionPerformance.next();

    q=QString
    (
        "UPDATE "
            "___SB_SCHEMA_NAME___playlist_composite "
        "SET "
            "playlist_position=playlist_position+1 "
        "WHERE "
            "playlist_id=%1 AND "
            "playlist_position>=%2 AND "
            "playlist_position<%3 "
    )
        .arg(playlistID.sb_playlist_id)
        .arg(newPosition)
        .arg(tmpPosition);
    dal->customize(q);

    qDebug() << SB_DEBUG_INFO << q;
    QSqlQuery updateToPositionComposite(q,db);
    updateToPositionComposite.next();

    //	6.	Reassign position to fromID
    q=QString
    (
        "UPDATE "
            "___SB_SCHEMA_NAME___playlist_performance "
        "SET "
            "playlist_position=%1 "
        "WHERE "
            "playlist_id=%2 AND "
            "playlist_position=%3 "
    )
        .arg(newPosition)
        .arg(playlistID.sb_playlist_id)
        .arg(tmpPosition);
    dal->customize(q);

    qDebug() << SB_DEBUG_INFO << q;
    QSqlQuery updateToNewPositionPerformance(q,db);
    updateToNewPositionPerformance.next();

    q=QString
    (
        "UPDATE "
            "___SB_SCHEMA_NAME___playlist_composite "
        "SET "
            "playlist_position=%1 "
        "WHERE "
            "playlist_id=%2 AND "
            "playlist_position=%3 "
    )
        .arg(newPosition)
        .arg(playlistID.sb_playlist_id)
        .arg(tmpPosition);
    dal->customize(q);

    qDebug() << SB_DEBUG_INFO << q;
    QSqlQuery updateToNewPositionComposite(q,db);
    updateToNewPositionComposite.next();
}

void
DataEntityPlaylist::reorderItem(const SBID &playlistID, const SBID &fID, const SBID &tID) const
{
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
    QString q;
    SBID fromID=fID;
    SBID toID=tID;

    qDebug() << SB_DEBUG_INFO << "from"
        << "itm" << fromID.sb_item_id()
        << "typ" << fromID.sb_item_type()
        << "pfr" << fromID.sb_performer_id
        << "sng" << fromID.sb_song_id
        << "alb" << fromID.sb_album_id
        << "pos" << fromID.sb_position
        << "pll" << fromID.sb_playlist_id
        << "plp" << fromID.sb_playlist_position
    ;
    qDebug() << SB_DEBUG_INFO << "to"
        << "itm" << toID.sb_item_id()
        << "typ" << toID.sb_item_type()
        << "pfr" << toID.sb_performer_id
        << "sng" << toID.sb_song_id
        << "alb" << toID.sb_album_id
        << "pos" << toID.sb_position
        << "pll" << toID.sb_playlist_id
        << "plp" << toID.sb_playlist_position
    ;

    //	-1.	Discard plan
    q="DISCARD PLAN";
    QSqlQuery discardPlan(q,db);
    discardPlan.next();

    //	0.	Make sure ordering is sane
    reorderPlaylistPositions(playlistID);

    //	1.	Find max position in current playlist
    q=QString
    (
        "SELECT "
            "a.playlist_position "
        "FROM "
        "( "
            "SELECT "
                "MAX(pp.playlist_position) AS playlist_position "
            "FROM "
                "___SB_SCHEMA_NAME___playlist_performance pp "
            //"WHERE "
                //"pp.playlist_id=%1 "
            "UNION "
            "SELECT "
                "MAX(pc.playlist_position) "
            "FROM "
                "___SB_SCHEMA_NAME___playlist_composite pc "
            //"WHERE "
                //"pc.playlist_id=%1 "
        ") a "
        "ORDER BY 1 DESC "
        "LIMIT 1"
    )
        .arg(playlistID.sb_playlist_id)
    ;
    dal->customize(q);

    qDebug() << SB_DEBUG_INFO << q;

    QSqlQuery maxPosition(q,db);
    maxPosition.next();
    int tmpPosition=maxPosition.value(0).toInt();
    qDebug() << SB_DEBUG_INFO << "tmpPosition=" << tmpPosition;
    tmpPosition+=10;
    qDebug() << SB_DEBUG_INFO << "tmpPosition=" << tmpPosition;


    //	2.	Assign tmpPosition to fromID
    q=QString
    (
        "UPDATE "
            "___SB_SCHEMA_NAME___playlist_performance "
        "SET "
            "playlist_position=%1 "
        "WHERE "
            "playlist_id=%2 AND "
            "artist_id=%3 AND "
            "song_id=%4 AND "
            "record_id=%5 AND "
            "record_position=%6 "
    )
        .arg(tmpPosition)
        .arg(playlistID.sb_playlist_id)
        .arg(fromID.sb_performer_id)
        .arg(fromID.sb_song_id)
        .arg(fromID.sb_album_id)
        .arg(fromID.sb_position)
    ;
    dal->customize(q);

    qDebug() << SB_DEBUG_INFO << q;
    QSqlQuery assignMin1Position(q,db);
    assignMin1Position.next();

    q=QString
    (
        "UPDATE "
            "___SB_SCHEMA_NAME___playlist_composite "
        "SET "
            "playlist_position=%1 "
        "WHERE "
            "playlist_id=%2 AND "
            "( "
                "playlist_playlist_id=%3 OR "
                "playlist_chart_id=%3 OR "
                "playlist_record_id=%3 OR "
                "playlist_artist_id=%3 "
            ") "
    )
        .arg(tmpPosition)
        .arg(playlistID.sb_playlist_id)
        .arg(fromID.sb_item_id());	//	legitimate use of sb_item_id()!
    dal->customize(q);

    qDebug() << SB_DEBUG_INFO << q;
    QSqlQuery assignMin1Composite(q,db);
    assignMin1Composite.next();

    //	3.	Reorder with fromID 'gone'
    reorderPlaylistPositions(playlistID,tmpPosition);

    //	4.	Get position of toID
    q=QString
    (
        "SELECT "
            "MAX(playlist_position) "
        "FROM "
        "( "
            "SELECT "
                "MAX(playlist_position) AS playlist_position "
            "FROM "
                "___SB_SCHEMA_NAME___playlist_performance p "
            "WHERE "
                "playlist_id=%1 AND "
                "artist_id=%2 AND "
                "song_id=%3 AND "
                "record_id=%4 AND "
                "record_position=%5 "
            "UNION "
            "SELECT "
                "MAX(playlist_position) AS playlist_position "
            "FROM "
                "___SB_SCHEMA_NAME___playlist_composite p "
            "WHERE "
                "playlist_id=%1 AND "
                "( "
                    "playlist_playlist_id=%6 OR "
                    "playlist_chart_id=%6 OR "
                    "playlist_record_id=%6 OR "
                    "playlist_artist_id=%6  "
                ") "
        ") b "
    )
        .arg(playlistID.sb_playlist_id)
        .arg(toID.sb_performer_id)
        .arg(toID.sb_song_id)
        .arg(toID.sb_album_id)
        .arg(toID.sb_position)
        .arg(toID.sb_item_id());
    dal->customize(q);

    qDebug() << SB_DEBUG_INFO << q;

    QSqlQuery getPosition(q,db);
    getPosition.next();
    int newPosition=getPosition.value(0).toInt();
    qDebug() << SB_DEBUG_INFO << "newPosition=" << newPosition;

    //	5.	Add 1 to all position from toID onwards
    q=QString
    (
        "UPDATE "
            "___SB_SCHEMA_NAME___playlist_performance "
        "SET "
            "playlist_position=playlist_position+1 "
        "WHERE "
            "playlist_id=%1 AND "
            "playlist_position>=%2 AND "
            "playlist_position<%3 "
    )
        .arg(playlistID.sb_playlist_id)
        .arg(newPosition)
        .arg(tmpPosition);
    dal->customize(q);

    qDebug() << SB_DEBUG_INFO << q;
    QSqlQuery updateToPositionPerformance(q,db);
    updateToPositionPerformance.next();

    q=QString
    (
        "UPDATE "
            "___SB_SCHEMA_NAME___playlist_composite "
        "SET "
            "playlist_position=playlist_position+1 "
        "WHERE "
            "playlist_id=%1 AND "
            "playlist_position>=%2 AND "
            "playlist_position<%3 "
    )
        .arg(playlistID.sb_playlist_id)
        .arg(newPosition)
        .arg(tmpPosition);
    dal->customize(q);

    qDebug() << SB_DEBUG_INFO << q;
    QSqlQuery updateToPositionComposite(q,db);
    updateToPositionComposite.next();

    //	6.	Reassign position to fromID
    q=QString
    (
        "UPDATE "
            "___SB_SCHEMA_NAME___playlist_performance "
        "SET "
            "playlist_position=%1 "
        "WHERE "
            "playlist_id=%2 AND "
            "playlist_position=%3 "
    )
        .arg(newPosition)
        .arg(playlistID.sb_playlist_id)
        .arg(tmpPosition);
    dal->customize(q);

    qDebug() << SB_DEBUG_INFO << q;
    QSqlQuery updateToNewPositionPerformance(q,db);
    updateToNewPositionPerformance.next();

    q=QString
    (
        "UPDATE "
            "___SB_SCHEMA_NAME___playlist_composite "
        "SET "
            "playlist_position=%1 "
        "WHERE "
            "playlist_id=%2 AND "
            "playlist_position=%3 "
    )
        .arg(newPosition)
        .arg(playlistID.sb_playlist_id)
        .arg(tmpPosition);
    dal->customize(q);

    qDebug() << SB_DEBUG_INFO << q;
    QSqlQuery updateToNewPositionComposite(q,db);
    updateToNewPositionComposite.next();
}


///	PRIVATE METHODS
void
DataEntityPlaylist::reorderPlaylistPositions(const SBID &id,int maxPosition) const
{
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());

    qDebug() << SB_DEBUG_INFO << id << id.playPosition << maxPosition;

    QString q=QString
    (
        "SELECT "
            "playlist_position "
        "FROM "
        "( "
            "SELECT "
                "playlist_id, "
                "playlist_position "
            "FROM "
                "___SB_SCHEMA_NAME___playlist_performance "
            "WHERE "
                "playlist_id=%1 "
            "UNION "
            "SELECT "
                "playlist_id, "
                "playlist_position "
            "FROM "
                "___SB_SCHEMA_NAME___playlist_composite "
            "WHERE "
                "playlist_id=%1 "
        ") a "
        "WHERE "
            "playlist_position>=0 AND "
            "playlist_position<%2 "
        "ORDER BY "
            "1 "
    )
        .arg(id.sb_playlist_id)
        .arg(maxPosition)
    ;
    dal->customize(q);

    QSqlQuery query(q,db);
    int newPosition=1;
    int actualPosition;

    while(query.next())
    {
        actualPosition=query.value(0).toInt();
        if(actualPosition!=newPosition)
        {
            //	Update playlist_performance
            q=QString
            (
                "UPDATE "
                    "___SB_SCHEMA_NAME___playlist_performance "
                "SET "
                    "playlist_position=%3 "
                "WHERE "
                    "playlist_id=%1 AND playlist_position=%2"
            )
                .arg(id.sb_playlist_id)
                .arg(actualPosition)
                .arg(newPosition)
            ;

            dal->customize(q);

            QSqlQuery update1(q,db);
            Q_UNUSED(update1);

            //	Update playlist_composite
            q=QString
            (
                "UPDATE "
                    "___SB_SCHEMA_NAME___playlist_composite "
                "SET "
                    "playlist_position=%3 "
                "WHERE "
                    "playlist_id=%1 AND playlist_position=%2"
            )
                .arg(id.sb_playlist_id)
                .arg(actualPosition)
                .arg(newPosition)
            ;

            dal->customize(q);

            QSqlQuery update2(q,db);
            Q_UNUSED(update2);

        }
        newPosition++;
    }
}

void
DataEntityPlaylist::init()
{
    qDebug() << SB_DEBUG_INFO;
}

