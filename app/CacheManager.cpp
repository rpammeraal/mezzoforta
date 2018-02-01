#include <QCompleter>

#include "CacheManager.h"
#include "CacheTemplate.h"
#include "Context.h"
#include "Controller.h"
#include "MainWindow.h"
#include "SearchItemModel.h"

static const SBKey::ItemType _order [] =
{
    SBKey::ChartPerformance,
    SBKey::Chart,
    SBKey::Album,
    SBKey::AlbumPerformance,
    SBKey::OnlinePerformance,
    SBKey::Performer,
    SBKey::PlaylistDetail,
    SBKey::Playlist,
    SBKey::SongPerformance,
    SBKey::Song
};

CacheManager::CacheManager()
{
    _init();
}

void
CacheManager::clearAllCaches()
{
    QMapIterator<SBKey::ItemType,CachePtr> cIT(_cache);
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
    QMapIterator<SBKey::ItemType,CachePtr> cIT(_cache); while(cIT.hasNext())
    {
        cIT.next();
        CachePtr cPtr=cIT.value();
        cPtr->debugShowChanges();
    }
}

SBIDPtr
CacheManager::get(SBKey::ItemType itemType,int itemID)
{
    SBIDPtr ptr;
    switch(itemType)
    {
    case SBKey::Album:
        ptr=SBIDAlbum::retrieveAlbum(itemID);
        break;

    case SBKey::Performer:
        ptr=SBIDPerformer::retrievePerformer(itemID);
        break;

    case SBKey::Song:
        ptr=SBIDSong::retrieveSong(itemID);
        break;

    case SBKey::Playlist:
        ptr=SBIDPlaylist::retrievePlaylist(itemID);
        break;

    case SBKey::AlbumPerformance:
        ptr=SBIDAlbumPerformance::retrieveAlbumPerformance(itemID);
        break;

    case SBKey::OnlinePerformance:
        ptr=SBIDOnlinePerformance::retrieveOnlinePerformance(itemID);
        break;

    case SBKey::Chart:
        ptr=SBIDChart::retrieveChart(itemID);
        break;

    case SBKey::ChartPerformance:
        ptr=SBIDChartPerformance::retrieveChartPerformance(itemID);
        break;

    case SBKey::PlaylistDetail:
        ptr=SBIDPlaylistDetail::retrievePlaylistDetail(itemID);
        break;

    case SBKey::SongPerformance:
        ptr=SBIDSongPerformance::retrieveSongPerformance(itemID);
        break;

    case SBKey::Invalid:
        break;
    }
    if(!ptr)
    {
        qDebug() << SB_DEBUG_NPTR;
    }
    return ptr;
}

SBIDPtr
CacheManager::get(SBKey key)
{
    return CacheManager::get(key.itemType(),key.itemID());
}

bool
CacheManager::saveChanges(const QString& progressDialogTitle)
{
    QStringList updateSQL;
    QStringList insertSQL;
    QStringList deleteSQL;
    DataAccessLayer* dal=Context::instance()->dataAccessLayer();

    for(size_t i=0;i<SBKey::ItemTypeCount();i++)
    {
        SBKey::ItemType itemType=_order[i];
        CachePtr cPtr=_cache[itemType];

        int numUpdatesBefore=insertSQL.count()+updateSQL.count()+deleteSQL.count();

        insertSQL.append(cPtr->retrieveChanges(Common::db_insert));
        updateSQL.append(cPtr->retrieveChanges(Common::db_update));
        deleteSQL.append(cPtr->retrieveChanges(Common::db_delete));

        int numUpdatesAfter=insertSQL.count()+updateSQL.count()+deleteSQL.count();
        if(numUpdatesAfter-numUpdatesBefore>3)
        {
            _albumsUpdatedFlag=(itemType==SBKey::ItemType::Album?1:albumsUpdatedFlag());
            _performersUpdatedFlag=(itemType==SBKey::ItemType::Performer?1:performersUpdatedFlag());
            _songsUpdatedFlag=(itemType==SBKey::ItemType::Song?1:songsUpdatedFlag());
        }

        //	Not quite using the detailed data contained in Cache::_changes,_removals yet.
        if(itemType==SBKey::Album && cPtr->removals().count()>0)
        {
            setAlbumsUpdatedFlag();
        }
        if(itemType==SBKey::Performer && cPtr->removals().count()>0)
        {
            setPerformersUpdatedFlag();
        }
        if(itemType==SBKey::Song && cPtr->removals().count()>0)
        {
            setSongsUpdatedFlag();
        }
    }

    QStringList SQL=updateSQL;
    SQL.append(insertSQL);
    SQL.append(deleteSQL);

    QString restorePoint=dal->createRestorePoint();
    bool resultFlag=dal->executeBatch(SQL,progressDialogTitle);
    if(!resultFlag)
    {
        dal->restore(restorePoint);
    }

    for(size_t i=0;i<SBKey::ItemTypeCount();i++)
    {
        SBKey::ItemType itemType=_order[i];
        CachePtr cPtr=_cache[itemType];
        cPtr->setChangedAsCommited();
        cPtr->distributeReloadList();
    }

    //	CWIP: remove when database is cached
    if(albumsUpdatedFlag() || performersUpdatedFlag() || songsUpdatedFlag())
    {
        Navigator* n=Context::instance()->navigator();
        n->refreshSearchCompleter();

        //	CWIP: remove when database is cached
        Context::instance()->controller()->preloadAllSongs();
    }

    return resultFlag;
}

///	Protected methods
///	Private methods
void
CacheManager::_init()
{
    _cache[SBKey::Album]=std::make_shared<CacheAlbumMgr>(CacheAlbumMgr("a_mgr", SBKey::Album));
    _cache[SBKey::AlbumPerformance]=std::make_shared<CacheAlbumPerformanceMgr>(CacheAlbumPerformanceMgr("ap_mgr",SBKey::AlbumPerformance));
    _cache[SBKey::Chart]=std::make_shared<CacheChartMgr>(CacheChartMgr("c_mgr",SBKey::Chart));
    _cache[SBKey::ChartPerformance]=std::make_shared<CacheChartPerformanceMgr>(CacheChartPerformanceMgr("cp_mgr",SBKey::ChartPerformance));
    _cache[SBKey::OnlinePerformance]=std::make_shared<CacheOnlinePerformanceMgr>(CacheOnlinePerformanceMgr("op_mgr",SBKey::OnlinePerformance));
    _cache[SBKey::Performer]=std::make_shared<CachePerformerMgr>(CachePerformerMgr("p_mgr",SBKey::Performer));
    _cache[SBKey::Playlist]=std::make_shared<CachePlaylistMgr>(CachePlaylistMgr("pl_mgr",SBKey::Playlist));
    _cache[SBKey::PlaylistDetail]=std::make_shared<CachePlaylistDetailMgr>(CachePlaylistDetailMgr("pld_mgr",SBKey::PlaylistDetail));
    _cache[SBKey::SongPerformance]=std::make_shared<CacheSongPerformanceMgr>(CacheSongPerformanceMgr("sp_mgr",SBKey::SongPerformance));
    _cache[SBKey::Song]=std::make_shared<CacheSongMgr>(CacheSongMgr("s_mgr",SBKey::Song));

    _clearAlbumsUpdatedFlag();
    _clearPerformersUpdatedFlag();
    _clearSongsUpdatedFlag();
}
