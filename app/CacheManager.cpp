#include <QCompleter>

#include "CacheManager.h"
#include "CacheTemplate.h"
#include "Context.h"
#include "Controller.h"
#include "MainWindow.h"
#include "SearchItemModel.h"

CacheManager::CacheManager()
{
    _init();
}

void
CacheManager::clearAllCaches()
{
    QMapIterator<sb_cache_type,CachePtr> cIT(_cache);
    while(cIT.hasNext())
    {
        cIT.next();
        CachePtr cPtr=cIT.value();
        cPtr->clearCache();
    }
}

void
CacheManager::debugShowChanges(const QString &title)
{
    qDebug() << SB_DEBUG_INFO << title;
    QMapIterator<sb_cache_type,CachePtr> cIT(_cache); while(cIT.hasNext())
    {
        cIT.next();
        CachePtr cPtr=cIT.value();
        cPtr->debugShowChanges();
    }
}

bool
CacheManager::saveChanges()
{
    QStringList updateSQL;
    QStringList insertSQL;
    QStringList deleteSQL;
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();

    QMapIterator<sb_cache_type,CachePtr> cIT(_cache); while(cIT.hasNext())
    {
        cIT.next();
        CachePtr cPtr=cIT.value();
        insertSQL.append(cPtr->retrieveChanges(Common::db_insert));
        updateSQL.append(cPtr->retrieveChanges(Common::db_update));
        deleteSQL.append(cPtr->retrieveChanges(Common::db_delete));
    }

    QStringList SQL=updateSQL;
    SQL.append(insertSQL);
    SQL.append(deleteSQL);

    QString restorePoint=dal->createRestorePoint();
    bool resultFlag=dal->executeBatch(SQL);
    if(!resultFlag)
    {
        dal->restore(restorePoint);
    }

    cIT.toFront();
    while(cIT.hasNext())
    {
        cIT.next();
        CachePtr cPtr=cIT.value();
        cPtr->setChangedAsCommited();
    }

    CachePerformerMgr* pMgr=performerMgr();
    int performerID=2078;
    QString key=SBIDPerformer::createKey(performerID);
    if(pMgr->contains(key))
    {
        SBIDPerformerPtr pPtr=SBIDPerformer::retrievePerformer(performerID);
    }

    //	CWIP: remove when database is cached
    SearchItemModel* oldSim=Context::instance()->searchItemModel();
    SearchItemModel* newSim=new SearchItemModel();

    QLineEdit* lineEdit=Context::instance()->getMainWindow()->ui.searchEdit;
    QCompleter* completer=lineEdit->completer();
    completer->setModel(newSim);

    delete(oldSim); oldSim=NULL;
    Context::instance()->setSearchItemModel(newSim);

    //	CWIP: remove when database is cached
    Context::instance()->getController()->preloadAllSongs();

    return resultFlag;
}

void
CacheManager::_init()
{
    _cache[sb_cache_album]=std::make_shared<CacheAlbumMgr>(CacheAlbumMgr("a_mgr", Common::sb_type_album));
    _cache[sb_cache_album_performance]=std::make_shared<CacheAlbumPerformanceMgr>(CacheAlbumPerformanceMgr("ap_mgr",Common::sb_type_album_performance));
    _cache[sb_cache_chart]=std::make_shared<CacheChartMgr>(CacheChartMgr("c_mgr",Common::sb_type_chart));
    _cache[sb_cache_chart_performance]=std::make_shared<CacheChartPerformanceMgr>(CacheChartPerformanceMgr("cp_mgr",Common::sb_type_chart_performance));
    _cache[sb_cache_online_performance]=std::make_shared<CacheOnlinePerformanceMgr>(CacheOnlinePerformanceMgr("op_mgr",Common::sb_type_online_performance));
    _cache[sb_cache_performer]=std::make_shared<CachePerformerMgr>(CachePerformerMgr("p_mgr",Common::sb_type_performer));
    _cache[sb_cache_playlist]=std::make_shared<CachePlaylistMgr>(CachePlaylistMgr("pl_mgr",Common::sb_type_playlist));
    _cache[sb_cache_playlist_detail]=std::make_shared<CachePlaylistDetailMgr>(CachePlaylistDetailMgr("pld_mgr",Common::sb_type_playlist_detail));
    _cache[sb_cache_song_performance]=std::make_shared<CacheSongPerformanceMgr>(CacheSongPerformanceMgr("sp_mgr",Common::sb_type_song_performance));
    _cache[sb_cache_song]=std::make_shared<CacheSongMgr>(CacheSongMgr("s_mgr",Common::sb_type_song));
}
