#include "QSqlRecord"

#include "Preloader.h"

#include "Context.h"
#include "DataAccessLayer.h"
#include "ProgressDialog.h"
#include "SBIDAlbumPerformance.h"

//	Retrieve map between chart and chart performance based on an SBID object, which is either:
//	-	chart
//	-	performer (all chart performances by chart for this performer)
//	-	song (all chart performances by chart for this song)
QMap<SBIDChartPerformancePtr,SBIDChartPtr>
Preloader::chartItems(const SBIDBase& id)
{
    CacheManager* cm=Context::instance()->cacheManager();
    CachePerformerMgr* pemgr=cm->performerMgr();
    CacheChartMgr* cmgr=cm->chartMgr();
    CacheChartPerformanceMgr* cpmgr=cm->chartPerformanceMgr();
    CacheSongPerformanceMgr* spmgr=cm->songPerformanceMgr();
    CacheSongMgr* smgr=cm->songMgr();
    DataAccessLayer* dal=Context::instance()->dataAccessLayer();
    QMap<SBIDChartPerformancePtr,SBIDChartPtr> items;
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());

    QString whereClause;
    switch(id.itemType())
    {
        case SBKey::Song:
            whereClause=QString("p.song_id=%1").arg(id.itemID());
            break;
        case SBKey::Performer:
            whereClause=QString("a.artist_id=%1").arg(id.itemID());
            break;
        case SBKey::Chart:
            whereClause=QString("cp.chart_id=%1").arg(id.itemID());
            break;

        case SBKey::Invalid:
        case SBKey::Album:
        case SBKey::Playlist:
        case SBKey::SongPerformance:
        case SBKey::AlbumPerformance:
        case SBKey::OnlinePerformance:
        case SBKey::ChartPerformance:
        case SBKey::PlaylistDetail:
            whereClause="1=0";
            break;
    }

    QStringList chartFields; chartFields                       << "0"  << "1"  << "3"  << "2"  << "22";
    QStringList chartPerformanceFields; chartPerformanceFields << "4"  << "0"  << "5"  << "6"  << "7";
    QStringList performerFields; performerFields               << "9"  << "18" << "19" << "20" << "21";
    QStringList songFields; songFields                         << "8"  << "14" << "15" << "16" << "17";
    QStringList songPerformanceFields; songPerformanceFields   << "5"  << "8"  << "9"  << "11" << "12"  << "13";
    QString q;

    q=QString
    (
        "SELECT DISTINCT "
            "c.chart_id, "                    //	0
            "c.name, "
            "c.release_date, "
            "c.notes, "
            "cp.chart_performance_id, "
            "cp.performance_id, "             //	5
            "cp.chart_position, "
            "cp.notes, "
            "p.song_id, "
            "p.artist_id, "
            "NULL AS rsole_id, "                     //	10
            "p.year, "
            "p.notes, "
            "p.preferred_record_performance_id , "
            "s.title, "
            "s.notes, "                       //	15
            "l.lyrics, "
            "s.original_performance_id, "
            "a.name, "
            "a.www, "
            "a.notes, "                       //	20
            "a.mbid, "
            "0 AS num_items "
        "FROM "
            "___SB_SCHEMA_NAME___chart_performance cp "
                "JOIN ___SB_SCHEMA_NAME___chart c ON "
                    "cp.chart_id=c.chart_id "
                "JOIN ___SB_SCHEMA_NAME___performance p ON "
                    "cp.performance_id=p.performance_id "
                "JOIN ___SB_SCHEMA_NAME___song s ON "
                    "p.song_id=s.song_id "
                "LEFT JOIN ___SB_SCHEMA_NAME___lyrics l ON "
                    "s.song_id=l.song_id "
                "JOIN ___SB_SCHEMA_NAME___artist a ON "
                    "p.artist_id=a.artist_id "
        "WHERE "
            "%1 "
        "ORDER BY "
            "cp.chart_position, "
            "s.title, "
            "a.name "
    )
        .arg(whereClause)
    ;

    dal->customize(q);
    qDebug() << SB_DEBUG_INFO << q;
    QSqlQuery queryList(q,db);

    //	Set up progress dialog
    int progressCurrentValue=0;
    int progressMaxValue=queryList.size();
    if(progressMaxValue<0)
    {
        //	Count items
        while(queryList.next())
        {
            progressMaxValue++;
        }
    }
    ProgressDialog::instance()->update("Preloader::chartItems",progressCurrentValue,progressMaxValue);

    items.clear();
    queryList.first();
    queryList.previous();
    int chartPosition=0;
    while(queryList.next())
    {
        SBKey key;
        QSqlRecord r;
        QSqlField f;
        SBIDChartPtr chartPtr;
        SBIDSongPerformancePtr songPerformancePtr;
        SBIDChartPerformancePtr chartPerformancePtr;
        SBIDPerformerPtr performerPtr;
        SBIDSongPtr songPtr;

        //	Chart position
        chartPosition++;

        //	Process chart
        if(!queryList.isNull(0))
        {
            key=SBIDChart::createKey(queryList.value(0).toInt());
            if(key.validFlag())
            {
                chartPtr=(cmgr->contains(key)? cmgr->retrieve(key,Cache::open_flag_parentonly): _instantiateChart(cmgr,chartFields,queryList));
            }
        }

        //	Process chart performance
        if(!queryList.isNull(4))
        {
            key=SBIDChartPerformance::createKey(queryList.value(4).toInt());
            if(key.validFlag())
            {
                chartPerformancePtr=(cpmgr->contains(key)? cpmgr->retrieve(key,Cache::open_flag_parentonly): _instantiateChartPerformance(cpmgr,chartPerformanceFields,queryList));
            }
        }

        //	Process performer
        if(!queryList.isNull(9))
        {
            key=SBIDPerformer::createKey(queryList.value(9).toInt());
            if(key.validFlag())
            {
                performerPtr=(pemgr->contains(key)? pemgr->retrieve(key,Cache::open_flag_parentonly): _instantiatePerformer(pemgr,performerFields,queryList));
            }
        }

        //	Process song
        if(!queryList.isNull(8))
        {
            key=SBIDSong::createKey(queryList.value(8).toInt());
            if(key.validFlag())
            {
                songPtr=(smgr->contains(key)? smgr->retrieve(key,Cache::open_flag_parentonly): _instantiateSong(smgr,songFields,queryList));
            }
        }

        //	Process song performance
        if(!queryList.isNull(5))
        {
            key=SBIDSongPerformance::createKey(queryList.value(5).toInt());
            if(key.validFlag())
            {
                songPerformancePtr=(spmgr->contains(key)? spmgr->retrieve(key,Cache::open_flag_parentonly): _instantiateSongPerformance(spmgr,songPerformanceFields,queryList));
            }
        }

        if(chartPerformancePtr)
        {
            items[chartPerformancePtr]=chartPtr;
        }
        ProgressDialog::instance()->update("Preloader::chartItems",progressCurrentValue++,progressMaxValue);
    }
    ProgressDialog::instance()->finishStep("Preloader::chartItems");
    return items;
}

QVector<SBIDAlbumPerformancePtr>
Preloader::albumPerformances(SBKey key, QString query)
{
    CacheManager* cm=Context::instance()->cacheManager();
    CacheAlbumMgr* amgr=cm->albumMgr();
    CachePerformerMgr* pemgr=cm->performerMgr();
    CacheSongPerformanceMgr* spmgr=cm->songPerformanceMgr();
    CacheAlbumPerformanceMgr* apmgr=cm->albumPerformanceMgr();
    CacheOnlinePerformanceMgr* opmgr=cm->onlinePerformanceMgr();
    CacheSongMgr* smgr=cm->songMgr();
    DataAccessLayer* dal=Context::instance()->dataAccessLayer();
    QVector<SBIDAlbumPerformancePtr> items;
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
    QStringList songFields; songFields                           << "0"  << "1"  << "2"  << "22" << "27";
    QStringList albumFields; albumFields                         << "6"  << "8"  << "7"  << "10" <<  "9" << "11";
    QStringList performerFields; performerFields                 << "12" << "13" << "14" << "15" << "16";
    QStringList albumPerformanceFields; albumPerformanceFields   << "21" << "25" << "6"  << "17" << "18" << "26" << "24";
    QStringList songPerformanceFields; songPerformanceFields     << "25" << "0"  << "12" << "4"  <<  "5" << "28";
    QStringList onlinePerformanceFields; onlinePerformanceFields << "23" << "21" << "20";

    dal->customize(query);
    qDebug() << SB_DEBUG_INFO << key << query;

    QSqlQuery queryList(query,db);

    //	Set up progress dialog
    int progressCurrentValue=0;
    int progressMaxValue=queryList.size();
    if(progressMaxValue<0)
    {
        //	Count items
        while(queryList.next())
        {
            progressMaxValue++;
        }
    }
    ProgressDialog::instance()->update("Preloader::performances",progressCurrentValue,progressMaxValue);

    items.clear();
    queryList.first();
    queryList.previous();
    while(queryList.next())
    {
        SBKey currentKey;
        SBIDAlbumPtr albumPtr;
        SBIDSongPerformancePtr spPtr;
        SBIDAlbumPerformancePtr albumPerformancePtr;
        SBIDOnlinePerformancePtr onlinePerformancePtr;
        SBIDPerformerPtr performerPtr;
        SBIDSongPtr songPtr;

        //	Process song
        if(!queryList.isNull(0))
        {
            currentKey=SBIDSong::createKey(queryList.value(0).toInt());
            if(currentKey.validFlag())
            {
                songPtr=(smgr->contains(currentKey)? smgr->retrieve(currentKey,Cache::open_flag_parentonly): _instantiateSong(smgr,songFields,queryList));
            }
        }

        //	Process album
        if(!queryList.isNull(6))
        {
            currentKey=SBIDAlbum::createKey(queryList.value(6).toInt());
            if(currentKey.validFlag())
            {
                albumPtr=(amgr->contains(currentKey)? amgr->retrieve(currentKey,Cache::open_flag_parentonly): _instantiateAlbum(amgr,albumFields,queryList));
            }
        }

        //	Process performer
        if(!queryList.isNull(12))
        {
            currentKey=SBIDPerformer::createKey(queryList.value(12).toInt());
            if(currentKey.validFlag())
            {
                performerPtr=(pemgr->contains(currentKey)? pemgr->retrieve(currentKey,Cache::open_flag_parentonly): _instantiatePerformer(pemgr,performerFields,queryList));
            }
        }

        //	Process song performance
        if(!queryList.isNull(25))
        {
            currentKey=SBIDSongPerformance::createKey(queryList.value(25).toInt());
            if(currentKey.validFlag())
            {
                spPtr=(spmgr->contains(currentKey)?spmgr->retrieve(currentKey, Cache::open_flag_parentonly):_instantiateSongPerformance(spmgr, songPerformanceFields, queryList));
            }
        }

        //	Process album performance
        if(!queryList.isNull(21))
        {
            currentKey=SBIDAlbumPerformance::createKey(queryList.value(21).toInt());
            if(currentKey.validFlag())
            {
                //	checked on albumPerformanceID
                albumPerformancePtr=(apmgr->contains(currentKey)? apmgr->retrieve(currentKey,Cache::open_flag_parentonly): _instantiateAlbumPerformance(apmgr,albumPerformanceFields,queryList));
            }
        }

        //	Process online performance
        if(!queryList.isNull(21))
        {
            currentKey=SBIDOnlinePerformance::createKey(queryList.value(21).toInt());
            if(currentKey.validFlag())
            {
                //	checked on albumPerformanceID
                onlinePerformancePtr=(opmgr->contains(currentKey)? opmgr->retrieve(currentKey,Cache::open_flag_parentonly): _instantiateOnlinePerformance(opmgr,onlinePerformanceFields,queryList));
            }
        }

        if(albumPerformancePtr)
        {
            items.append(albumPerformancePtr);
        }
        ProgressDialog::instance()->update("Preloader::performances",progressCurrentValue++,progressMaxValue);
    }
    ProgressDialog::instance()->finishStep("Preloader::performances");
    return items;
}

QVector<SBIDOnlinePerformancePtr>
Preloader::onlinePerformances(QString query)
{
    CacheManager* cm=Context::instance()->cacheManager();
    CacheAlbumMgr* amgr=cm->albumMgr();
    CachePerformerMgr* pemgr=cm->performerMgr();
    CacheSongPerformanceMgr* spmgr=cm->songPerformanceMgr();
    CacheAlbumPerformanceMgr* apmgr=cm->albumPerformanceMgr();
    CacheOnlinePerformanceMgr* opmgr=cm->onlinePerformanceMgr();
    CacheSongMgr* smgr=cm->songMgr();
    DataAccessLayer* dal=Context::instance()->dataAccessLayer();
    QVector<SBIDOnlinePerformancePtr> items;
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
    QStringList songFields; songFields                           << "0"  << "1"  << "2"  << "22" << "27";
    QStringList albumFields; albumFields                         << "6"  << "8"  << "7"  << "10" << "11" << "9";
    QStringList performerFields; performerFields                 << "12" << "13" << "14" << "15" << "16";
    QStringList albumPerformanceFields; albumPerformanceFields   << "21" << "25" << "6"  << "17" << "18" << "26" << "24";
    QStringList songPerformanceFields; songPerformanceFields     << "25" << "0"  << "12" << "29" << "4"  << "5"  << "28";
    QStringList onlinePerformanceFields; onlinePerformanceFields << "23" << "21" << "20";

    dal->customize(query);
    qDebug() << SB_DEBUG_INFO << query;

    QSqlQuery queryList(query,db);

    //	Set up progress dialog
    int progressCurrentValue=0;
    int progressMaxValue=queryList.size();
    if(progressMaxValue<0)
    {
        //	Count items
        while(queryList.next())
        {
            progressMaxValue++;
        }
    }
    ProgressDialog::instance()->update("Preloader::onlinePerformances",progressCurrentValue,progressMaxValue);

    items.clear();
    queryList.first();
    queryList.previous();
    while(queryList.next())
    {
        SBKey key;
        SBIDAlbumPtr albumPtr;
        SBIDSongPerformancePtr spPtr;
        SBIDAlbumPerformancePtr albumPerformancePtr;
        SBIDOnlinePerformancePtr onlinePerformancePtr;
        SBIDPerformerPtr performerPtr;
        SBIDSongPtr songPtr;

        //	Process song
        if(!queryList.isNull(0))
        {
            key=SBIDSong::createKey(queryList.value(0).toInt());
            if(key.validFlag())
            {
                songPtr=(smgr->contains(key)? smgr->retrieve(key,Cache::open_flag_parentonly): _instantiateSong(smgr,songFields,queryList));
            }
        }

        //	Process album
        if(!queryList.isNull(6))
        {
            key=SBIDAlbum::createKey(queryList.value(6).toInt());
            if(key.validFlag())
            {
                albumPtr=(amgr->contains(key)? amgr->retrieve(key,Cache::open_flag_parentonly): _instantiateAlbum(amgr,albumFields,queryList));
            }
        }

        //	Process performer
        if(!queryList.isNull(12))
        {
            key=SBIDPerformer::createKey(queryList.value(12).toInt());
            if(key.validFlag())
            {
                performerPtr=(pemgr->contains(key)? pemgr->retrieve(key,Cache::open_flag_parentonly): _instantiatePerformer(pemgr,performerFields,queryList));
            }
        }

        //	Process song performance
        if(!queryList.isNull(25))
        {
            key=SBIDSongPerformance::createKey(queryList.value(25).toInt());
            if(key.validFlag())
            {
                spPtr=(spmgr->contains(key)?spmgr->retrieve(key, Cache::open_flag_parentonly):_instantiateSongPerformance(spmgr, songPerformanceFields, queryList));
            }
        }

        //	Process album performance
        if(!queryList.isNull(21))
        {
            key=SBIDAlbumPerformance::createKey(queryList.value(21).toInt());
            if(key.validFlag())
            {
                //	checked on albumPerformanceID
                albumPerformancePtr=(apmgr->contains(key)? apmgr->retrieve(key,Cache::open_flag_parentonly): _instantiateAlbumPerformance(apmgr,albumPerformanceFields,queryList));
            }
        }

        //	Process online performance
        if(!queryList.isNull(21))
        {
            key=SBIDOnlinePerformance::createKey(queryList.value(21).toInt());
            if(key.validFlag())
            {
                //	checked on albumPerformanceID
                onlinePerformancePtr=(opmgr->contains(key)? opmgr->retrieve(key,Cache::open_flag_parentonly): _instantiateOnlinePerformance(opmgr,onlinePerformanceFields,queryList));
            }
        }

        if(onlinePerformancePtr)
        {
            items.append(onlinePerformancePtr);
        }
        ProgressDialog::instance()->update("Preloader::onlinePerformances",progressCurrentValue++,progressMaxValue);
    }
    ProgressDialog::instance()->finishStep("Preloader::onlinePerformances");
    return items;
}

QMap<int,SBIDAlbumPerformancePtr>
Preloader::performanceMap(QString query)
{
    CacheManager* cm=Context::instance()->cacheManager();
    CacheAlbumMgr* amgr=cm->albumMgr();
    CachePerformerMgr* pemgr=cm->performerMgr();
    CacheSongPerformanceMgr* spmgr=cm->songPerformanceMgr();
    CacheAlbumPerformanceMgr* apmgr=cm->albumPerformanceMgr();
    CacheSongMgr* smgr=cm->songMgr();
    DataAccessLayer* dal=Context::instance()->dataAccessLayer();
    QMap<int,SBIDAlbumPerformancePtr> items;
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
    QStringList songFields; songFields                         << "0"  << "1"  << "2"  << "21" << "24";
    QStringList albumFields; albumFields                       << "6"  << "8"  << "7"  << "10" << "11" << "9";
    QStringList performerFields; performerFields               << "12" << "13" << "14" << "15" << "16";
    QStringList songPerformanceFields; songPerformanceFields   << "22" << "0"  << "3"  << "23" << "4"  << "5"  << "25";
    QStringList albumPerformanceFields; albumPerformanceFields << "20" << "22" << "6"  << "17" << "18" << "19" << "26";

    dal->customize(query);
    qDebug() << SB_DEBUG_INFO << query;
    QSqlQuery queryList(query,db);

    //	Set up progress dialog
    int progressCurrentValue=0;
    int progressMaxValue=queryList.size();
    if(progressMaxValue<0)
    {
        //	Count items
        while(queryList.next())
        {
            progressMaxValue++;
        }
    }
    ProgressDialog::instance()->update("Preloader::performanceMap",progressCurrentValue,progressMaxValue);

    items.clear();
    queryList.first();
    queryList.previous();
    while(queryList.next())
    {
        SBKey key;
        SBIDAlbumPtr albumPtr;
        SBIDSongPerformancePtr songPerformancePtr;
        SBIDAlbumPerformancePtr albumPerformancePtr;
        SBIDPerformerPtr performerPtr;
        SBIDSongPtr songPtr;

        //	Process song
        if(!queryList.isNull(0))
        {
            key=SBIDSong::createKey(queryList.value(0).toInt());
            if(key.validFlag())
            {
                songPtr=(smgr->contains(key)? smgr->retrieve(key,Cache::open_flag_parentonly): _instantiateSong(smgr,songFields,queryList));
            }
        }

        //	Process album
        if(!queryList.isNull(6))
        {
            key=SBIDAlbum::createKey(queryList.value(6).toInt());
            if(key.validFlag())
            {
                albumPtr=(amgr->contains(key)? amgr->retrieve(key,Cache::open_flag_parentonly): _instantiateAlbum(amgr,albumFields,queryList));
            }
        }

        //	Process performer
        if(!queryList.isNull(12))
        {
            key=SBIDPerformer::createKey(queryList.value(12).toInt());
            if(key.validFlag())
            {
                performerPtr=(pemgr->contains(key)? pemgr->retrieve(key,Cache::open_flag_parentonly): _instantiatePerformer(pemgr,performerFields,queryList));
            }
        }

        //	Process song performance
        if(!queryList.isNull(22))
        {
            key=SBIDSongPerformance::createKey(queryList.value(22).toInt());
            if(key.validFlag())
            {
                //	checked on albumPerformanceID
                songPerformancePtr=(spmgr->contains(key)? spmgr->retrieve(key,Cache::open_flag_parentonly): _instantiateSongPerformance(spmgr,songPerformanceFields,queryList));
            }
        }

        //	Process album performance
        if(!queryList.isNull(20))
        {
            key=SBIDAlbumPerformance::createKey(queryList.value(20).toInt());
            if(key.validFlag())
            {
                //	checked on albumPerformanceID
                albumPerformancePtr=(apmgr->contains(key)? apmgr->retrieve(key,Cache::open_flag_parentonly): _instantiateAlbumPerformance(apmgr,albumPerformanceFields,queryList));
            }
        }

        if(albumPerformancePtr)
        {
            items[albumPerformancePtr->albumPerformanceID()]=albumPerformancePtr;
        }

        ProgressDialog::instance()->update("Preloader::performanceMap",progressCurrentValue++,progressMaxValue);
    }
    ProgressDialog::instance()->finishStep("Preloader::performanceMap");
    return items;
}

QMap<int,SBIDPlaylistDetailPtr>
Preloader::playlistItems(int playlistID)
{
    CacheManager* cm=Context::instance()->cacheManager();
    CacheAlbumMgr* amgr=cm->albumMgr();
    CacheAlbumPerformanceMgr* apmgr=cm->albumPerformanceMgr();
    CacheChartMgr* cmgr=cm->chartMgr();
    CachePerformerMgr* pemgr=cm->performerMgr();
    CacheOnlinePerformanceMgr* opmgr=cm->onlinePerformanceMgr();
    CachePlaylistMgr* plmgr=cm->playlistMgr();
    CachePlaylistDetailMgr* pdmgr=cm->playlistDetailMgr();
    CacheSongMgr* smgr=cm->songMgr();
    CacheSongPerformanceMgr* spmgr=cm->songPerformanceMgr();
    QMap<int,SBIDPlaylistDetailPtr> items;
    DataAccessLayer* dal=Context::instance()->dataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
    QStringList songFields; songFields                           << "14" << "15" << "16" << "17" << "18";
    QStringList albumFields; albumFields                         <<  "6" << "22" << "23" << "24" << "25" << "26";
    QStringList performerFields; performerFields                 <<  "7" << "10" << "11" << "12" << "13";
    QStringList songPerformanceFields; songPerformanceFields     << "18" << "14" <<  "7" << "19" << "20" << "21";
    QStringList albumPerformanceFields; albumPerformanceFields   << "21" << "18" <<  "6" << "27" << "28" << "29" <<  "4";
    QStringList onlinePerformanceFields; onlinePerformanceFields <<  "3" << "21" << "36";
    QStringList chartFields; chartFields                         <<  "5" << "30" << "31" << "32" << "33";
    QStringList playlistFields; playlistFields                   <<  "4" << "34" << "35";
    QStringList playlistDetailFields; playlistDetailFields       <<  "0" <<  "1" <<  "2" <<  "3" <<  "4" <<  "5" <<  "6" <<  "7" <<  "8";

    QString q;

    //	Retrieve detail
    q=QString
    (
        "SELECT "
            "pd.playlist_detail_id, "                   //	0
            "pd.playlist_id, "
            "pd.playlist_position, "
            "NULL AS online_performance_id, "
            "pd.child_playlist_id, "

            "pd.chart_id, "                             //	5
            "pd.record_id, "
            "pd.artist_id, "
            "pd.notes, "
            "NULL, "	//	future use

            "a.name, "                                  //	10
            "a.www, "
            "a.notes, "
            "a.mbid, "
            "NULL AS song_id, "

            "NULL AS title, "                           //	15
            "NULL AS notes, "
            "NULL AS lyrics, "
            "NULL AS song_performance_id, "
            "NULL AS year, "

            "NULL AS notes, "                           //	20
            "NULL AS album_performance_id, "
            "r.artist_id, "
            "r.title, "
            "r.genre, "

            "r.year, "                                  //	25
            "r.notes, "
            "NULL AS record_position, "
            "NULL AS duration, "
            "NULL AS notes, "

            "c.name, "                                  //	30
            "c.notes, "
            "c.release_date, "
            "0 AS playlist_num_items, "
            "pl.name, "

            "pl.duration, "                             //	35
            "NULL::VARCHAR "

        "FROM "
            "___SB_SCHEMA_NAME___playlist_detail pd "
                "LEFT JOIN ___SB_SCHEMA_NAME___artist a ON "
                    "pd.artist_id=a.artist_id "
                "LEFT JOIN ___SB_SCHEMA_NAME___record r ON "
                    "pd.record_id=r.record_id "
                "LEFT JOIN ___SB_SCHEMA_NAME___playlist pl ON "
                    "pd.child_playlist_id=pl.playlist_id "
                "LEFT JOIN ___SB_SCHEMA_NAME___chart c ON "
                    "pd.chart_id=c.chart_id "
        "WHERE "
            "pd.online_performance_id IS NULL AND "
            "pd.playlist_id=%1 "
        "UNION "
        "SELECT "
            "pd.playlist_detail_id, "                   //	0
            "pd.playlist_id, "
            "pd.playlist_position, "
            "pd.online_performance_id, "
            "NULL AS child_playlist_id, "

            "NULL AS chart_id, "                        //	5
            "r.record_id, "
            "a.artist_id, "
            "pd.notes, "
            "NULL, "	//	future use

            "a.name, "                                  //	10
            "a.www, "
            "a.notes, "
            "a.mbid, "
            "s.song_id, "

            "s.title, "                                 //	15
            "s.notes, "
            "l.lyrics, "
            "p.performance_id, "
            "p.year, "

            "s.notes, "                                 //	20
            "rp.record_performance_id, "
            "r.artist_id, "
            "r.title, "
            "r.genre, "

            "r.year, "                                  //	25
            "r.notes, "
            "rp.record_position, "
            "rp.duration, "
            "rp.notes, "

            "NULL AS name, "                            //	30
            "NULL AS notes, "
            "NULL AS release_date, "
            "0 AS playlist_num_items, "
            "NULL AS pl_name, "

            "NULL AS pl_duration, "                      //	35
            "op.path "

        "FROM "
            "___SB_SCHEMA_NAME___playlist_detail pd  "
                //	perform a LEFT JOIN ON online_performance to show everything.
                "LEFT JOIN ___SB_SCHEMA_NAME___online_performance op ON "
                    "op.online_performance_id=pd.online_performance_id "
                "JOIN ___SB_SCHEMA_NAME___record_performance rp ON "
                    "op.record_performance_id=rp.record_performance_id "
                "JOIN ___SB_SCHEMA_NAME___record r ON "
                    "rp.record_id=r.record_id "
                "LEFT JOIN ___SB_SCHEMA_NAME___performance p ON "
                    "rp.performance_id=p.performance_id "
                "JOIN ___SB_SCHEMA_NAME___artist a ON "
                    "p.artist_id=a.artist_id "
                "JOIN ___SB_SCHEMA_NAME___song s ON "
                    "p.song_id=s.song_id "
                "LEFT JOIN ___SB_SCHEMA_NAME___lyrics l ON "
                    "s.song_id=l.song_id "
        "WHERE "
            "pd.online_performance_id IS NOT NULL AND "
            "pd.playlist_id=%1 "
        "ORDER BY 3"
    )
            .arg(playlistID)
    ;

    dal->customize(q);
    QSqlQuery queryList(q,db);

    qDebug() << SB_DEBUG_INFO << q;

    //	Set up progress dialog
    int progressCurrentValue=0;
    int progressMaxValue=queryList.size();
    ProgressDialog::instance()->update("Preloader::playlistItems",progressCurrentValue,progressMaxValue);

    int playlistIndex=0;
    items.clear();
    while(queryList.next())
    {
        SBKey key;
        SBIDPtr itemPtr;

        //	Process performer
        key=queryList.isNull(7)?SBKey():SBIDPerformer::createKey(queryList.value(7).toInt());
        if(key.validFlag())
        {
            itemPtr=(pemgr->contains(key)? pemgr->retrieve(key,Cache::open_flag_parentonly): _instantiatePerformer(pemgr,performerFields,queryList));
        }

        //	Process album
        key=queryList.isNull(6)?SBKey():SBIDAlbum::createKey(queryList.value(6).toInt());
        if(key.validFlag())
        {
            itemPtr=(amgr->contains(key)? amgr->retrieve(key,Cache::open_flag_parentonly): _instantiateAlbum(amgr,albumFields,queryList));
        }

        //	Process chart
        key=queryList.isNull(5)?SBKey():SBIDChart::createKey(queryList.value(5).toInt());
        if(key.validFlag())
        {
            itemPtr=(cmgr->contains(key)? cmgr->retrieve(key,Cache::open_flag_parentonly): _instantiateChart(cmgr,chartFields,queryList));
        }

        //	Process playlist
        key=queryList.isNull(4)?SBKey():SBIDPlaylist::createKey(queryList.value(2).toInt());
        if(key.validFlag())
        {
            itemPtr=(plmgr->contains(key)? plmgr->retrieve(key,Cache::open_flag_parentonly): _instantiatePlaylist(plmgr,playlistFields,queryList));
        }

        //	Process performance
        //	Load song in cache
        key=queryList.isNull(3)?SBKey():SBIDSong::createKey(queryList.value(3).toInt());
        if(key.validFlag())
        {
            (smgr->contains(key)? smgr->retrieve(key,Cache::open_flag_parentonly): _instantiateSong(smgr,songFields,queryList));
        }

        //	Load songPerformance in cache
        key=queryList.isNull(18)?SBKey():SBIDSongPerformance::createKey(queryList.value(18).toInt());
        if(key.validFlag())
        {
            (spmgr->contains(key)? spmgr->retrieve(key,Cache::open_flag_parentonly): _instantiateSongPerformance(spmgr,songPerformanceFields,queryList));
        }

        //	Load albumPerformance in cache
        key=queryList.isNull(21)?SBKey():SBIDAlbumPerformance::createKey(queryList.value(21).toInt());
        if(key.validFlag())
        {
            (apmgr->contains(key)? apmgr->retrieve(key,Cache::open_flag_parentonly): _instantiateAlbumPerformance(apmgr,albumPerformanceFields,queryList));
        }

        //	Load onlinePerformance in cache
        key=queryList.isNull(3)?SBKey():SBIDOnlinePerformance::createKey(queryList.value(3).toInt());
        if(key.validFlag())
        {
            itemPtr=(opmgr->contains(key)? opmgr->retrieve(key,Cache::open_flag_parentonly): _instantiateOnlinePerformance(opmgr,onlinePerformanceFields,queryList));
        }

        //	Load playlistDetail in cache
        key=queryList.isNull(0)?SBKey():SBIDPlaylistDetail::createKey(queryList.value(0).toInt());
        SBIDPlaylistDetailPtr pdPtr=(pdmgr->contains(key)? pdmgr->retrieve(key,Cache::open_flag_parentonly): _instantiatePlaylistDetailInstance(pdmgr,playlistDetailFields,queryList));

        if(pdPtr)
        {
            items[playlistIndex++]=pdPtr;
            if(pdPtr->playlistPosition()!=playlistIndex)
            {
                //	playlistIndex is 0 based, playlistPosition is 1 based
                //	Do after increment of playlistIndex
                pdPtr->setPlaylistPosition(playlistIndex);	//	in case of data inconsistencies :)
                cm->saveChanges();
            }
        }
        ProgressDialog::instance()->update("Preloader::playlistItems",progressCurrentValue++,progressMaxValue);
    }
    ProgressDialog::instance()->finishStep("Preloader::playlistItems");
    return items;
}

QVector<SBIDSongPerformancePtr>
Preloader::songPerformances(QString query)
{
    CacheManager* cm=Context::instance()->cacheManager();
    CacheAlbumMgr* amgr=cm->albumMgr();
    CachePerformerMgr* pemgr=cm->performerMgr();
    CacheSongPerformanceMgr* spmgr=cm->songPerformanceMgr();
    CacheAlbumPerformanceMgr* apmgr=cm->albumPerformanceMgr();
    CacheOnlinePerformanceMgr* opmgr=cm->onlinePerformanceMgr();
    CacheSongMgr* smgr=cm->songMgr();
    DataAccessLayer* dal=Context::instance()->dataAccessLayer();
    QVector<SBIDSongPerformancePtr> items;
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
    QStringList songFields; songFields                           << "0"  << "1"  << "2"  << "22" << "27";
    QStringList albumFields; albumFields                         << "6"  << "8"  << "7"  << "10" <<  "9" << "11";
    QStringList performerFields; performerFields                 << "12" << "13" << "14" << "15" << "16";
    QStringList albumPerformanceFields; albumPerformanceFields   << "21" << "25" << "6"  << "17" << "18" << "26" << "24";
    QStringList songPerformanceFields; songPerformanceFields     << "25" <<  "0" << "12" <<  "4" <<  "5" << "28";
    QStringList onlinePerformanceFields; onlinePerformanceFields << "23" << "21" << "20";

    dal->customize(query);
    qDebug() << SB_DEBUG_INFO << query;

    QSqlQuery queryList(query,db);

    //	Set up progress dialog
    int progressCurrentValue=0;
    int progressMaxValue=queryList.size();
    if(progressMaxValue<0)
    {
        //	Count items
        while(queryList.next())
        {
            progressMaxValue++;
        }
    }
    ProgressDialog::instance()->update("Preloader::onlinePerformances",progressCurrentValue,progressMaxValue);

    items.clear();
    queryList.first();
    queryList.previous();
    while(queryList.next())
    {
        SBKey key;
        SBIDAlbumPtr albumPtr;
        SBIDSongPerformancePtr spPtr;
        SBIDAlbumPerformancePtr albumPerformancePtr;
        SBIDOnlinePerformancePtr onlinePerformancePtr;
        SBIDPerformerPtr performerPtr;
        SBIDSongPtr songPtr;

        //	Process song
        if(!queryList.isNull(0))
        {
            key=SBIDSong::createKey(queryList.value(0).toInt());
            if(key.validFlag())
            {
                songPtr=(smgr->contains(key)? smgr->retrieve(key,Cache::open_flag_parentonly): _instantiateSong(smgr,songFields,queryList));
            }
        }

        //	Process album
        if(!queryList.isNull(6))
        {
            key=SBIDAlbum::createKey(queryList.value(6).toInt());
            if(key.validFlag())
            {
                albumPtr=(amgr->contains(key)? amgr->retrieve(key,Cache::open_flag_parentonly): _instantiateAlbum(amgr,albumFields,queryList));
            }
        }

        //	Process performer
        if(!queryList.isNull(12))
        {
            key=SBIDPerformer::createKey(queryList.value(12).toInt());
            if(key.validFlag())
            {
                performerPtr=(pemgr->contains(key)? pemgr->retrieve(key,Cache::open_flag_parentonly): _instantiatePerformer(pemgr,performerFields,queryList));
            }
        }

        //	Process song performance
        if(!queryList.isNull(25))
        {
            key=SBIDSongPerformance::createKey(queryList.value(25).toInt());
            if(key.validFlag())
            {
                spPtr=(spmgr->contains(key)?spmgr->retrieve(key, Cache::open_flag_parentonly):_instantiateSongPerformance(spmgr, songPerformanceFields, queryList));
            }
        }

        //	Process album performance
        if(!queryList.isNull(21))
        {
            key=SBIDAlbumPerformance::createKey(queryList.value(21).toInt());
            if(key.validFlag())
            {
                //	checked on albumPerformanceID
                albumPerformancePtr=(apmgr->contains(key)? apmgr->retrieve(key,Cache::open_flag_parentonly): _instantiateAlbumPerformance(apmgr,albumPerformanceFields,queryList));
            }
        }

        //	Process online performance
        if(!queryList.isNull(21))
        {
            key=SBIDOnlinePerformance::createKey(queryList.value(21).toInt());
            if(key.validFlag())
            {
                //	checked on albumPerformanceID
                onlinePerformancePtr=(opmgr->contains(key)? opmgr->retrieve(key,Cache::open_flag_parentonly): _instantiateOnlinePerformance(opmgr,onlinePerformanceFields,queryList));
            }
        }

        if(spPtr)
        {
            items.append(spPtr);
        }
        ProgressDialog::instance()->update("Preloader::onlinePerformances",progressCurrentValue++,progressMaxValue);
    }
    ProgressDialog::instance()->finishStep("Preloader::onlinePerformances");
    return items;
}

void
Preloader::loadAll()
{
    loadAllSongs();
    loadAllPerformers();
}

void
Preloader::loadAllSongs()
{
    CacheManager* cm=Context::instance()->cacheManager();
    CacheSongMgr* mgr=cm->songMgr();
    QSqlQueryModel* qm=SBIDSong::retrieveSQL();
    SB_RETURN_VOID_IF_NULL(qm);
    for(int i=0;i<qm->rowCount();i++)
    {
        SBIDSongPtr Ptr=SBIDSong::instantiate(qm->record(i));
        mgr->addItem(Ptr);
    }
}

void
Preloader::loadAllPerformers()
{
    CacheManager* cm=Context::instance()->cacheManager();
    CachePerformerMgr* mgr=cm->performerMgr();
    QSqlQueryModel* qm=SBIDPerformer::retrieveSQL();
    SB_RETURN_VOID_IF_NULL(qm);
    for(int i=0;i<qm->rowCount();i++)
    {
        SBIDPerformerPtr sPtr=SBIDPerformer::instantiate(qm->record(i));
        mgr->addItem(sPtr);
    }
}


///	Private methods
SBIDAlbumPtr
Preloader::_instantiateAlbum(CacheAlbumMgr* amgr, const QStringList& fields, const QSqlQuery& queryList)
{
    SBIDAlbumPtr albumPtr=SBIDAlbum::instantiate(_populate(fields,queryList));
    amgr->addItem(albumPtr);
    return albumPtr;
}

SBIDChartPtr
Preloader::_instantiateChart(CacheChartMgr* cmgr, const QStringList& fields, const QSqlQuery& queryList)
{
    SBIDChartPtr cPtr=SBIDChart::instantiate(_populate(fields,queryList));
    cmgr->addItem(cPtr);
    return cPtr;
}

SBIDChartPerformancePtr
Preloader::_instantiateChartPerformance(CacheChartPerformanceMgr* cpmgr, const QStringList& fields, const QSqlQuery& queryList)
{
    SBIDChartPerformancePtr cpPtr=SBIDChartPerformance::instantiate(_populate(fields,queryList));
    cpmgr->addItem(cpPtr);
    return cpPtr;
}

SBIDPlaylistPtr
Preloader::_instantiatePlaylist(CachePlaylistMgr *pdmgr, const QStringList &fields, const QSqlQuery &queryList)
{
    SBIDPlaylistPtr pPtr=SBIDPlaylist::instantiate(_populate(fields,queryList));
    pdmgr->addItem(pPtr);
    return pPtr;
}

SBIDPlaylistDetailPtr
Preloader::_instantiatePlaylistDetailInstance(CachePlaylistDetailMgr *pdmgr, const QStringList &fields, const QSqlQuery &queryList)
{
    SBIDPlaylistDetailPtr pdPtr=SBIDPlaylistDetail::instantiate(_populate(fields,queryList));
    pdmgr->addItem(pdPtr);
    return pdPtr;
}

SBIDSongPerformancePtr
Preloader::_instantiateSongPerformance(CacheSongPerformanceMgr* spmgr, const QStringList& fields, const QSqlQuery& queryList)
{
    SBIDSongPerformancePtr performancePtr=SBIDSongPerformance::instantiate(_populate(fields,queryList));
    spmgr->addItem(performancePtr);
    return performancePtr;
}

SBIDAlbumPerformancePtr
Preloader::_instantiateAlbumPerformance(CacheAlbumPerformanceMgr* apmgr, const QStringList& fields, const QSqlQuery& queryList)
{
    SBIDAlbumPerformancePtr performancePtr=SBIDAlbumPerformance::instantiate(_populate(fields,queryList));
    apmgr->addItem(performancePtr);
    return performancePtr;
}

SBIDOnlinePerformancePtr
Preloader::_instantiateOnlinePerformance(CacheOnlinePerformanceMgr* opmgr, const QStringList& fields, const QSqlQuery& queryList)
{
    SBIDOnlinePerformancePtr performancePtr=SBIDOnlinePerformance::instantiate(_populate(fields,queryList));
    opmgr->addItem(performancePtr);
    return performancePtr;
}

SBIDPerformerPtr
Preloader::_instantiatePerformer(CachePerformerMgr* pemgr, const QStringList& fields, const QSqlQuery& queryList)
{
    QSqlRecord r;
    QSqlField f;

    f=QSqlField("f1",QVariant::Int);    f.setValue(queryList.value(fields.at(0).toInt()).toInt());    r.append(f);
    f=QSqlField("f2",QVariant::String); f.setValue(queryList.value(fields.at(1).toInt()).toString()); r.append(f);
    f=QSqlField("f3",QVariant::String); f.setValue(queryList.value(fields.at(2).toInt()).toString()); r.append(f);
    f=QSqlField("f4",QVariant::String); f.setValue(queryList.value(fields.at(3).toInt()).toString()); r.append(f);
    f=QSqlField("f5",QVariant::String); f.setValue(queryList.value(fields.at(4).toInt()).toString()); r.append(f);

    SBIDPerformerPtr performerPtr=SBIDPerformer::instantiate(_populate(fields,queryList));
    pemgr->addItem(performerPtr);
    return performerPtr;
}

SBIDSongPtr
Preloader::_instantiateSong(CacheSongMgr* smgr, const QStringList& fields, const QSqlQuery& queryList)
{
    QSqlRecord r;
    QSqlField f;

    f=QSqlField("f1",QVariant::Int);    f.setValue(queryList.value(fields.at(0).toInt()).toInt());    r.append(f);
    f=QSqlField("f2",QVariant::String); f.setValue(queryList.value(fields.at(1).toInt()).toString()); r.append(f);
    f=QSqlField("f3",QVariant::String); f.setValue(queryList.value(fields.at(2).toInt()).toString()); r.append(f);
    f=QSqlField("f4",QVariant::String); f.setValue(queryList.value(fields.at(3).toInt()).toString()); r.append(f);
    f=QSqlField("f5",QVariant::Int);    f.setValue(queryList.value(fields.at(4).toInt()).toInt());    r.append(f);

    SBIDSongPtr songPtr=SBIDSong::instantiate(_populate(fields,queryList));
    smgr->addItem(songPtr);
    return songPtr;
}

QSqlRecord
Preloader::_populate(const QStringList& fields, const QSqlQuery& queryList)
{
    QSqlRecord r;

    for(int i=0;i<fields.length();i++)
    {
        QVariant v=queryList.value(fields.at(i).toInt());
        QSqlField f=QSqlField(QString("f%1").arg(i+1),v.type());
        f.setValue(v);
        r.append(f);
    }
    return r;
}
