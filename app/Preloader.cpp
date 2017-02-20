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
    QStringList songFields; songFields << "0" << "1" << "2" << "3" << "4" << "22";
    QStringList albumFields; albumFields << "6" << "7" << "8" << "9" << "10" << "11";
    QStringList performerFields; performerFields << "12" << "13" << "14" << "15" << "16";
    QStringList albumPerformanceFields; albumPerformanceFields << "21" << "0" << "6" << "17" << "12" << "18" << "4" << "19" << "20";

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
            key=SBIDSong::createKey(queryList.value(0).toInt());
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
        if(!queryList.isNull(21))
        {
            key=SBIDAlbumPerformance::createKey(queryList.value(21).toInt());
            if(key.length()>0)
            {
                //	checked on albumPerformanceID
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
    QStringList songFields; songFields << "0" << "1" << "2" << "3" << "4" << "22";
    QStringList albumFields; albumFields << "6" << "7" << "8" << "9" << "10" << "11";
    QStringList performerFields; performerFields << "12" << "13" << "14" << "15" << "16";
    QStringList albumPerformanceFields; albumPerformanceFields << "21" << "0" << "6" << "17" << "12" << "18" << "4" << "19" << "20";

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
            key=SBIDSong::createKey(queryList.value(0).toInt());
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
        if(!queryList.isNull(21))
        {
            key=SBIDAlbumPerformance::createKey(queryList.value(21).toInt());
            if(key.length()>0)
            {
                //	checked on albumPerformanceID
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
    QStringList albumPerformanceFields; albumPerformanceFields << "2" << "29" << "3" << "4" << "5" << "25" << "26" << "27" << "28";

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
                "___SB_SCHEMA_NAME___playlist_detail pc "
            "WHERE "
                "pc.playlist_id=%1 "
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
            "pc.playlist_position as \"#\", "                       //	0
            "CASE "
                "WHEN pc.child_playlist_id IS NOT NULL THEN %2 "
                "WHEN pc.record_id         IS NOT NULL THEN %3 "
                "WHEN pc.artist_id         IS NOT NULL THEN %4 "
            "END AS SB_ITEM_TYPE, "
            "COALESCE(pc.child_playlist_id,pc.record_id,pc.artist_id) AS SB_ITEM_ID, "
            "0 AS SB_ALBUM_ID, "
            "0 AS SB_POSITION_ID, "

            "a.artist_id, "                                         //	5
            "a.name, "
            "a.www, "
            "a.notes, "
            "a.mbid, "

            "r.record_id, "                                         //	10
            "r.title, "
            "r.artist_id, "
            "r.year, "
            "r.genre, "

            "r.notes, "                                             //	15
            "pl.playlist_id, "
            "pl.name AS playlist_name, "
            "pl.duration AS playlist_duration, "
            "0 AS playlist_num_items, "

            "NULL AS song_title, "                                  //	20
            "NULL AS notes, "
            "NULL AS artist_id, "
            "NULL AS year, "
            "NULL AS lyrics, "

            "NULL AS duration, "                                    //	25
            "NULL AS year, "
            "NULL AS notes, "
            "NULL AS path, "
            "NULL AS song_id "                                      //	29
        "FROM "
            "___SB_SCHEMA_NAME___playlist_detail pc "
                "LEFT JOIN ___SB_SCHEMA_NAME___artist a ON "
                    "pc.artist_id=a.artist_id "
                "LEFT JOIN ___SB_SCHEMA_NAME___record r ON "
                    "pc.record_id=r.record_id "
                "LEFT JOIN ___SB_SCHEMA_NAME___playlist pl ON "
                    "pc.child_playlist_id=pl.playlist_id "
        "WHERE "
            "pc.record_performance_id IS NULL AND "
            "pc.playlist_id=%1 "
        "UNION "
        "SELECT "
            "pp.playlist_position, "                                //	0
            "%5, "
            "rp.record_performance_id, "	//	not used, only to indicate a performance
            "rp.record_id AS SB_ALBUM_ID, "
            "rp.record_position AS SB_POSITION_ID, "

            "a.artist_id, "                                         //	5
            "a.name, "
            "a.www, "
            "a.notes, "
            "a.mbid, "

            "r.record_id, "                                         //	10
            "r.title, "
            "r.artist_id, "
            "r.year, "
            "r.genre, "

            "r.notes, "                                             //	15
            "NULL AS playlist_id, "
            "NULL AS playlist_name, "
            "NULL AS playlist_duration, "
            "NULL AS playlist_num_items, "

            "s.title, "                                             //	20
            "s.notes, "
            "p.artist_id, "
            "p.year, "
            "l.lyrics, "

            "rp.duration, "                                         //	25
            "p.year, "
            "rp.notes, "
            "op.path, "
            "s.song_id "                                            //	29

        "FROM "
            "___SB_SCHEMA_NAME___playlist_detail pp  "
                "JOIN ___SB_SCHEMA_NAME___record_performance rp ON "
                    "pp.record_performance_id=rp.record_performance_id "
                "JOIN ___SB_SCHEMA_NAME___record r ON "
                    "rp.record_id=r.record_id "
                //	LEFT JOIN ON online_performance to show everything.
                "LEFT JOIN ___SB_SCHEMA_NAME___online_performance op ON "
                    "op.record_performance_id=rp.record_performance_id "
                "JOIN ___SB_SCHEMA_NAME___performance p ON "
                    "rp.performance_id=p.performance_id "
                "JOIN ___SB_SCHEMA_NAME___artist a ON "
                    "p.artist_id=a.artist_id "
                "JOIN ___SB_SCHEMA_NAME___song s ON "
                    "p.song_id=s.song_id "
                "LEFT JOIN ___SB_SCHEMA_NAME___lyrics l ON "
                    "s.song_id=l.song_id "
        "WHERE "
            "pp.record_performance_id IS NOT NULL AND "
            "pp.playlist_id=%1 "
        "ORDER BY 1"
    )
            .arg(playlistID)
            .arg(Common::sb_field_playlist_id)
            .arg(Common::sb_field_album_id)
            .arg(Common::sb_field_performer_id)
            .arg(Common::sb_field_album_performance_id)
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
        Common::sb_field itemType=static_cast<Common::sb_field>(queryList.value(1).toInt());
        SBIDPlaylistPtr playlistPtr;
        SBIDSongPtr songPtr;
        SBIDPtr itemPtr;

        //	Process performer
        if(itemType==Common::sb_field_performer_id)
        {
            key=SBIDPerformer::createKey(queryList.value(5).toInt());
            if(key.length()>0)
            {
                itemPtr=(pemgr->contains(key)? pemgr->retrieve(key,SBIDPerformerMgr::open_flag_parentonly): _instantiatePerformer(pemgr,performerFields,queryList));
            }
        }

        //	Process album
        if(itemType==Common::sb_field_album_id)
        {
            key=SBIDAlbum::createKey(queryList.value(10).toInt());
            if(key.length()>0)
            {
                itemPtr=(amgr->contains(key)? amgr->retrieve(key,SBIDAlbumMgr::open_flag_parentonly): _instantiateAlbum(amgr,albumFields,queryList));
            }
        }

        //	Process playlist
        if(itemType==Common::sb_field_playlist_id)
        {
            key=SBIDPlaylist::createKey(queryList.value(16).toInt());
            if(key.length()>0)
            {
                if(plmgr->contains(key))
                {
                    itemPtr=plmgr->retrieve(key,SBIDPlaylistMgr::open_flag_parentonly);
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
                    itemPtr=playlistPtr;
                }
            }
        }

        //	Process song
        if(itemType==Common::sb_field_song_id)
        {
            key=SBIDSong::createKey(queryList.value(2).toInt());
            if(key.length()>0)
            {
                itemPtr=(smgr->contains(key)? smgr->retrieve(key,SBIDSongMgr::open_flag_parentonly): _instantiateSong(smgr,songFields,queryList));
            }
        }

        //	Process performance
        if(itemType==Common::sb_field_album_performance_id)
        {
            key=SBIDAlbumPerformance::createKey(queryList.value(2).toInt());
            if(key.length()>0)
            {
                //	checked on albumPerformanceID
                itemPtr=(apmgr->contains(key)? apmgr->retrieve(key,SBIDAlbumPerformanceMgr::open_flag_parentonly): _instantiateAlbumPerformance(apmgr,albumPerformanceFields,queryList));

                if(itemPtr)
                {
                    qDebug() << SB_DEBUG_INFO << (*itemPtr);
                }
            }
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

    //	CWIP: apid
    f=QSqlField("f1",QVariant::Int);    f.setValue(queryList.value(fields.at(0).toInt()).toInt());    r.append(f);
    f=QSqlField("f2",QVariant::Int);    f.setValue(queryList.value(fields.at(1).toInt()).toInt());    r.append(f);
    f=QSqlField("f3",QVariant::Int);    f.setValue(queryList.value(fields.at(2).toInt()).toInt());    r.append(f);
    f=QSqlField("f4",QVariant::Int);    f.setValue(queryList.value(fields.at(3).toInt()).toInt());    r.append(f);
    f=QSqlField("f5",QVariant::Int);    f.setValue(queryList.value(fields.at(4).toInt()).toInt());    r.append(f);
    f=QSqlField("f6",QVariant::String); f.setValue(queryList.value(fields.at(5).toInt()).toString()); r.append(f);
    f=QSqlField("f7",QVariant::Int);    f.setValue(queryList.value(fields.at(6).toInt()).toInt());    r.append(f);
    f=QSqlField("f8",QVariant::String); f.setValue(queryList.value(fields.at(7).toInt()).toString()); r.append(f);
    f=QSqlField("f9",QVariant::String); f.setValue(queryList.value(fields.at(8).toInt()).toString()); r.append(f);

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
