#ifndef PRELOADER_H
#define PRELOADER_H

#include <QSqlQuery>

#include "SBIDBase.h"
#include "SBIDOnlinePerformance.h"
#include "Context.h"

class Preloader
{
public:
    static QVector<SBIDAlbumPerformancePtr> performances(QString query, bool showProgressDialogFlag);
    static QMap<int,SBIDAlbumPerformancePtr> performanceMap(QString query, bool showProgressDialogFlag);
    static QMap<int,SBIDPtr> playlistItems(int playlistID,bool showProgressDialogFlag);

private:
    static SBIDAlbumPtr _instantiateAlbum(SBIDAlbumMgr* amgr,const QStringList& fields, const QSqlQuery& queryList);
    static SBIDSongPerformancePtr _instantiateSongPerformance(SBIDSongPerformanceMgr* spmgr,const QStringList& fields, const QSqlQuery& queryList);
    static SBIDAlbumPerformancePtr _instantiateAlbumPerformance(SBIDAlbumPerformanceMgr* apmgr,const QStringList& fields, const QSqlQuery& queryList);
    static SBIDOnlinePerformancePtr _instantiateOnlinePerformance(SBIDOnlinePerformanceMgr* apmgr,const QStringList& fields, const QSqlQuery& queryList);
    static SBIDPerformerPtr _instantiatePerformer(SBIDPerformerMgr* pemgr,const QStringList& fields, const QSqlQuery& queryList);
    static SBIDSongPtr _instantiateSong(SBIDSongMgr* smgr,const QStringList& fields, const QSqlQuery& queryList);
};

#endif // PRELOADER_H
