#include "QSqlRecord"

#include "Preloader.h"

#include "Context.h"
#include "DataAccessLayer.h"
#include "SBIDAlbumPerformance.h"

QVector<SBIDAlbumPerformancePtr>
Preloader::performances(QString query, bool showProgressDialogFlag)
{
    SBIDAlbumMgr* amgr=Context::instance()->getAlbumMgr();
    SBIDPerformerMgr* pemgr=Context::instance()->getPerformerMgr();
    SBIDAlbumPerformanceMgr* apmgr=Context::instance()->getAlbumPerformanceMgr();
    SBIDSongMgr* smgr=Context::instance()->getSongMgr();
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QVector<SBIDAlbumPerformancePtr> items;
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
    QStringList songFields; songFields << "0" << "1" << "2" << "3" << "4" << "5";
    QStringList albumFields; albumFields << "6" << "7" << "8" << "9" << "10" << "11";
    QStringList performerFields; performerFields << "12" << "13" << "14" << "15" << "16";
    QStringList albumPerformanceFields; albumPerformanceFields << "0" << "6" << "17" << "12" << "18" << "4" << "19" << "20";

    dal->customize(query);
    QSqlQuery queryList(query,db);
    int maxValue=queryList.size();
    if(maxValue<0)
    {
        //	Count items
        while(queryList.next())
        {
            maxValue++;
        }
    }

    //	Set up progress dialog
    QProgressDialog pd("Retrieving Performances",QString(),0,maxValue);
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

    items.clear();
    queryList.first();
    queryList.previous();
    while(queryList.next())
    {
        QString key;
        QSqlRecord r;
        QSqlField f;
        SBIDAlbumPtr albumPtr;
        SBIDAlbumPerformancePtr performancePtr;
        SBIDPerformerPtr performerPtr;
        SBIDPlaylistPtr playlistPtr;
        SBIDSongPtr songPtr;

        //	Process song
        if(!queryList.isNull(0))
        {
            key=SBIDSong::createKey(queryList.value(20).toInt());
            if(key.length()>0)
            {
                songPtr=(smgr->contains(key)? smgr->retrieve(key,SBIDSongMgr::open_flag_parentonly): _instantiateSong(smgr,songFields,queryList));
            }
        }

        //	Process album
        if(!queryList.isNull(6))
        {
            key=SBIDAlbum::createKey(queryList.value(6).toInt());
            if(key.length()>0)
            {
                albumPtr=(amgr->contains(key)? amgr->retrieve(key,SBIDAlbumMgr::open_flag_parentonly): _instantiateAlbum(amgr,albumFields,queryList));
            }
        }

        //	Process performer
        if(!queryList.isNull(12))
        {
            key=SBIDPerformer::createKey(queryList.value(12).toInt());
            if(key.length()>0)
            {
                performerPtr=(pemgr->contains(key)? pemgr->retrieve(key,SBIDPerformerMgr::open_flag_parentonly): _instantiatePerformer(pemgr,performerFields,queryList));
            }
        }

        //	Process performance
        if(!queryList.isNull(6) && !queryList.isNull(17))
        {
            key=SBIDAlbumPerformance::createKey(queryList.value(6).toInt(),queryList.value(17).toInt());
            if(key.length()>0)
            {
                performancePtr=(apmgr->contains(key)? apmgr->retrieve(key,SBIDAlbumPerformanceMgr::open_flag_parentonly): _instantiateAlbumPerformance(apmgr,albumPerformanceFields,queryList));
            }
        }

        if(performancePtr)
        {
            items.append(performancePtr);
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

QMap<int,SBIDAlbumPerformancePtr>
Preloader::performanceMap(QString query, bool showProgressDialogFlag)
{
    SBIDAlbumMgr* amgr=Context::instance()->getAlbumMgr();
    SBIDPerformerMgr* pemgr=Context::instance()->getPerformerMgr();
    SBIDAlbumPerformanceMgr* apmgr=Context::instance()->getAlbumPerformanceMgr();
    SBIDSongMgr* smgr=Context::instance()->getSongMgr();
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QMap<int,SBIDAlbumPerformancePtr> items;
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
    QStringList songFields; songFields << "0" << "1" << "2" << "3" << "4" << "5";
    QStringList albumFields; albumFields << "6" << "7" << "8" << "9" << "10" << "11";
    QStringList performerFields; performerFields << "12" << "13" << "14" << "15" << "16";
    QStringList albumPerformanceFields; albumPerformanceFields << "0" << "6" << "17" << "12" << "18" << "4" << "19" << "20";

    dal->customize(query);
    qDebug() << SB_DEBUG_INFO << query;
    QSqlQuery queryList(query,db);
    int maxValue=queryList.size();
    if(maxValue<0)
    {
        //	Count items
        while(queryList.next())
        {
            maxValue++;
        }
    }

    //	Set up progress dialog
    QProgressDialog pd("Retrieving Performances",QString(),0,maxValue);
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

    items.clear();
    queryList.first();
    queryList.previous();
    while(queryList.next())
    {
        QString key;
        QSqlRecord r;
        QSqlField f;
        SBIDAlbumPtr albumPtr;
        SBIDAlbumPerformancePtr performancePtr;
        SBIDPerformerPtr performerPtr;
        SBIDPlaylistPtr playlistPtr;
        SBIDSongPtr songPtr;

        //	Process song
        if(!queryList.isNull(0))
        {
            key=SBIDSong::createKey(queryList.value(20).toInt());
            if(key.length()>0)
            {
                songPtr=(smgr->contains(key)? smgr->retrieve(key,SBIDSongMgr::open_flag_parentonly): _instantiateSong(smgr,songFields,queryList));
            }
        }

        //	Process album
        if(!queryList.isNull(6))
        {
            key=SBIDAlbum::createKey(queryList.value(6).toInt());
            if(key.length()>0)
            {
                albumPtr=(amgr->contains(key)? amgr->retrieve(key,SBIDAlbumMgr::open_flag_parentonly): _instantiateAlbum(amgr,albumFields,queryList));
            }
        }

        //	Process performer
        if(!queryList.isNull(12))
        {
            key=SBIDPerformer::createKey(queryList.value(12).toInt());
            if(key.length()>0)
            {
                performerPtr=(pemgr->contains(key)? pemgr->retrieve(key,SBIDPerformerMgr::open_flag_parentonly): _instantiatePerformer(pemgr,performerFields,queryList));
            }
        }

        //	Process performance
        if(!queryList.isNull(6) && !queryList.isNull(17))
        {
            key=SBIDAlbumPerformance::createKey(queryList.value(6).toInt(),queryList.value(17).toInt());
            if(key.length()>0)
            {
                performancePtr=(apmgr->contains(key)? apmgr->retrieve(key,SBIDAlbumPerformanceMgr::open_flag_parentonly): _instantiateAlbumPerformance(apmgr,albumPerformanceFields,queryList));
            }
        }

        if(performancePtr)
        {
            items[performancePtr->albumPosition()]=performancePtr;
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

QMap<int,SBIDPtr>
Preloader::playlistItems(int playlistID,bool showProgressDialogFlag)
{
    SBIDAlbumMgr* amgr=Context::instance()->getAlbumMgr();
    SBIDPerformerMgr* pemgr=Context::instance()->getPerformerMgr();
    SBIDAlbumPerformanceMgr* apmgr=Context::instance()->getAlbumPerformanceMgr();
    SBIDPlaylistMgr* plmgr=Context::instance()->getPlaylistMgr();
    SBIDSongMgr* smgr=Context::instance()->getSongMgr();
    QMap<int,SBIDPtr> items;
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
    int maxValue=0;
    QStringList songFields; songFields << "2" << "20" << "21" << "22" << "23" << "24";
    QStringList albumFields; albumFields << "10" << "11" << "12" << "13" << "14" << "15";
    QStringList performerFields; performerFields << "5" << "6" << "7" << "8" << "9";
    QStringList albumPerformanceFields; albumPerformanceFields << "2" << "3" << "4" << "5" << "25" << "26" << "27" << "28";

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
        SBIDAlbumPerformancePtr performancePtr;
        SBIDPerformerPtr performerPtr;
        SBIDPlaylistPtr playlistPtr;
        SBIDSongPtr songPtr;

        //	Process performer
        if(!queryList.isNull(5))
        {
            key=SBIDPerformer::createKey(queryList.value(5).toInt());
            if(key.length()>0)
            {
                performerPtr=(pemgr->contains(key)? pemgr->retrieve(key,SBIDPerformerMgr::open_flag_parentonly): _instantiatePerformer(pemgr,performerFields,queryList));
            }
        }

        //	Process album
        if(!queryList.isNull(10))
        {
            key=SBIDAlbum::createKey(queryList.value(10).toInt());
            if(key.length()>0)
            {
                albumPtr=(amgr->contains(key)? amgr->retrieve(key,SBIDAlbumMgr::open_flag_parentonly): _instantiateAlbum(amgr,albumFields,queryList));
            }
        }

        //	Process playlist
        if(!queryList.isNull(16))
        {
            key=SBIDPlaylist::createKey(queryList.value(16).toInt());
            if(key.length()>0)
            {
                if(plmgr->contains(key))
                {
                    playlistPtr=plmgr->retrieve(key,SBIDPlaylistMgr::open_flag_parentonly);
                }
                else
                {
                    //	Instantiate playlist
                    r.clear();
                    f=QSqlField("f1",QVariant::Int);    f.setValue(queryList.value(16).toInt());    r.append(f);
                    f=QSqlField("f2",QVariant::String); f.setValue(queryList.value(17).toString()); r.append(f);
                    f=QSqlField("f3",QVariant::String); f.setValue(queryList.value(18).toString()); r.append(f);
                    f=QSqlField("f4",QVariant::Int);    f.setValue(queryList.value(19).toInt());    r.append(f);
                    playlistPtr=SBIDPlaylist::instantiate(r);
                    plmgr->addItem(playlistPtr);
                }
            }
        }

        //	Process song
        if(!queryList.isNull(2))
        {
            key=SBIDSong::createKey(queryList.value(20).toInt());
            if(key.length()>0)
            {
                songPtr=(smgr->contains(key)? smgr->retrieve(key,SBIDSongMgr::open_flag_parentonly): _instantiateSong(smgr,songFields,queryList));
            }
        }

        //	Process performance
        if(!queryList.isNull(3) && !queryList.isNull(4))
        {
            key=SBIDAlbumPerformance::createKey(queryList.value(3).toInt(),queryList.value(4).toInt());
            if(key.length()>0)
            {
                performancePtr=(apmgr->contains(key)? apmgr->retrieve(key,SBIDAlbumPerformanceMgr::open_flag_parentonly): _instantiateAlbumPerformance(apmgr,albumPerformanceFields,queryList));
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
        case Common::sb_field_key:
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

///	Private methods
SBIDAlbumPtr
Preloader::_instantiateAlbum(SBIDAlbumMgr* amgr, const QStringList& fields, const QSqlQuery& queryList)
{
    QSqlRecord r;
    QSqlField f;

    f=QSqlField("f1",QVariant::Int);    f.setValue(queryList.value(fields.at(0).toInt()).toInt());    r.append(f);
    f=QSqlField("f2",QVariant::String); f.setValue(queryList.value(fields.at(1).toInt()).toString()); r.append(f);
    f=QSqlField("f3",QVariant::Int);    f.setValue(queryList.value(fields.at(2).toInt()).toInt());    r.append(f);
    f=QSqlField("f4",QVariant::Int);    f.setValue(queryList.value(fields.at(3).toInt()).toInt());    r.append(f);
    f=QSqlField("f5",QVariant::String); f.setValue(queryList.value(fields.at(4).toInt()).toString()); r.append(f);
    f=QSqlField("f6",QVariant::String); f.setValue(queryList.value(fields.at(5).toInt()).toString()); r.append(f);

    SBIDAlbumPtr albumPtr=SBIDAlbum::instantiate(r);
    amgr->addItem(albumPtr);
    return albumPtr;
}

SBIDAlbumPerformancePtr
Preloader::_instantiateAlbumPerformance(SBIDAlbumPerformanceMgr* apmgr, const QStringList& fields, const QSqlQuery& queryList)
{
    QSqlRecord r;
    QSqlField f;

    f=QSqlField("f1",QVariant::Int);    f.setValue(queryList.value(fields.at(0).toInt()).toInt());    r.append(f);
    f=QSqlField("f2",QVariant::Int);    f.setValue(queryList.value(fields.at(1).toInt()).toInt());    r.append(f);
    f=QSqlField("f3",QVariant::Int);    f.setValue(queryList.value(fields.at(2).toInt()).toInt());    r.append(f);
    f=QSqlField("f4",QVariant::Int);    f.setValue(queryList.value(fields.at(3).toInt()).toInt());    r.append(f);
    f=QSqlField("f5",QVariant::String); f.setValue(queryList.value(fields.at(4).toInt()).toString()); r.append(f);
    f=QSqlField("f6",QVariant::Int);    f.setValue(queryList.value(fields.at(5).toInt()).toInt());    r.append(f);
    f=QSqlField("f7",QVariant::String); f.setValue(queryList.value(fields.at(6).toInt()).toString()); r.append(f);
    f=QSqlField("f8",QVariant::String); f.setValue(queryList.value(fields.at(7).toInt()).toString()); r.append(f);

    SBIDAlbumPerformancePtr performancePtr=SBIDAlbumPerformance::instantiate(r);
    apmgr->addItem(performancePtr);
    return performancePtr;
}

SBIDPerformerPtr
Preloader::_instantiatePerformer(SBIDPerformerMgr* pemgr, const QStringList& fields, const QSqlQuery& queryList)
{
    QSqlRecord r;
    QSqlField f;

    f=QSqlField("f1",QVariant::Int);    f.setValue(queryList.value(fields.at(0).toInt()).toInt());    r.append(f);
    f=QSqlField("f2",QVariant::String); f.setValue(queryList.value(fields.at(1).toInt()).toString()); r.append(f);
    f=QSqlField("f3",QVariant::String); f.setValue(queryList.value(fields.at(2).toInt()).toString()); r.append(f);
    f=QSqlField("f4",QVariant::String); f.setValue(queryList.value(fields.at(3).toInt()).toString()); r.append(f);
    f=QSqlField("f5",QVariant::String); f.setValue(queryList.value(fields.at(4).toInt()).toString()); r.append(f);

    SBIDPerformerPtr performerPtr=SBIDPerformer::instantiate(r);
    pemgr->addItem(performerPtr);
    return performerPtr;
}

SBIDSongPtr
Preloader::_instantiateSong(SBIDSongMgr* smgr, const QStringList& fields, const QSqlQuery& queryList)
{
    QSqlRecord r;
    QSqlField f;

    f=QSqlField("f1",QVariant::Int);    f.setValue(queryList.value(fields.at(0).toInt()).toInt());    r.append(f);
    f=QSqlField("f2",QVariant::String); f.setValue(queryList.value(fields.at(1).toInt()).toString()); r.append(f);
    f=QSqlField("f3",QVariant::String); f.setValue(queryList.value(fields.at(2).toInt()).toString()); r.append(f);
    f=QSqlField("f4",QVariant::Int);    f.setValue(queryList.value(fields.at(3).toInt()).toInt());    r.append(f);
    f=QSqlField("f5",QVariant::Int);    f.setValue(queryList.value(fields.at(4).toInt()).toInt());    r.append(f);
    f=QSqlField("f6",QVariant::String); f.setValue(queryList.value(fields.at(5).toInt()).toString()); r.append(f);

    SBIDSongPtr songPtr=SBIDSong::instantiate(r);
    smgr->addItem(songPtr);
    return songPtr;
}
