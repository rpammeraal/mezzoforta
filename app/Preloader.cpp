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
    SBIDPerformerMgr* pemgr=Context::instance()->getPerformerMgr();
    SBIDChartMgr* cmgr=Context::instance()->getChartMgr();
    SBIDChartPerformanceMgr* cpmgr=Context::instance()->getChartPerformanceMgr();
    SBIDSongPerformanceMgr* spmgr=Context::instance()->getSongPerformanceMgr();
    SBIDSongMgr* smgr=Context::instance()->getSongMgr();
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QMap<SBIDChartPerformancePtr,SBIDChartPtr> items;
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());

    QString whereClause;
    switch(id.itemType())
    {
        case SBIDBase::sb_type_song:
            whereClause=QString("p.song_id=%1").arg(id.itemID());
            break;
        case SBIDBase::sb_type_performer:
            whereClause=QString("a.artist_id=%1").arg(id.itemID());
            break;
        case SBIDBase::sb_type_chart:
            whereClause=QString("cp.chart_id=%1").arg(id.itemID());
            break;

        case SBIDBase::sb_type_invalid:
        case SBIDBase::sb_type_album:
        case SBIDBase::sb_type_playlist:
        case SBIDBase::sb_type_song_performance:
        case SBIDBase::sb_type_album_performance:
        case SBIDBase::sb_type_online_performance:
        case SBIDBase::sb_type_chart_performance:
        case SBIDBase::sb_type_playlist_detail:
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
        QString key;
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
            if(key.length()>0)
            {
                chartPtr=(cmgr->contains(key)? cmgr->retrieve(key,SBIDChartMgr::open_flag_parentonly): _instantiateChart(cmgr,chartFields,queryList));
            }
        }

        //	Process chart performance
        if(!queryList.isNull(4))
        {
            key=SBIDChartPerformance::createKey(queryList.value(4).toInt());
            if(key.length()>0)
            {
                chartPerformancePtr=(cpmgr->contains(key)? cpmgr->retrieve(key,SBIDChartPerformanceMgr::open_flag_parentonly): _instantiateChartPerformance(cpmgr,chartPerformanceFields,queryList));
            }
        }

        //	Process performer
        if(!queryList.isNull(9))
        {
            key=SBIDPerformer::createKey(queryList.value(9).toInt());
            if(key.length()>0)
            {
                performerPtr=(pemgr->contains(key)? pemgr->retrieve(key,SBIDPerformerMgr::open_flag_parentonly): _instantiatePerformer(pemgr,performerFields,queryList));
            }
        }

        //	Process song
        if(!queryList.isNull(8))
        {
            key=SBIDSong::createKey(queryList.value(8).toInt());
            if(key.length()>0)
            {
                songPtr=(smgr->contains(key)? smgr->retrieve(key,SBIDSongMgr::open_flag_parentonly): _instantiateSong(smgr,songFields,queryList));
            }
        }

        //	Process song performance
        if(!queryList.isNull(5))
        {
            key=SBIDSongPerformance::createKey(queryList.value(5).toInt());
            if(key.length()>0)
            {
                songPerformancePtr=(spmgr->contains(key)? spmgr->retrieve(key,SBIDSongPerformanceMgr::open_flag_parentonly): _instantiateSongPerformance(spmgr,songPerformanceFields,queryList));
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
Preloader::albumPerformances(QString query)
{
    SBIDAlbumMgr* amgr=Context::instance()->getAlbumMgr();
    SBIDPerformerMgr* pemgr=Context::instance()->getPerformerMgr();
    SBIDSongPerformanceMgr* spmgr=Context::instance()->getSongPerformanceMgr();
    SBIDAlbumPerformanceMgr* apmgr=Context::instance()->getAlbumPerformanceMgr();
    SBIDOnlinePerformanceMgr* opmgr=Context::instance()->getOnlinePerformanceMgr();
    SBIDSongMgr* smgr=Context::instance()->getSongMgr();
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QVector<SBIDAlbumPerformancePtr> items;
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
    QStringList songFields; songFields                           << "0"  << "1"  << "2"  << "22" << "27";
    QStringList albumFields; albumFields                         << "6"  << "8"  << "7"  << "10" <<  "9" << "11";
    QStringList performerFields; performerFields                 << "12" << "13" << "14" << "15" << "16";
    QStringList albumPerformanceFields; albumPerformanceFields   << "21" << "25" << "6"  << "17" << "18" << "26" << "24";
    QStringList songPerformanceFields; songPerformanceFields     << "25" << "0"  << "12" << "4"  <<  "5" << "28";
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
    ProgressDialog::instance()->update("Preloader::performances",progressCurrentValue,progressMaxValue);

    items.clear();
    queryList.first();
    queryList.previous();
    while(queryList.next())
    {
        QString key;
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

        //	Process song performance
        if(!queryList.isNull(25))
        {
            key=SBIDSongPerformance::createKey(queryList.value(25).toInt());
            if(key.length()>0)
            {
                spPtr=(spmgr->contains(key)?spmgr->retrieve(key, SBIDSongPerformanceMgr::open_flag_parentonly):_instantiateSongPerformance(spmgr, songPerformanceFields, queryList));
            }
        }

        //	Process album performance
        if(!queryList.isNull(21))
        {
            key=SBIDAlbumPerformance::createKey(queryList.value(21).toInt());
            if(key.length()>0)
            {
                //	checked on albumPerformanceID
                albumPerformancePtr=(apmgr->contains(key)? apmgr->retrieve(key,SBIDAlbumPerformanceMgr::open_flag_parentonly): _instantiateAlbumPerformance(apmgr,albumPerformanceFields,queryList));
            }
        }

        //	Process online performance
        if(!queryList.isNull(21))
        {
            key=SBIDOnlinePerformance::createKey(queryList.value(21).toInt());
            if(key.length()>0)
            {
                //	checked on albumPerformanceID
                onlinePerformancePtr=(opmgr->contains(key)? opmgr->retrieve(key,SBIDOnlinePerformanceMgr::open_flag_parentonly): _instantiateOnlinePerformance(opmgr,onlinePerformanceFields,queryList));
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
    SBIDAlbumMgr* amgr=Context::instance()->getAlbumMgr();
    SBIDPerformerMgr* pemgr=Context::instance()->getPerformerMgr();
    SBIDSongPerformanceMgr* spmgr=Context::instance()->getSongPerformanceMgr();
    SBIDAlbumPerformanceMgr* apmgr=Context::instance()->getAlbumPerformanceMgr();
    SBIDOnlinePerformanceMgr* opmgr=Context::instance()->getOnlinePerformanceMgr();
    SBIDSongMgr* smgr=Context::instance()->getSongMgr();
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
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
        QString key;
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

        //	Process song performance
        if(!queryList.isNull(25))
        {
            key=SBIDSongPerformance::createKey(queryList.value(25).toInt());
            if(key.length()>0)
            {
                spPtr=(spmgr->contains(key)?spmgr->retrieve(key, SBIDSongPerformanceMgr::open_flag_parentonly):_instantiateSongPerformance(spmgr, songPerformanceFields, queryList));
            }
        }

        //	Process album performance
        if(!queryList.isNull(21))
        {
            key=SBIDAlbumPerformance::createKey(queryList.value(21).toInt());
            if(key.length()>0)
            {
                //	checked on albumPerformanceID
                albumPerformancePtr=(apmgr->contains(key)? apmgr->retrieve(key,SBIDAlbumPerformanceMgr::open_flag_parentonly): _instantiateAlbumPerformance(apmgr,albumPerformanceFields,queryList));
            }
        }

        //	Process online performance
        if(!queryList.isNull(21))
        {
            key=SBIDOnlinePerformance::createKey(queryList.value(21).toInt());
            if(key.length()>0)
            {
                //	checked on albumPerformanceID
                onlinePerformancePtr=(opmgr->contains(key)? opmgr->retrieve(key,SBIDOnlinePerformanceMgr::open_flag_parentonly): _instantiateOnlinePerformance(opmgr,onlinePerformanceFields,queryList));
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
    SBIDAlbumMgr* amgr=Context::instance()->getAlbumMgr();
    SBIDPerformerMgr* pemgr=Context::instance()->getPerformerMgr();
    SBIDSongPerformanceMgr* spmgr=Context::instance()->getSongPerformanceMgr();
    SBIDAlbumPerformanceMgr* apmgr=Context::instance()->getAlbumPerformanceMgr();
    SBIDSongMgr* smgr=Context::instance()->getSongMgr();
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
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
        QString key;
        QSqlRecord r;
        QSqlField f;
        SBIDAlbumPtr albumPtr;
        SBIDSongPerformancePtr songPerformancePtr;
        SBIDAlbumPerformancePtr albumPerformancePtr;
        SBIDOnlinePerformancePtr onlinePerformancePtr;
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

        //	Process song performance
        if(!queryList.isNull(22))
        {
            key=SBIDSongPerformance::createKey(queryList.value(22).toInt());
            if(key.length()>0)
            {
                //	checked on albumPerformanceID
                songPerformancePtr=(spmgr->contains(key)? spmgr->retrieve(key,SBIDSongPerformanceMgr::open_flag_parentonly): _instantiateSongPerformance(spmgr,songPerformanceFields,queryList));
            }
        }

        //	Process album performance
        if(!queryList.isNull(20))
        {
            key=SBIDAlbumPerformance::createKey(queryList.value(20).toInt());
            if(key.length()>0)
            {
                //	checked on albumPerformanceID
                albumPerformancePtr=(apmgr->contains(key)? apmgr->retrieve(key,SBIDAlbumPerformanceMgr::open_flag_parentonly): _instantiateAlbumPerformance(apmgr,albumPerformanceFields,queryList));
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
    SBIDAlbumMgr* amgr=Context::instance()->getAlbumMgr();
    SBIDAlbumPerformanceMgr* apmgr=Context::instance()->getAlbumPerformanceMgr();
    SBIDChartMgr* cmgr=Context::instance()->getChartMgr();
    SBIDPerformerMgr* pemgr=Context::instance()->getPerformerMgr();
    SBIDOnlinePerformanceMgr* opmgr=Context::instance()->getOnlinePerformanceMgr();
    SBIDPlaylistMgr* plmgr=Context::instance()->getPlaylistMgr();
    SBIDPlaylistDetailMgr* pdmgr=Context::instance()->getPlaylistDetailMgr();
    SBIDSongMgr* smgr=Context::instance()->getSongMgr();
    SBIDSongPerformanceMgr* spmgr=Context::instance()->getSongPerformanceMgr();
    QMap<int,SBIDPlaylistDetailPtr> items;
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
    QStringList songFields; songFields                           << "14" << "15" << "16" << "17" << "18";
    QStringList albumFields; albumFields                         <<  "6" << "22" << "23" << "24" << "25" << "26";
    QStringList performerFields; performerFields                 <<  "7" << "10" << "11" << "12" << "13";
    QStringList songPerformanceFields; songPerformanceFields     << "18" << "14" <<  "7" << "19" << "20" << "21";
    QStringList albumPerformanceFields; albumPerformanceFields   << "21" << "18" <<  "6" << "27" << "28" << "29" <<  "4";
    QStringList onlinePerformanceFields; onlinePerformanceFields <<  "3" << "21" << "36";
    QStringList chartFields; chartFields                         <<  "5" << "30" << "31" << "32" << "33";
    QStringList playlistFields; playlistFields                   <<  "2" << "34" << "35" << "33";
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
//            .arg(Common::sb_field_playlist_id)
//            .arg(Common::sb_field_album_id)
//            .arg(Common::sb_field_performer_id)
//            .arg(Common::sb_field_online_performance_id)
//            .arg(Common::sb_field_chart_id)
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
        QString key;
        SBIDPtr itemPtr;

        //	Process performer
        key=queryList.isNull(7)?QString():SBIDPerformer::createKey(queryList.value(7).toInt());
        if(key.length()>0)
        {
            itemPtr=(pemgr->contains(key)? pemgr->retrieve(key,SBIDPerformerMgr::open_flag_parentonly): _instantiatePerformer(pemgr,performerFields,queryList));
        }

        //	Process album
        key=queryList.isNull(6)?QString():SBIDAlbum::createKey(queryList.value(6).toInt());
        if(key.length()>0)
        {
            itemPtr=(amgr->contains(key)? amgr->retrieve(key,SBIDAlbumMgr::open_flag_parentonly): _instantiateAlbum(amgr,albumFields,queryList));
        }

        //	Process chart
        key=queryList.isNull(5)?QString():SBIDChart::createKey(queryList.value(5).toInt());
        if(key.length()>0)
        {
            itemPtr=(cmgr->contains(key)? cmgr->retrieve(key,SBIDChartMgr::open_flag_parentonly): _instantiateChart(cmgr,chartFields,queryList));
        }

        //	Process playlist
        key=queryList.isNull(2)?QString():SBIDPlaylist::createKey(queryList.value(2).toInt());
        if(key.length()>0)
        {
            itemPtr=(plmgr->contains(key)? plmgr->retrieve(key,SBIDPlaylistMgr::open_flag_parentonly): _instantiatePlaylist(plmgr,playlistFields,queryList));
        }

        //	Process performance
        //	Load song in cache
        key=queryList.isNull(3)?QString():SBIDSong::createKey(queryList.value(3).toInt());
        if(key.length()>0)
        {
            (smgr->contains(key)? smgr->retrieve(key,SBIDSongMgr::open_flag_parentonly): _instantiateSong(smgr,songFields,queryList));
        }

        //	Load songPerformance in cache
        key=queryList.isNull(18)?QString():SBIDSongPerformance::createKey(queryList.value(18).toInt());
        if(key.length()>0)
        {
            (spmgr->contains(key)? spmgr->retrieve(key,SBIDSongPerformanceMgr::open_flag_parentonly): _instantiateSongPerformance(spmgr,songPerformanceFields,queryList));
        }

        //	Load albumPerformance in cache
        key=queryList.isNull(21)?QString():SBIDAlbumPerformance::createKey(queryList.value(21).toInt());
        if(key.length()>0)
        {
            (apmgr->contains(key)? apmgr->retrieve(key,SBIDAlbumPerformanceMgr::open_flag_parentonly): _instantiateAlbumPerformance(apmgr,albumPerformanceFields,queryList));
        }

        //	Load onlinePerformance in cache
        key=queryList.isNull(3)?QString():SBIDOnlinePerformance::createKey(queryList.value(3).toInt());
        if(key.length()>0)
        {
            itemPtr=(opmgr->contains(key)? opmgr->retrieve(key,SBIDOnlinePerformanceMgr::open_flag_parentonly): _instantiateOnlinePerformance(opmgr,onlinePerformanceFields,queryList));
        }

        //	Load playlistDetail in cache
        key=queryList.isNull(0)?QString():SBIDPlaylistDetail::createKey(queryList.value(0).toInt());
        SBIDPlaylistDetailPtr pdPtr=(pdmgr->contains(key)? pdmgr->retrieve(key,SBIDPlaylistDetailMgr::open_flag_parentonly): _instantiatePlaylistDetailInstance(pdmgr,playlistDetailFields,queryList));

        if(pdPtr)
        {
            items[playlistIndex++]=pdPtr;
            if(pdPtr->playlistPosition()!=playlistIndex)
            {
                //	playlistIndex is 0 based, playlistPosition is 1 based
                //	Do after increment of playlistIndex
                pdPtr->setPlaylistPosition(playlistIndex);	//	in case of data inconsistencies :)
                pdmgr->commit(pdPtr,dal);
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
    SBIDAlbumMgr* amgr=Context::instance()->getAlbumMgr();
    SBIDPerformerMgr* pemgr=Context::instance()->getPerformerMgr();
    SBIDSongPerformanceMgr* spmgr=Context::instance()->getSongPerformanceMgr();
    SBIDAlbumPerformanceMgr* apmgr=Context::instance()->getAlbumPerformanceMgr();
    SBIDOnlinePerformanceMgr* opmgr=Context::instance()->getOnlinePerformanceMgr();
    SBIDSongMgr* smgr=Context::instance()->getSongMgr();
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    QVector<SBIDSongPerformancePtr> items;
    QSqlDatabase db=QSqlDatabase::database(dal->getConnectionName());
    QStringList songFields; songFields                           << "0"  << "1"  << "2"  << "22" << "27";
    QStringList albumFields; albumFields                         << "6"  << "8"  << "7"  << "10" <<  "9" << "11";
    QStringList performerFields; performerFields                 << "12" << "13" << "14" << "15" << "16";
    QStringList albumPerformanceFields; albumPerformanceFields   << "21" << "25" << "6"  << "17" << "18" << "26" << "24";
    QStringList songPerformanceFields; songPerformanceFields     << "25" << "0"  << "12" << "29" <<  "4" <<  "5" << "28";
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
        QString key;
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

        //	Process song performance
        if(!queryList.isNull(25))
        {
            key=SBIDSongPerformance::createKey(queryList.value(25).toInt());
            if(key.length()>0)
            {
                spPtr=(spmgr->contains(key)?spmgr->retrieve(key, SBIDSongPerformanceMgr::open_flag_parentonly):_instantiateSongPerformance(spmgr, songPerformanceFields, queryList));
            }
        }

        //	Process album performance
        if(!queryList.isNull(21))
        {
            key=SBIDAlbumPerformance::createKey(queryList.value(21).toInt());
            if(key.length()>0)
            {
                //	checked on albumPerformanceID
                albumPerformancePtr=(apmgr->contains(key)? apmgr->retrieve(key,SBIDAlbumPerformanceMgr::open_flag_parentonly): _instantiateAlbumPerformance(apmgr,albumPerformanceFields,queryList));
            }
        }

        //	Process online performance
        if(!queryList.isNull(21))
        {
            key=SBIDOnlinePerformance::createKey(queryList.value(21).toInt());
            if(key.length()>0)
            {
                //	checked on albumPerformanceID
                onlinePerformancePtr=(opmgr->contains(key)? opmgr->retrieve(key,SBIDOnlinePerformanceMgr::open_flag_parentonly): _instantiateOnlinePerformance(opmgr,onlinePerformanceFields,queryList));
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

///	Private methods
SBIDAlbumPtr
Preloader::_instantiateAlbum(SBIDAlbumMgr* amgr, const QStringList& fields, const QSqlQuery& queryList)
{
    SBIDAlbumPtr albumPtr=SBIDAlbum::instantiate(_populate(fields,queryList));
    amgr->addItem(albumPtr);
    return albumPtr;
}

SBIDChartPtr
Preloader::_instantiateChart(SBIDChartMgr* cmgr, const QStringList& fields, const QSqlQuery& queryList)
{
    SBIDChartPtr cPtr=SBIDChart::instantiate(_populate(fields,queryList));
    cmgr->addItem(cPtr);
    return cPtr;
}

SBIDChartPerformancePtr
Preloader::_instantiateChartPerformance(SBIDChartPerformanceMgr* cpmgr, const QStringList& fields, const QSqlQuery& queryList)
{
    SBIDChartPerformancePtr cpPtr=SBIDChartPerformance::instantiate(_populate(fields,queryList));
    cpmgr->addItem(cpPtr);
    return cpPtr;
}

SBIDPlaylistPtr
Preloader::_instantiatePlaylist(SBIDPlaylistMgr *pdmgr, const QStringList &fields, const QSqlQuery &queryList)
{
    SBIDPlaylistPtr pPtr=SBIDPlaylist::instantiate(_populate(fields,queryList));
    pdmgr->addItem(pPtr);
    return pPtr;
}

SBIDPlaylistDetailPtr
Preloader::_instantiatePlaylistDetailInstance(SBIDPlaylistDetailMgr *pdmgr, const QStringList &fields, const QSqlQuery &queryList)
{
    SBIDPlaylistDetailPtr pdPtr=SBIDPlaylistDetail::instantiate(_populate(fields,queryList));
    pdmgr->addItem(pdPtr);
    return pdPtr;
}

SBIDSongPerformancePtr
Preloader::_instantiateSongPerformance(SBIDSongPerformanceMgr* spmgr, const QStringList& fields, const QSqlQuery& queryList)
{
    SBIDSongPerformancePtr performancePtr=SBIDSongPerformance::instantiate(_populate(fields,queryList));
    spmgr->addItem(performancePtr);
    return performancePtr;
}

SBIDAlbumPerformancePtr
Preloader::_instantiateAlbumPerformance(SBIDAlbumPerformanceMgr* apmgr, const QStringList& fields, const QSqlQuery& queryList)
{
    SBIDAlbumPerformancePtr performancePtr=SBIDAlbumPerformance::instantiate(_populate(fields,queryList));
    apmgr->addItem(performancePtr);
    return performancePtr;
}

SBIDOnlinePerformancePtr
Preloader::_instantiateOnlinePerformance(SBIDOnlinePerformanceMgr* opmgr, const QStringList& fields, const QSqlQuery& queryList)
{
    SBIDOnlinePerformancePtr performancePtr=SBIDOnlinePerformance::instantiate(_populate(fields,queryList));
    opmgr->addItem(performancePtr);
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

    SBIDPerformerPtr performerPtr=SBIDPerformer::instantiate(_populate(fields,queryList));
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
