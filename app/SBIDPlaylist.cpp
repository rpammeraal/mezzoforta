#include "SBIDPlaylist.h"

#include "Context.h"
#include "SBMessageBox.h"
#include "SBModelQueuedSongs.h"
#include "SBSqlQueryModel.h"

SBIDPlaylist::SBIDPlaylist():SBIDBase()
{
    _init();
}

SBIDPlaylist::SBIDPlaylist(const SBIDPlaylist &c):SBIDBase(c)
{
}

SBIDPlaylist::SBIDPlaylist(const SBIDBase &c):SBIDBase(c)
{
    _sb_item_type=SBIDBase::sb_type_playlist;
}

SBIDPlaylist::SBIDPlaylist(int itemID):SBIDBase()
{
    _init();
    _sb_playlist_id=itemID;
}

SBIDPlaylist::~SBIDPlaylist()
{
}

///	Public methods
SBSqlQueryModel*
SBIDPlaylist::findMatches(const QString& name) const
{
    Q_UNUSED(name);
    qDebug() << SB_DEBUG_ERROR << "NOT IMPLEMENTED!";
    return NULL;
}

int
SBIDPlaylist::getDetail(bool createIfNotExistFlag)
{
    Q_UNUSED(createIfNotExistFlag);
    SBIDPlaylist result;
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
    ).arg(this->playlistID());
    dal->customize(q);
    qDebug() << SB_DEBUG_INFO << q;

    QSqlQuery query(q,db);

    if(query.next())
    {
        this->setPlaylistName(query.value(0).toString());
        this->setDuration(query.value(1).toString());
        this->setCount1(query.value(2).toInt());

        return 1;
    }
    return 0;
}

QString
SBIDPlaylist::genericDescription() const
{
    return "Playlist - " + this->text();
}

QString
SBIDPlaylist::hash() const
{
    return QString("%1:%2").arg(itemType()).arg(this->playlistID());
}

QString
SBIDPlaylist::iconResourceLocation() const
{
    return ":/images/PlaylistIcon.png";
}

int
SBIDPlaylist::itemID() const
{
    return _sb_playlist_id;
}

SBIDBase::sb_type
SBIDPlaylist::itemType() const
{
    return SBIDBase::sb_type_playlist;
}

bool
SBIDPlaylist::save()
{
    qDebug() << SB_DEBUG_ERROR << "NOT IMPLEMENTED!";
    return 0;
}

void
SBIDPlaylist::sendToPlayQueue(bool enqueueFlag)
{
    QMap<int,SBIDBase> list;
    list=_retrievePlaylistItems();

    SBModelQueuedSongs* mqs=Context::instance()->getSBModelQueuedSongs();
    mqs->populate(list,enqueueFlag);
}

void
SBIDPlaylist::setText(const QString &text)
{
    _playlistName=text;
}

QString
SBIDPlaylist::text() const
{
    return this->_playlistName;
}

QString
SBIDPlaylist::type() const
{
    return "playlist";
}

//	Methods specific to SBIDPlaylist

void
SBIDPlaylist::assignPlaylistItem(const SBIDBase &toBeAssignedID) const
{
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
    QString q;

    switch(toBeAssignedID.itemType())
    {
    case SBIDBase::sb_type_song:
        if(toBeAssignedID.albumID()==-1 || toBeAssignedID.albumPosition()==-1)
        {
            qDebug() << SB_DEBUG_ERROR << "assignment of song without album";
            qDebug() << SB_DEBUG_ERROR << toBeAssignedID;
            qDebug() << SB_DEBUG_ERROR << toBeAssignedID.albumID();
            qDebug() << SB_DEBUG_ERROR << toBeAssignedID.albumPosition();

            SBMessageBox::createSBMessageBox("Error: you should never see this...",
                "Assignment of song without album",
                QMessageBox::Warning,
                QMessageBox::Ok,
                QMessageBox::Ok,
                QMessageBox::Ok,
                0);

            return;
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
        )
            .arg(this->playlistID())
            .arg(toBeAssignedID.songID())
            .arg(toBeAssignedID.commonPerformerID())
            .arg(toBeAssignedID.albumID())
            .arg(toBeAssignedID.albumPosition())
            .arg(dal->getGetDate())
            .arg(dal->getIsNull());
        break;

    case SBIDBase::sb_type_performer:
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
          ).arg(this->playlistID())
           .arg(dal->getGetDate())
           .arg(dal->getIsNull())
           .arg(toBeAssignedID.commonPerformerID())
        ;
        break;

    case SBIDBase::sb_type_album:
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
          ).arg(this->playlistID())
           .arg(dal->getGetDate())
           .arg(dal->getIsNull())
           .arg(toBeAssignedID.albumID())
        ;
        break;

    case SBIDBase::sb_type_chart:
        break;

    case SBIDBase::sb_type_playlist:
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
          ).arg(this->playlistID())
           .arg(dal->getGetDate())
           .arg(dal->getIsNull())
           .arg(toBeAssignedID.playlistID())
        ;
        break;

    case SBIDBase::sb_type_invalid:
        break;
    }

    dal->customize(q);
    if(q.length()>0)
    {
        QSqlQuery insert(q,db);
        Q_UNUSED(insert);
        qDebug() << SB_DEBUG_INFO << q;
        recalculatePlaylistDuration(std::make_shared<SBIDBase>(*this));
    }
}

SBIDPlaylist
SBIDPlaylist::createNewPlaylistDB()
{
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
    SBIDPlaylist result;
    QString q;

    //	Get next ID available
    q=QString("SELECT %1(MAX(playlist_id),0)+1 FROM ___SB_SCHEMA_NAME___playlist ").arg(dal->getIsNull());
    dal->customize(q);
    qDebug() << SB_DEBUG_INFO << q;
    QSqlQuery qID(q,db);
    qID.next();

    result=SBIDPlaylist(qID.value(0).toInt());

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
        existing.replace("New Playlist","");
        int i=existing.toInt();
        if(i>=maxNum)
        {
            maxNum=i+1;
        }
    }
    result.setPlaylistName(QString("New Playlist%1").arg(maxNum));

    //	Insert
    q=QString("INSERT INTO ___SB_SCHEMA_NAME___playlist (playlist_id, name,created,play_mode) VALUES(%1,'%2',%3,1)")
            .arg(result.playlistID())
            .arg(result.playlistName())
            .arg(dal->getGetDate());
    dal->customize(q);
    qDebug() << SB_DEBUG_INFO << q;
    QSqlQuery insert(q,db);
    Q_UNUSED(insert);
    //	insert.exec();	-- no need to run on insert statements

    return result;
}

void
SBIDPlaylist::deletePlaylist()
{
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
    QString qTemplate=QString("DELETE FROM ___SB_SCHEMA_NAME___%1 WHERE playlist_id=%2");

    QStringList l;
    l.append("playlist_performance");
    l.append("playlist_composite");
    l.append("playlist");

    for(int i=0;i<l.count();i++)
    {
        QString q=qTemplate.arg(l.at(i)).arg(this->playlistID());
        dal->customize(q);
        qDebug() << SB_DEBUG_INFO << q;
        QSqlQuery toExec(q,db);
        Q_UNUSED(toExec);
    }
}

void
SBIDPlaylist::deletePlaylistItem(SBIDBase::sb_type itemType,int playlistPosition) const
{
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
    QString q;

    switch(itemType)
    {
    case SBIDBase::sb_type_song:
        q=QString
          (
            "DELETE FROM ___SB_SCHEMA_NAME___playlist_performance "
            "WHERE "
                "playlist_id=%1 AND "
                "playlist_position=%2 "
          ).arg(this->playlistID())
           .arg(playlistPosition);
        break;

    case SBIDBase::sb_type_performer:
    case SBIDBase::sb_type_album:
    case SBIDBase::sb_type_chart:
    case SBIDBase::sb_type_playlist:
        q=QString
          (
            "DELETE FROM ___SB_SCHEMA_NAME___playlist_composite "
            "WHERE "
                "playlist_id=%1 AND "
                "playlist_position=%2 "
          ).arg(this->playlistID())
           .arg(playlistPosition)
        ;
        break;

    case SBIDBase::sb_type_invalid:
    default:
        break;
    }

    dal->customize(q);
    if(q.length()>0)
    {
        qDebug() << SB_DEBUG_INFO << q;
        QSqlQuery remove(q,db);
        Q_UNUSED(remove);
        SBIDPlaylist playlist(this->playlistID());
        playlist._reorderPlaylistPositions();
        recalculatePlaylistDuration(std::make_shared<SBIDBase>(playlist));
    }
}

SBSqlQueryModel*
SBIDPlaylist::getAllItemsByPlaylist() const
{
    this->_reorderPlaylistPositions();

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
            "'song - ' || s.title || ' [' || CAST(rp.duration AS VARCHAR) || '] / ' || a.name || ' - ' || r.title, "
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
            .arg(this->playlistID())
            .arg(Common::sb_field_playlist_id)
            .arg(Common::sb_field_chart_id)
            .arg(Common::sb_field_album_id)
            .arg(Common::sb_field_performer_id)
            .arg(Common::sb_field_song_id)
            .arg(Common::sb_field_album_position);

    return new SBSqlQueryModel(q,0);
}

SBSqlQueryModel*
SBIDPlaylist::getAllPlaylists()
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

SBIDSong
SBIDPlaylist::getDetailPlaylistItemSong(int playlistPosition) const
{
    SBIDSong result;
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());

    QString q=QString
    (
        "SELECT DISTINCT "
            "s.song_id, "           //	0
            "a.artist_id, "
            "r.record_id, "
            "rp.record_position, "
            "rp.duration, "
            "s.title, "             //	5
            "a.name, "
            "r.title, "
            "op.path, "
            "pp.playlist_position "
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
        .arg(this->playlistID())
        .arg(playlistPosition)
    ;
    dal->customize(q);

    qDebug() << SB_DEBUG_INFO << q;
    QSqlQuery query(q,db);

    if(query.next())
    {
        result=SBIDSong(query.value(0).toInt());
        result.setSongPerformerID(query.value(1).toInt());
        result.setAlbumID(query.value(2).toInt());
        result.setAlbumPosition(query.value(3).toInt());
        result.setDuration(query.value(4).toTime());
        result.setSongTitle(query.value(5).toString());
        result.setSongPerformerName(query.value(6).toString());
        result.setAlbumTitle(query.value(7).toString());
        result.setPath(query.value(8).toString());
        result.setPlaylistPosition(query.value(9).toInt());	//	CWIP: possibly not needed
    }
    return result;
}

void
SBIDPlaylist::recalculateAllPlaylistDurations()
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
        SBIDPlaylist playlist(query.value(1).toInt());

        recalculatePlaylistDuration(std::make_shared<SBIDBase>(playlist));
    }
}

void
SBIDPlaylist::recalculatePlaylistDuration(const SBIDPtr &ptr)
{
    QList<SBIDBase> compositesTraversed;
    QList<SBIDBase> allSongs;

    //	Get all songs
    compositesTraversed.clear();
    allSongs.clear();
    _getAllItemsByPlaylistRecursive(compositesTraversed,allSongs,ptr);

    //	Calculate duration
    Duration duration;
    for(int i=0;i<allSongs.count();i++)
    {
        duration+=allSongs.at(i).duration();
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
        .arg(duration.toString(Duration::sb_full_hhmmss_format))
        .arg(ptr->playlistID())
    ;
    dal->customize(q);

    QSqlQuery query(q,db);
    query.exec();
}

void
SBIDPlaylist::renamePlaylist()
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
        .arg(this->playlistName())
        .arg(this->playlistID())
    ;
    dal->customize(q);
    qDebug() << SB_DEBUG_INFO << q;

    QSqlQuery query(q,db);
    query.exec();
}

void
SBIDPlaylist::reorderItem(const SBIDBase &fID, const SBIDBase &tID) const
{
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
    QString q;
    SBIDBase fromID=fID;
    SBIDBase toID=tID;

    qDebug() << SB_DEBUG_INFO << "from"
        << "itm" << fromID.itemID()
        << "typ" << fromID.itemType()
        << "pfr" << fromID.commonPerformerID()
        << "sng" << fromID.songID()
        << "alb" << fromID.albumID()
        << "pos" << fromID.albumPosition()
        << "pll" << fromID.playlistID()
        << "plp" << fromID.playlistPosition()
    ;
    qDebug() << SB_DEBUG_INFO << "to"
        << "itm" << toID.itemID()
        << "typ" << toID.itemType()
        << "pfr" << toID.commonPerformerID()
        << "sng" << toID.songID()
        << "alb" << toID.albumID()
        << "pos" << toID.albumPosition()
        << "pll" << toID.playlistID()
        << "plp" << toID.playlistPosition()
    ;

    //	-1.	Discard plan
    q="DISCARD PLAN";
    QSqlQuery discardPlan(q,db);
    discardPlan.next();

    //	0.	Make sure ordering is sane
    _reorderPlaylistPositions();

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
        .arg(this->playlistID())
    ;
    dal->customize(q);

    qDebug() << SB_DEBUG_INFO << q;

    QSqlQuery maxPosition(q,db);
    maxPosition.next();
    int tmpPosition=maxPosition.value(0).toInt();
    tmpPosition+=10;


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
        .arg(this->playlistID())
        .arg(fromID.commonPerformerID())
        .arg(fromID.songID())
        .arg(fromID.albumID())
        .arg(fromID.albumPosition())
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
        .arg(this->playlistID())
        .arg(fromID.itemID());	//	legitimate use of sb_item_id()!
    dal->customize(q);

    qDebug() << SB_DEBUG_INFO << q;
    QSqlQuery assignMin1Composite(q,db);
    assignMin1Composite.next();

    //	3.	Reorder with fromID 'gone'
    _reorderPlaylistPositions(tmpPosition);

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
        .arg(this->playlistID())
        .arg(toID.commonPerformerID())
        .arg(toID.songID())
        .arg(toID.albumID())
        .arg(toID.albumPosition())
        .arg(toID.itemID());
    dal->customize(q);

    qDebug() << SB_DEBUG_INFO << q;

    QSqlQuery getPosition(q,db);
    getPosition.next();
    int newPosition=getPosition.value(0).toInt();

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
        .arg(this->playlistID())
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
        .arg(this->playlistID())
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
        .arg(this->playlistID())
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
        .arg(this->playlistID())
        .arg(tmpPosition);
    dal->customize(q);

    qDebug() << SB_DEBUG_INFO << q;
    QSqlQuery updateToNewPositionComposite(q,db);
    updateToNewPositionComposite.next();
}

void
SBIDPlaylist::reorderItem(const SBIDBase &fID, int row) const
{
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
    QString q;
    SBIDBase fromID=fID;

    //	-1.	Discard plan
    q="DISCARD PLAN";
    QSqlQuery discardPlan(q,db);
    discardPlan.next();

    //	0.	Make sure ordering is sane
    _reorderPlaylistPositions();

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
        .arg(this->playlistID())
    ;
    dal->customize(q);
    qDebug() << SB_DEBUG_INFO << q;

    QSqlQuery maxPosition(q,db);
    maxPosition.next();
    int tmpPosition=maxPosition.value(0).toInt();
    tmpPosition+=10;


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
        .arg(this->playlistID())
        .arg(fromID.commonPerformerID())
        .arg(fromID.songID())
        .arg(fromID.albumID())
        .arg(fromID.albumPosition())
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
        .arg(this->playlistID())
        .arg(fromID.itemID());	//	legitimate use of sb_item_id()!
    dal->customize(q);

    qDebug() << SB_DEBUG_INFO << q;
    QSqlQuery assignMin1Composite(q,db);
    assignMin1Composite.next();

    //	3.	Reorder with fromID 'gone'
    _reorderPlaylistPositions(tmpPosition);

    int newPosition=row;

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
        .arg(this->playlistID())
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
        .arg(this->playlistID())
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
        .arg(this->playlistID())
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
        .arg(this->playlistID())
        .arg(tmpPosition);
    dal->customize(q);

    qDebug() << SB_DEBUG_INFO << q;
    QSqlQuery updateToNewPositionComposite(q,db);
    updateToNewPositionComposite.next();
}


///	Operators
bool
SBIDPlaylist::operator ==(const SBIDBase& i) const
{
    if(
        i._sb_playlist_id==this->_sb_playlist_id
    )
    {
        return 1;
    }
    return 0;
}

SBIDPlaylist::operator QString() const
{
    QString playlistName=this->_playlistName.length() ? this->_playlistName : "<N/A>";
    return QString("SBIDPlaylist:%1,%2:n=%3")
            .arg(this->_sb_playlist_id)
            .arg(this->_sb_tmp_item_id)
            .arg(playlistName)
    ;
}

///	Private methods

void
SBIDPlaylist::_getAllItemsByPlaylistRecursive(QList<SBIDBase>& compositesTraversed,QList<SBIDBase>& allSongs,const SBIDPtr& rootPtr)
{
    SBIDPlaylist playlist;
    if(rootPtr->itemType()==SBIDBase::sb_type_playlist)
    {
        playlist=SBIDPlaylist(rootPtr->itemType());
        playlist._reorderPlaylistPositions();
    }
    else
    {
        return;
    }

    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
    QString q;

    if(compositesTraversed.contains(*rootPtr))
    {
        return;
    }
    compositesTraversed.append(*rootPtr);

    //	If *rootPtr is a playlist, traverse trough all items within this playlist, recurse when neccessary.
    switch(rootPtr->itemType())
    {
    case SBIDBase::sb_type_playlist:
        q=QString
            (
                "SELECT "
                    "0 AS composite_flag, "    //	0
                    "pp.playlist_position, "
                    "0 AS playlist_id, "
                    "0 AS chart_id, "
                    "pp.record_id, "
                    "pp.artist_id, "           //	5
                    "pp.song_id, "
                    "pp.record_position, "
                    "op.path, "
                    "s.title, "
                    "a.name, "                 //	10
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
                .arg(rootPtr->playlistID())
            ;

        dal->customize(q);
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
                    SBIDBase t;
                    SBIDPtr ptr;
                    SBIDBase::sb_type itemType=SBIDBase::sb_type_invalid;
                    int itemID;
                    if(playlistID!=0)
                    {
                        itemType=SBIDBase::sb_type_playlist;
                        itemID=playlistID;
                    }
                    else if(chartID!=0)
                    {
                        itemType=SBIDBase::sb_type_chart;
                        itemID=chartID;
                    }
                    else if(albumID!=0)
                    {
                        itemType=SBIDBase::sb_type_album;
                        itemID=albumID;
                    }
                    else if(performerID!=0)
                    {
                        itemType=SBIDBase::sb_type_performer;
                        itemID=performerID;
                    }
                    if(itemType!=SBIDBase::sb_type_invalid)
                    {
                        ptr=SBIDBase::createPtr(itemType,itemID);
                        _getAllItemsByPlaylistRecursive(compositesTraversed,allSongs,ptr);
                    }
                }
                else
                {
                int songID=allItems.value(6).toInt();
                    SBIDSong song=SBIDSong(songID);
                    song.setAlbumID(albumID);
                    song.setSongPerformerID(performerID);
                    song.setAlbumPosition(allItems.value(7).toInt());
                    song.setPath(allItems.value(8).toString());
                    song.setSongTitle(allItems.value(9).toString());
                    song.setSongPerformerName(allItems.value(10).toString());
                    song.setAlbumTitle(allItems.value(11).toString());
                    song.setPlayPosition(allSongs.count()+1);
                    song.setDuration(allItems.value(12).toTime());
                    song.setPlaylistPosition(playlistPosition);

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

    case SBIDBase::sb_type_chart:
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
                .arg(rootPtr->playlistID())
            ;
        break;

    case SBIDBase::sb_type_album:
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
                .arg(rootPtr->albumID())
            ;
        break;

    case SBIDBase::sb_type_performer:
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
                .arg(rootPtr->commonPerformerID())
            ;
        break;

    case SBIDBase::sb_type_invalid:
    case SBIDBase::sb_type_song:
        break;
    }

    if(q.length())
    {
        dal->customize(q);
        QSqlQuery querySong(q,db);
        while(querySong.next())
        {
            SBIDSong song(querySong.value(1).toInt());
            song.setSongPerformerID(querySong.value(0).toInt());
            song.setAlbumID(querySong.value(2).toInt());
            song.setAlbumPosition(querySong.value(3).toInt());
            song.setDuration(querySong.value(4).toTime());
            song.setSongTitle(querySong.value(5).toString());
            song.setSongPerformerName(querySong.value(6).toString());
            song.setAlbumTitle(querySong.value(7).toString());
            song.setPath(querySong.value(8).toString());
            song.setPlaylistPosition(allSongs.count()+1);

            if(allSongs.contains(song)==0)
            {
                allSongs.append(song);
            }
        }
    }
    return;
}


void
SBIDPlaylist::_init()
{
    _sb_item_type=SBIDBase::sb_type_playlist;
    _sb_playlist_id=-1;
}

void
SBIDPlaylist::_reorderPlaylistPositions(int maxPosition) const
{
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());

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
        .arg(this->playlistID())
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
                .arg(this->playlistID())
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
                .arg(this->playlistID())
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

QMap<int,SBIDBase>
SBIDPlaylist::_retrievePlaylistItems()
{
    QList<SBIDBase> compositesTraversed;
    QList<SBIDBase> allSongs;
    QMap<int,SBIDBase> playList;

    //	Get all songs
    compositesTraversed.clear();
    allSongs.clear();
    _getAllItemsByPlaylistRecursive(compositesTraversed,allSongs,std::make_shared<SBIDBase>(*this));

    //	Populate playlist
    for(int i=0;i<allSongs.count();i++)
    {
        playList[i]=allSongs.at(i);
    }
    return playList;
}
