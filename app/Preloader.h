#ifndef PRELOADER_H
#define PRELOADER_H

#include <QSqlQuery>

#include "SBIDBase.h"
#include "SBIDOnlinePerformance.h"
#include "CacheManager.h"

class Preloader
{
public:
    //	CWIP: this should be templatized
    static QMap<SBIDChartPerformancePtr,SBIDChartPtr> chartItems(const SBIDBase& id);
    static QVector<SBIDAlbumPerformancePtr> albumPerformances(QString query);
    static QVector<SBIDOnlinePerformancePtr> onlinePerformances(QString query);
    static QMap<int,SBIDAlbumPerformancePtr> performanceMap(QString query);
    static QMap<int,SBIDPlaylistDetailPtr> playlistItems(int playlistID);
    static QVector<SBIDSongPerformancePtr> songPerformances(QString query);

    static void loadAll();
    static void loadAllSongs();
    static void loadAllPerformers();

private:
    static SBIDAlbumPtr _instantiateAlbum(SBIDAlbumMgr* amgr,const QStringList& fields, const QSqlQuery& queryList);
    static SBIDChartPtr _instantiateChart(SBIDChartMgr* spmgr,const QStringList& fields, const QSqlQuery& queryList);
    static SBIDChartPerformancePtr _instantiateChartPerformance(SBIDChartPerformanceMgr* spmgr,const QStringList& fields, const QSqlQuery& queryList);
    static SBIDPlaylistPtr _instantiatePlaylist(SBIDPlaylistMgr* plmgr,const QStringList& fields, const QSqlQuery& queryList);
    static SBIDPlaylistDetailPtr _instantiatePlaylistDetailInstance(SBIDPlaylistDetailMgr* pdmgr,const QStringList& fields, const QSqlQuery& queryList);
    static SBIDSongPerformancePtr _instantiateSongPerformance(SBIDSongPerformanceMgr* spmgr,const QStringList& fields, const QSqlQuery& queryList);
    static SBIDAlbumPerformancePtr _instantiateAlbumPerformance(SBIDAlbumPerformanceMgr* apmgr,const QStringList& fields, const QSqlQuery& queryList);
    static SBIDOnlinePerformancePtr _instantiateOnlinePerformance(SBIDOnlinePerformanceMgr* apmgr,const QStringList& fields, const QSqlQuery& queryList);
    static SBIDPerformerPtr _instantiatePerformer(SBIDPerformerMgr* pemgr,const QStringList& fields, const QSqlQuery& queryList);
    static SBIDSongPtr _instantiateSong(SBIDSongMgr* smgr,const QStringList& fields, const QSqlQuery& queryList);

    static QSqlRecord _populate(const QStringList& fields, const QSqlQuery& queryList);
};

#endif // PRELOADER_H
