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
    CacheManager();
    virtual ~CacheManager() { }

    inline bool albumsUpdatedFlag() const { return _albumsUpdatedFlag; }
    void clearAllCaches();
    void debugShowChanges(const QString& title);
    static SBIDPtr get(SBKey::ItemType itemType,int ID);
    static SBIDPtr get(SBKey key);
    inline bool performersUpdatedFlag() const { return _performersUpdatedFlag; }
    bool saveChanges(const QString& progressDialogTitle=QString(),bool doNotUpdateCompletersFlag=0);
    inline bool songsUpdatedFlag() const { return _songsUpdatedFlag; }
    inline void setAlbumsUpdatedFlag() { _albumsUpdatedFlag=1; }
    inline void setPerformersUpdatedFlag() { _performersUpdatedFlag=1; }
    inline void setSongsUpdatedFlag() { _songsUpdatedFlag=1; }

    inline CacheAlbumMgr* albumMgr() const { return dynamic_cast<CacheAlbumMgr *>(_cache[SBKey::Album].get()); }
    inline CacheAlbumPerformanceMgr* albumPerformanceMgr() const { return dynamic_cast<CacheAlbumPerformanceMgr *>(_cache[SBKey::AlbumPerformance].get()); }
    inline CacheChartMgr* chartMgr() const { return dynamic_cast<CacheChartMgr *>(_cache[SBKey::Chart].get()); }
    inline CacheChartPerformanceMgr* chartPerformanceMgr() const { return dynamic_cast<CacheChartPerformanceMgr *>(_cache[SBKey::ChartPerformance].get()); }
    inline CacheOnlinePerformanceMgr* onlinePerformanceMgr() const { return dynamic_cast<CacheOnlinePerformanceMgr *>(_cache[SBKey::OnlinePerformance].get()); }
    inline CachePerformerMgr* performerMgr() const { return dynamic_cast<CachePerformerMgr *>(_cache[SBKey::Performer].get()); }
    inline CachePlaylistMgr* playlistMgr() const { return dynamic_cast<CachePlaylistMgr *>(_cache[SBKey::Playlist].get()); }
    inline CachePlaylistDetailMgr* playlistDetailMgr() const { return dynamic_cast<CachePlaylistDetailMgr *>(_cache[SBKey::PlaylistDetail].get()); }
    inline CacheSongMgr* songMgr() const { return dynamic_cast<CacheSongMgr *>(_cache[SBKey::Song].get()); }
    inline CacheSongPerformanceMgr* songPerformanceMgr() const { return dynamic_cast<CacheSongPerformanceMgr *>(_cache[SBKey::SongPerformance].get()); }

protected:
    friend class SBIDBase;

private:
    QMap<SBKey::ItemType,CachePtr> _cache;

    bool _albumsUpdatedFlag;
    bool _performersUpdatedFlag;
    bool _songsUpdatedFlag;

    void _clearAlbumsUpdatedFlag() { _albumsUpdatedFlag=0; }
    void _clearPerformersUpdatedFlag() { _performersUpdatedFlag=0; }
    void _clearSongsUpdatedFlag() { _songsUpdatedFlag=0; }
    void _init();
};

#endif // CACHEMANAGER_H
