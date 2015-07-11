#include "BackgroundThread.h"
#include "Common.h"
#include "Context.h"
#include "Controller.h"
#include "DataAccessLayer.h"
#include "SBID.h"
#include "SBSqlQueryModel.h"
#include "SBModelPlaylist.h"

///	PUBLIC METHODS
SBModelPlaylist::SBModelPlaylist()
{
    qDebug() << SB_DEBUG_INFO << "ctor";
    _init();
}

SBModelPlaylist::~SBModelPlaylist()
{
    qDebug() << SB_DEBUG_INFO << "dtor";
}

void
SBModelPlaylist::assignItem(const SBID &assignID, const SBID &toID)
{
    qDebug() << SB_DEBUG_INFO << "assign" << assignID << assignID.sb_album_id << assignID.sb_position;
    qDebug() << SB_DEBUG_INFO << "to" << toID;
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
    QString q;

    switch(assignID.sb_item_type)
    {
    case SBID::sb_type_song:
        qDebug() << SB_DEBUG_INFO;
        if(assignID.sb_album_id==0 || assignID.sb_position==0)
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
          ).arg(toID.sb_item_id)
           .arg(assignID.sb_item_id)
           .arg(assignID.sb_performer_id)
           .arg(assignID.sb_album_id)
           .arg(assignID.sb_position)
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
          ).arg(toID.sb_item_id)
           .arg(dal->getGetDate())
           .arg(dal->getIsNull())
           .arg(assignID.sb_item_id);
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
          ).arg(toID.sb_item_id)
           .arg(dal->getGetDate())
           .arg(dal->getIsNull())
           .arg(assignID.sb_item_id);
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
          ).arg(toID.sb_item_id)
           .arg(dal->getGetDate())
           .arg(dal->getIsNull())
           .arg(assignID.sb_item_id);
        break;

    case SBID::sb_type_allsongs:
    case SBID::sb_type_songsearch:
    case SBID::sb_type_invalid:
    default:
        qDebug() << SB_DEBUG_INFO;
        break;
    }

    dal->customize(q);
    if(q.length()>0)
    {
        QSqlQuery insert(q,db);
        Q_UNUSED(insert);
        qDebug() << SB_DEBUG_INFO << q;
        _calculatePlaylistDuration(toID);
        qDebug() << SB_DEBUG_INFO;
    }
    qDebug() << SB_DEBUG_INFO;
}

SBID
SBModelPlaylist::createNewPlaylist()
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

    result.sb_item_type   =SBID::sb_type_playlist;
    result.sb_item_id     =qID.value(0).toInt();

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
            .arg(result.sb_item_id)
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
SBModelPlaylist::deleteItem(const SBID &assignID, const SBID &fromID)
{
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
    QString q;

    switch(assignID.sb_item_type)
    {
    case SBID::sb_type_song:
        q=QString
          (
            "DELETE FROM ___SB_SCHEMA_NAME___playlist_performance "
            "WHERE "
                "playlist_id=%1 AND "
                "playlist_position=%2 "
          ).arg(fromID.sb_item_id)
           .arg(assignID.sb_position);
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
          ).arg(fromID.sb_item_id)
           .arg(assignID.sb_position);
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
        _reorderPlaylistPositions(fromID);
        _calculatePlaylistDuration(fromID);
    }
}

void
SBModelPlaylist::deletePlaylist(const SBID &id)
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
        QString q=qTemplate.arg(l.at(i)).arg(id.sb_item_id);
        dal->customize(q);
        qDebug() << SB_DEBUG_INFO << q;
        QSqlQuery toExec(q,db);
        Q_UNUSED(toExec);
    }
}

SBID
SBModelPlaylist::getDetail(const SBID& id)
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
    ).arg(id.sb_item_id);
    dal->customize(q);

    qDebug() << SB_DEBUG_INFO << q;

    QSqlQuery query(q,db);
    query.next();

    result.sb_item_type   =SBID::sb_type_playlist;
    result.sb_item_id     =id.sb_item_id;
    result.sb_playlist_id=id.sb_item_id;
    result.playlistName   =query.value(0).toString();
    result.duration       =query.value(1).toTime();
    result.count1         =query.value(2).toInt();

    return result;
}

SBSqlQueryModel*
SBModelPlaylist::getAllItemsByPlaylist(const SBID& id)
{
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
            "0 AS SB_PERFORMER_ID "
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
    )
            .arg(id.sb_item_id)            //	1
            .arg(SBID::sb_type_playlist)
            .arg(SBID::sb_type_chart)
            .arg(SBID::sb_type_album)
            .arg(SBID::sb_type_performer)  //	5
            .arg(SBID::sb_type_song)
            .arg(SBID::sb_type_position);

    return new SBSqlQueryModel(q);
}

void
SBModelPlaylist::getAllItemsByPlaylistRecursive(QHash<int,int>& compositesTraversed,QList<SBID>& allSongs,const SBID &id)
{
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
    QString q;

    qDebug() << SB_DEBUG_INFO << "calculate for" << id;

    if(compositesTraversed.contains(id.sb_item_id)==0)
    {
        compositesTraversed.insert(id.sb_item_id,1);
        switch(id.sb_item_type)
        {
            case SBID::sb_type_playlist:
                //	Calculate duration of all items in playlist_composite
                q=QString
                    (
                        "SELECT "
                            "%1(playlist_playlist_id,0), "
                            "%1(playlist_chart_id,0), "
                            "%1(playlist_record_id,0), "
                            "%1(playlist_artist_id,0) "
                        "FROM "
                            "___SB_SCHEMA_NAME___playlist_composite pc "
                        "WHERE "
                            "pc.playlist_id=%2 "
                    )
                        .arg(dal->getIsNull())
                        .arg(id.sb_item_id);
                dal->customize(q);
                qDebug() << SB_DEBUG_INFO << q;
                //	Declaring variables in compound statement to avoid compiler errors
                {
                    QSqlQuery queryComposite(q,db);

                    while(queryComposite.next())
                    {
                        int playlistID=queryComposite.value(0).toInt();
                        int chartID=queryComposite.value(1).toInt();
                        int albumID=queryComposite.value(2).toInt();
                        int performerID=queryComposite.value(3).toInt();

                        SBID t;
                        if(playlistID!=0)
                        {
                            qDebug() << SB_DEBUG_INFO;
                            t.sb_item_type=SBID::sb_type_playlist;
                            t.sb_item_id=playlistID;
                        }
                        else if(chartID!=0)
                        {
                            qDebug() << SB_DEBUG_INFO;
                            t.sb_item_type=SBID::sb_type_chart;
                            t.sb_item_id=chartID;
                        }
                        else if(albumID!=0)
                        {
                            qDebug() << SB_DEBUG_INFO;
                            t.sb_item_type=SBID::sb_type_album;
                            t.sb_item_id=albumID;
                        }
                        else if(performerID!=0)
                        {
                            qDebug() << SB_DEBUG_INFO;
                            t.sb_item_type=SBID::sb_type_performer;
                            t.sb_item_id=performerID;
                        }
                        qDebug() << SB_DEBUG_INFO << t;

                        getAllItemsByPlaylistRecursive(compositesTraversed,allSongs,t);
                    }
                }

                //	Calculate duration of all songs in playlist_performance
                q=QString
                    (
                        "SELECT "
                            "pp.artist_id, "
                            "pp.song_id, "
                            "pp.record_id, "
                            "pp.record_position, "
                            "rp.duration "
                        "FROM "
                            "___SB_SCHEMA_NAME___playlist_performance pp "
                                "JOIN ___SB_SCHEMA_NAME___record_performance rp ON "
                                    "pp.artist_id=rp.artist_id AND "
                                    "pp.song_id=rp.song_id AND "
                                    "pp.record_id=rp.record_id AND "
                                    "pp.record_position=rp.record_position "
                        "WHERE "
                            "pp.playlist_id=%2"
                    )
                        .arg(id.sb_item_id);
                break;

            case SBID::sb_type_chart:
                q=QString
                    (
                        "SELECT "
                            "pp.artist_id, "
                            "pp.song_id, "
                            "pp.record_id, "
                            "pp.record_position, "
                            "rp.duration "
                        "FROM "
                            "___SB_SCHEMA_NAME___chart_performance pp "
                                "JOIN ___SB_SCHEMA_NAME___record_performance rp ON "
                                    "pp.artist_id=rp.artist_id AND "
                                    "pp.song_id=rp.song_id AND "
                                    "pp.record_id=rp.record_id AND "
                                    "pp.record_position=rp.record_position "
                        "WHERE "
                            "pp.chart_id=%2"
                    )
                        .arg(id.sb_item_id);
                break;

            case SBID::sb_type_album:
                q=QString
                    (
                        "SELECT "
                            "rp.artist_id, "
                            "rp.song_id, "
                            "rp.record_id, "
                            "rp.record_position, "
                            "rp.duration "
                        "FROM "
                            "___SB_SCHEMA_NAME___record_performance rp "
                        "WHERE "
                            "rp.record_id=%2"
                    )
                        .arg(id.sb_item_id);
                break;

            case SBID::sb_type_performer:
                q=QString
                    (
                        "SELECT "
                            "rp.artist_id, "
                            "rp.song_id, "
                            "rp.record_id, "
                            "rp.record_position, "
                            "rp.duration "
                        "FROM "
                            "___SB_SCHEMA_NAME___record_performance rp "
                        "WHERE "
                            "rp.artist_id=%2"
                    )
                        .arg(id.sb_item_id);
                break;

            case SBID::sb_type_invalid:
            case SBID::sb_type_song:
            case SBID::sb_type_position:
            case SBID::sb_type_allsongs:
            case SBID::sb_type_songsearch:
                break;
        }
    }
    if(q.length())
    {
        dal->customize(q);
        qDebug() << SB_DEBUG_INFO << q;
        QSqlQuery querySong(q,db);
        while(querySong.next())
        {
            SBID songID;
            songID.sb_item_type=SBID::sb_type_song;
            songID.sb_item_id=querySong.value(1).toInt();
            songID.sb_performer_id=querySong.value(0).toInt();
            songID.sb_album_id=querySong.value(2).toInt();
            songID.sb_position=querySong.value(3).toInt();
            songID.duration=querySong.value(4).toTime();

            if(allSongs.contains(songID)==0)
            {
                allSongs.append(songID);
            }
        }
    }
}

SBSqlQueryModel*
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

    return new SBSqlQueryModel(q);
}

void
SBModelPlaylist::recalculateAllPlaylistDurations()
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
        SBID t;
        t.sb_item_type=SBID::sb_type_playlist;
        t.sb_item_id=query.value(1).toInt();

        _calculatePlaylistDuration(t);
    }
}

void
SBModelPlaylist::renamePlaylist(const SBID &id)
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
    ).arg(id.playlistName).arg(id.sb_item_id);
    dal->customize(q);

    qDebug() << SB_DEBUG_INFO << q;

    QSqlQuery query(q,db);
    query.exec();
}

///	PRIVATE METHODS
void
SBModelPlaylist::_reorderPlaylistPositions(const SBID &id)
{
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());

    qDebug() << SB_DEBUG_INFO << id;

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
        "ORDER BY 1 "
    ).arg(id.sb_item_id);
    dal->customize(q);

    qDebug() << SB_DEBUG_INFO << q;

    QSqlQuery query(q,db);
    int newPosition=1;
    int actualPosition;

    while(query.next())
    {
        actualPosition=query.value(0).toInt();
        qDebug() << SB_DEBUG_INFO << "actual=" << actualPosition << ":new=" << newPosition;
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
            .arg(id.sb_item_id)
            .arg(actualPosition)
            .arg(newPosition);

            dal->customize(q);

            qDebug() << SB_DEBUG_INFO << q;

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
            .arg(id.sb_item_id)
            .arg(actualPosition)
            .arg(newPosition);

            dal->customize(q);

            qDebug() << SB_DEBUG_INFO << q;

            QSqlQuery update2(q,db);
            Q_UNUSED(update2);

        }
        newPosition++;
    }
}

void
SBModelPlaylist::_init()
{
    qDebug() << SB_DEBUG_INFO;
}

void
SBModelPlaylist::_calculatePlaylistDuration(const SBID &id)
{
    QHash<int,int> compositesTraversed;
    QList<SBID> allSongs;

    //	Get all songs
    qDebug() << SB_DEBUG_INFO;
    compositesTraversed.clear();
    allSongs.clear();
    getAllItemsByPlaylistRecursive(compositesTraversed,allSongs,id);
    qDebug() << SB_DEBUG_INFO;

    //	Calculate duration
    SBTime duration;
    qDebug() << SB_DEBUG_INFO << allSongs.count();
    for(int i=0;i<allSongs.count();i++)
    {
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
            .arg(duration.toString("hh:mm:ss"))
            .arg(id.sb_item_id);
    dal->customize(q);

    qDebug() << SB_DEBUG_INFO << q;

    QSqlQuery query(q,db);
    query.exec();
}
