#ifndef CACHEMANAGER_H
#define CACHEMANAGER_H

#include "SBIDManagerTemplate.h"
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


typedef SBIDManagerTemplate<SBIDAlbum,SBIDBase> SBIDAlbumMgr;
typedef SBIDManagerTemplate<SBIDAlbumPerformance,SBIDBase> SBIDAlbumPerformanceMgr;
typedef SBIDManagerTemplate<SBIDChart,SBIDBase> SBIDChartMgr;
typedef SBIDManagerTemplate<SBIDChartPerformance,SBIDBase> SBIDChartPerformanceMgr;
typedef SBIDManagerTemplate<SBIDOnlinePerformance,SBIDBase> SBIDOnlinePerformanceMgr;
typedef SBIDManagerTemplate<SBIDPlaylist,SBIDBase> SBIDPlaylistMgr;
typedef SBIDManagerTemplate<SBIDPlaylistDetail,SBIDBase> SBIDPlaylistDetailMgr;
typedef SBIDManagerTemplate<SBIDPerformer,SBIDBase> SBIDPerformerMgr;
typedef SBIDManagerTemplate<SBIDSong,SBIDBase> SBIDSongMgr;
typedef SBIDManagerTemplate<SBIDSongPerformance,SBIDBase> SBIDSongPerformanceMgr;


class CacheManager
{
public:
    CacheManager();
    void clearAllCaches();
    bool commitAllCaches();

    inline SBIDAlbumMgr* albumMgr() { return &_albumMgr; }
    inline SBIDAlbumPerformanceMgr* albumPerformanceMgr() { return &_albumPerformanceMgr; }
    inline SBIDChartMgr* chartMgr() { return &_chartMgr; }
    inline SBIDChartPerformanceMgr* chartPerformanceMgr() { return &_chartPerformanceMgr; }
    inline SBIDManagerHelper* managerHelper() { return &_mh; }
    inline SBIDOnlinePerformanceMgr* onlinePerformanceMgr() { return &_onlinePerformanceMgr; }
    inline SBIDPerformerMgr* performerMgr() { return &_performerMgr; }
    inline SBIDPlaylistMgr* playlistMgr() { return &_playlistMgr; }
    inline SBIDPlaylistDetailMgr* playlistDetailMgr() { return &_playlistDetailMgr; }
    inline SBIDSongMgr* songMgr() { return &_songMgr; }
    inline SBIDSongPerformanceMgr* songPerformanceMgr() { return &_songPerformanceMgr; }

private:
    SBIDAlbumMgr _albumMgr;
    SBIDAlbumPerformanceMgr _albumPerformanceMgr;
    SBIDChartMgr _chartMgr;
    SBIDChartPerformanceMgr _chartPerformanceMgr;
    SBIDOnlinePerformanceMgr _onlinePerformanceMgr;
    SBIDPerformerMgr _performerMgr;
    SBIDPlaylistMgr _playlistMgr;
    SBIDPlaylistDetailMgr _playlistDetailMgr;
    SBIDSongMgr _songMgr;
    SBIDSongPerformanceMgr _songPerformanceMgr;

    SBIDManagerHelper _mh;

    void _init();
};

#endif // CACHEMANAGER_H
