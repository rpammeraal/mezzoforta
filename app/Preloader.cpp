#include "QSqlRecord"

#include "Preloader.h"

#include "Context.h"
#include "DataAccessLayer.h"

QMap<int,SBIDPtr>
Preloader::playlistItems(int playlistID,bool showProgressDialogFlag)
{
    SBIDAlbumMgr* amgr=Context::instance()->getAlbumMgr();
    SBIDPerformerMgr* pemgr=Context::instance()->getPerformerMgr();
    SBIDPerformanceMgr* pfmgr=Context::instance()->getPerformanceMgr();
    SBIDPlaylistMgr* plmgr=Context::instance()->getPlaylistMgr();
    SBIDSongMgr* smgr=Context::instance()->getSongMgr();
    QMap<int,SBIDPtr> items;
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
    int maxValue=0;

    QString q;

    //	Retrieve number of items
    q=QString
    (
        "SELECT SUM(cnt) "
        "FROM "
        "( "
            "SELECT "
                "COUNT(*) AS cnt "
            "FROM "
                "___SB_SCHEMA_NAME___playlist_composite pc "
            "WHERE "
                "pc.playlist_id=%1 "
            "UNION "
            "SELECT "
                "COUNT(*) AS cnt "
            "FROM "
                "___SB_SCHEMA_NAME___playlist_performance pp  "
            "WHERE "
                "pp.playlist_id=%1 "
        ") a "
    )
        .arg(playlistID)
    ;

    dal->customize(q);
    qDebug() << SB_DEBUG_INFO << q;

    QSqlQuery countList(q,db);
    if(countList.next())
    {
        maxValue=countList.value(0).toInt();
    }

    //	Retrieve detail
    q=QString
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
            "0 AS SB_ALBUM_ID, "
            "0 AS SB_POSITION_ID, "
            "a.artist_id, "
            "a.name, "
            "a.www, "
            "a.notes, "
            "a.mbid, "
            "r.record_id, "
            "r.title, "
            "r.artist_id, "
            "r.year, "
            "r.genre, "
            "r.notes, "
            "pl.playlist_id, "
            "pl.name AS playlist_name, "
            "pl.duration AS playlist_duration, "
            "0 AS playlist_num_items, "
            "NULL AS song_title, "
            "NULL AS notes, "
            "NULL AS artist_id, "
            "NULL AS year, "
            "NULL AS lyrics, "
            "NULL AS duration, "
            "NULL AS year, "
            "NULL AS notes, "
            "NULL AS path "
        "FROM "
            "___SB_SCHEMA_NAME___playlist_composite pc "
                "LEFT JOIN ___SB_SCHEMA_NAME___artist a ON "
                    "pc.playlist_artist_id=a.artist_id "
                "LEFT JOIN ___SB_SCHEMA_NAME___record r ON "
                    "pc.playlist_record_id=r.record_id "
                "LEFT JOIN ___SB_SCHEMA_NAME___playlist pl ON "
                    "pc.playlist_playlist_id=pl.playlist_id "
        "WHERE "
            "pc.playlist_id=%1 "
        "UNION "
        "SELECT "
            "pp.playlist_position, "
            "%6, "
            "pp.song_id, "	//	not used, only to indicate a performance
            "pp.record_id AS SB_ALBUM_ID, "
            "pp.record_position AS SB_POSITION_ID, "
            "a.artist_id, "
            "a.name, "
            "a.www, "
            "a.notes, "
            "a.mbid, "
            "r.record_id, "
            "r.title, "
            "r.artist_id, "
            "r.year, "
            "r.genre, "
            "r.notes, "
            "NULL AS playlist_id, "
            "NULL AS playlist_name, "
            "NULL AS playlist_duration, "
            "NULL AS playlist_num_items, "
            "s.title, "
            "s.notes, "
            "p.artist_id, "
            "p.year, "
            "l.lyrics, "
            "rp.duration, "
            "p.year, "
            "rp.notes, "
            "op.path "
        "FROM "
            "___SB_SCHEMA_NAME___playlist_performance pp  "
                "JOIN ___SB_SCHEMA_NAME___artist a ON "
                    "pp.artist_id=a.artist_id "
                "JOIN ___SB_SCHEMA_NAME___record r ON "
                    "pp.record_id=r.record_id "
                "JOIN ___SB_SCHEMA_NAME___song s ON "
                    "pp.song_id=s.song_id "
                "JOIN ___SB_SCHEMA_NAME___performance p ON "
                    "pp.song_id=p.song_id AND "
                    "p.role_id=0 "
                "JOIN ___SB_SCHEMA_NAME___record_performance rp ON "
                    "pp.song_id=rp.song_id AND "
                    "pp.artist_id=rp.artist_id AND "
                    "pp.record_id=rp.record_id AND "
                    "pp.record_position=rp.record_position "
                "JOIN ___SB_SCHEMA_NAME___online_performance op ON "
                    "rp.op_song_id=op.song_id AND "
                    "rp.op_artist_id=op.artist_id AND "
                    "rp.op_record_id=op.record_id AND "
                    "rp.op_record_position=op.record_position "
                "LEFT JOIN ___SB_SCHEMA_NAME___lyrics l ON "
                    "s.song_id=l.song_id "
        "WHERE "
            "pp.playlist_id=%1 "
        "ORDER BY 1"
    )
            .arg(playlistID)
            .arg(Common::sb_field_playlist_id)
            .arg(Common::sb_field_chart_id)
            .arg(Common::sb_field_album_id)
            .arg(Common::sb_field_performer_id)
            .arg(Common::sb_field_song_id)
    ;

    dal->customize(q);
    qDebug() << SB_DEBUG_INFO << q;

    //	Set up progress dialog
    QProgressDialog pd("Retrieving All Items",QString(),0,maxValue);
    if(maxValue<=10)
    {
        showProgressDialogFlag=0;
    }

    int currentValue=0;
    if(showProgressDialogFlag)
    {
        pd.setWindowModality(Qt::WindowModal);
        pd.show();
        pd.raise();
        pd.activateWindow();
        QCoreApplication::processEvents();
    }

    QSqlQuery queryList(q,db);
    int playlistIndex=0;
    items.clear();
    while(queryList.next())
    {
        QSqlRecord r;
        QSqlField f;
        QString key;
        SBIDAlbumPtr albumPtr;
        SBIDPerformancePtr performancePtr;
        SBIDPerformerPtr performerPtr;
        SBIDPlaylistPtr playlistPtr;
        SBIDSongPtr songPtr;

        //	Process performer
        if(!queryList.isNull(5))
        {
            key=SBIDPerformer::createKey(queryList.value(5).toInt());
            if(key.length()>0 && !pemgr->contains(key))
            {
                //	Instantiate performer
                r.clear();
                f=QSqlField("f1",QVariant::Int);    f.setValue(queryList.value( 5).toInt());    r.append(f);
                f=QSqlField("f2",QVariant::String); f.setValue(queryList.value( 6).toString()); r.append(f);
                f=QSqlField("f3",QVariant::String); f.setValue(queryList.value( 7).toString()); r.append(f);
                f=QSqlField("f4",QVariant::String); f.setValue(queryList.value( 8).toString()); r.append(f);
                f=QSqlField("f5",QVariant::String); f.setValue(queryList.value( 9).toString()); r.append(f);
                performerPtr=SBIDPerformer::instantiate(r,1);
                pemgr->addItem(performerPtr);
                qDebug() << SB_DEBUG_INFO << performerPtr->key() << performerPtr->genericDescription();
            }
        }

        //	Process album
        if(!queryList.isNull(10))
        {
            key=SBIDAlbum::createKey(queryList.value(10).toInt());
            if(key.length()>0 && !amgr->contains(key))
            {
                //	Instantiate album
                r.clear();
                f=QSqlField("f1",QVariant::Int);    f.setValue(queryList.value(10).toInt());    r.append(f);
                f=QSqlField("f2",QVariant::String); f.setValue(queryList.value(11).toString()); r.append(f);
                f=QSqlField("f3",QVariant::Int);    f.setValue(queryList.value(12).toInt());    r.append(f);
                f=QSqlField("f4",QVariant::Int);    f.setValue(queryList.value(13).toInt());    r.append(f);
                f=QSqlField("f5",QVariant::String); f.setValue(queryList.value(14).toString()); r.append(f);
                f=QSqlField("f6",QVariant::String); f.setValue(queryList.value(15).toString()); r.append(f);
                albumPtr=SBIDAlbum::instantiate(r,1);
                amgr->addItem(albumPtr);
                qDebug() << SB_DEBUG_INFO << albumPtr->key() << albumPtr->genericDescription();
            }
        }

        //	Process playlist
        if(!queryList.isNull(16))
        {
            key=SBIDPlaylist::createKey(queryList.value(16).toInt());
            if(key.length()>0 && !plmgr->contains(key))
            {
                //	Instantiate playlist
                r.clear();
                f=QSqlField("f1",QVariant::Int);    f.setValue(queryList.value(16).toInt());    r.append(f);
                f=QSqlField("f2",QVariant::String); f.setValue(queryList.value(17).toString()); r.append(f);
                f=QSqlField("f3",QVariant::String); f.setValue(queryList.value(18).toString()); r.append(f);
                f=QSqlField("f4",QVariant::Int);    f.setValue(queryList.value(19).toInt());    r.append(f);
                playlistPtr=SBIDPlaylist::instantiate(r,1);
                plmgr->addItem(playlistPtr);
                qDebug() << SB_DEBUG_INFO << playlistPtr->key() << playlistPtr->genericDescription();
            }
        }

        //	Process song
        if(!queryList.isNull(2))
        {
            key=SBIDSong::createKey(queryList.value(20).toInt());
            if(key.length()>0 && !plmgr->contains(key))
            {
                //	Instantiate song
                r.clear();
                f=QSqlField("f1",QVariant::Int);    f.setValue(queryList.value( 2).toInt());    r.append(f);
                f=QSqlField("f2",QVariant::String); f.setValue(queryList.value(20).toString()); r.append(f);
                f=QSqlField("f3",QVariant::String); f.setValue(queryList.value(21).toString()); r.append(f);
                f=QSqlField("f4",QVariant::Int);    f.setValue(queryList.value(22).toInt());    r.append(f);
                f=QSqlField("f5",QVariant::Int);    f.setValue(queryList.value(23).toInt());    r.append(f);
                f=QSqlField("f6",QVariant::String); f.setValue(queryList.value(24).toString()); r.append(f);

                songPtr=SBIDSong::instantiate(r,1);
                smgr->addItem(songPtr);
                qDebug() << SB_DEBUG_INFO << songPtr->key() << songPtr->genericDescription();
            }
        }

        //	Process performance
        if(!queryList.isNull(3) && !queryList.isNull(4))
        {
            key=SBIDPerformance::createKey(queryList.value(3).toInt(),queryList.value(4).toInt());
            if(key.length()>0 && !pfmgr->contains(key))
            {
                //	Instantiate performance
                r.clear();
                f=QSqlField("f1",QVariant::Int);    f.setValue(queryList.value( 2).toInt());    r.append(f);
                f=QSqlField("f2",QVariant::Int);    f.setValue(queryList.value( 3).toInt());    r.append(f);
                f=QSqlField("f3",QVariant::Int);    f.setValue(queryList.value( 4).toInt());    r.append(f);
                f=QSqlField("f4",QVariant::Int);    f.setValue(queryList.value( 5).toInt());    r.append(f);
                f=QSqlField("f5",QVariant::Int);    f.setValue(queryList.value(25).toTime());   r.append(f);
                f=QSqlField("f6",QVariant::Int);    f.setValue(queryList.value(26).toInt());    r.append(f);
                f=QSqlField("f7",QVariant::String); f.setValue(queryList.value(27).toString()); r.append(f);
                f=QSqlField("f8",QVariant::String); f.setValue(queryList.value(28).toString()); r.append(f);
                performancePtr=SBIDPerformance::instantiate(r,1);
                pfmgr->addItem(performancePtr);
                qDebug() << SB_DEBUG_INFO << performancePtr->key() << performancePtr->genericDescription();
            }
        }

        //	Instantiate playlist item
        Common::sb_field itemType=static_cast<Common::sb_field>(queryList.value(1).toInt());
        SBIDPtr itemPtr;

        switch(itemType)
        {
        case Common::sb_field_playlist_id:
            itemPtr=playlistPtr;
            break;

        case Common::sb_field_chart_id:
            break;

        case Common::sb_field_album_id:
            itemPtr=albumPtr;
            break;

        case Common::sb_field_performer_id:
            itemPtr=performerPtr;
            break;

        case Common::sb_field_song_id:
            if(queryList.value(3).isNull())
            {
                //	Item is a song
                itemPtr=songPtr;
            }
            else
            {
                //	Item is a performance as we have album_id and album_position populated
                itemPtr=performancePtr;
            }
            break;

        case Common::sb_field_invalid:
        case Common::sb_field_album_position:
            break;
        }
        if(itemPtr)
        {
            items[playlistIndex++]=itemPtr;
        }
        if(showProgressDialogFlag && (currentValue%10)==0)
        {
            QCoreApplication::processEvents();
            pd.setValue(currentValue);
        }
        currentValue++;
    }
    if(showProgressDialogFlag)
    {
        pd.setValue(maxValue);
    }
    return items;
}
