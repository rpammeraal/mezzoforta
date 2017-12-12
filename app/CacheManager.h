#ifndef CACHEMANAGER_H
#define CACHEMANAGER_H

#include "CacheTemplate.h"
#include "SBIDAlbum.h"
#include "SBIDAlbumPerformance.h"
#include "SBIDChart.h"
#include "SBIDChartPerformance.h"
#include "SBIDOnlinePerformance.h"
#include "SBIDPlaylist.h"
#include "SBIDPlaylistDetail.h"
#include "SBIDPerformer.h"
#include "SBIDSong.h"
#include "SBIDSongPerformance.h"


typedef CacheTemplate<SBIDAlbum,SBIDBase> CacheAlbumMgr;
typedef CacheTemplate<SBIDAlbumPerformance,SBIDBase> CacheAlbumPerformanceMgr;
typedef CacheTemplate<SBIDChart,SBIDBase> CacheChartMgr;
typedef CacheTemplate<SBIDChartPerformance,SBIDBase> CacheChartPerformanceMgr;
typedef CacheTemplate<SBIDOnlinePerformance,SBIDBase> CacheOnlinePerformanceMgr;
typedef CacheTemplate<SBIDPlaylist,SBIDBase> CachePlaylistMgr;
typedef CacheTemplate<SBIDPlaylistDetail,SBIDBase> CachePlaylistDetailMgr;
typedef CacheTemplate<SBIDPerformer,SBIDBase> CachePerformerMgr;
typedef CacheTemplate<SBIDSong,SBIDBase> CacheSongMgr;
typedef CacheTemplate<SBIDSongPerformance,SBIDBase> CacheSongPerformanceMgr;


class CacheManager
{
public:
    enum sb_cache_type
    {
        sb_cache_none=0,
        sb_cache_album,
        sb_cache_album_performance,
        sb_cache_chart,
        sb_cache_chart_performance,
        sb_cache_online_performance,
        sb_cache_performer,
        sb_cache_playlist,
        sb_cache_playlist_detail,
        sb_cache_song,
        sb_cache_song_performer
    };

    CacheManager();
    virtual ~CacheManager() { }

    void clearAllCaches();
    bool saveChanges();

    inline CacheManagerHelper* managerHelper() { return &_mh; }

    inline CacheAlbumMgr* albumMgr() const { return dynamic_cast<CacheAlbumMgr *>(_cache[sb_cache_album].get()); }
    inline CacheAlbumPerformanceMgr* albumPerformanceMgr() const { return dynamic_cast<CacheAlbumPerformanceMgr *>(_cache[sb_cache_album_performance].get()); }
    inline CacheChartMgr* chartMgr() const { return dynamic_cast<CacheChartMgr *>(_cache[sb_cache_chart].get()); }
    inline CacheChartPerformanceMgr* chartPerformanceMgr() const { return dynamic_cast<CacheChartPerformanceMgr *>(_cache[sb_cache_chart_performance].get()); }
    inline CacheOnlinePerformanceMgr* onlinePerformanceMgr() const { return dynamic_cast<CacheOnlinePerformanceMgr *>(_cache[sb_cache_online_performance].get()); }
    inline CachePerformerMgr* performerMgr() const { return dynamic_cast<CachePerformerMgr *>(_cache[sb_cache_performer].get()); }
    inline CachePlaylistMgr* playlistMgr() const { return dynamic_cast<CachePlaylistMgr *>(_cache[sb_cache_playlist].get()); }
    inline CachePlaylistDetailMgr* playlistDetailMgr() const { return dynamic_cast<CachePlaylistDetailMgr *>(_cache[sb_cache_playlist_detail].get()); }
    inline CacheSongMgr* songMgr() const { return dynamic_cast<CacheSongMgr *>(_cache[sb_cache_song].get()); }
    inline CacheSongPerformanceMgr* songPerformanceMgr() const { return dynamic_cast<CacheSongPerformanceMgr *>(_cache[sb_cache_song_performer].get()); }

private:
    QMap<sb_cache_type,CachePtr> _cache;
    CacheManagerHelper _mh;

    void _init();
};

#endif // CACHEMANAGER_H
