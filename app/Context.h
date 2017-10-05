#ifndef CONTEXT_H
#define CONTEXT_H

#include <QDebug>

#include "Chooser.h"
#include "Common.h"
#include "CompleterFactory.h"
#include "DataAccessLayer.h"
#include "DBManager.h"
#include "KeyboardEventCatcher.h"
#include "Navigator.h"
#include "PlayerController.h"
#include "PlayManager.h"
#include "Properties.h"
#include "SBModelQueuedSongs.h"
#include "SBIDAlbum.h"
#include "SBIDChart.h"
#include "SBIDChartPerformance.h"
#include "SBIDManagerHelper.h"
#include "SBIDManagerTemplate.h"
#include "SBIDSongPerformance.h"
#include "SBIDAlbumPerformance.h"
#include "SBIDPerformer.h"
#include "SBIDSong.h"
#include "SearchItemModel.h"
#include "ScreenStack.h"

class BackgroundThread;
class Controller;
class DataAccessLayer;
class ExternalData;
class MainWindow;
class SBTab;
class SBTabQueuedSongs;	//	CWIP: remove

typedef SBIDManagerTemplate<SBIDAlbum,SBIDBase> SBIDAlbumMgr;
typedef SBIDManagerTemplate<SBIDChart,SBIDBase> SBIDChartMgr;
typedef SBIDManagerTemplate<SBIDChartPerformance,SBIDBase> SBIDChartPerformanceMgr;
typedef SBIDManagerTemplate<SBIDAlbumPerformance,SBIDBase> SBIDAlbumPerformanceMgr;
typedef SBIDManagerTemplate<SBIDOnlinePerformance,SBIDBase> SBIDOnlinePerformanceMgr;
typedef SBIDManagerTemplate<SBIDPlaylist,SBIDBase> SBIDPlaylistMgr;
typedef SBIDManagerTemplate<SBIDPlaylistDetail,SBIDBase> SBIDPlaylistDetailMgr;
typedef SBIDManagerTemplate<SBIDPerformer,SBIDBase> SBIDPerformerMgr;
typedef SBIDManagerTemplate<SBIDSong,SBIDBase> SBIDSongMgr;
typedef SBIDManagerTemplate<SBIDSongPerformance,SBIDBase> SBIDSongPerformanceMgr;



class Context
{
public:
    static Context* instance()
    {
        //	In future, we'll maintain all Context instances and
        //	return an integer to refer to a specific context
        static Context _instance;
        return &_instance;
    }

    inline BackgroundThread* getBackgroundThread() const { SB_DEBUG_IF_NULL(_bgt); return _bgt; }
    inline CompleterFactory* completerFactory() { return &_cf; }
    inline Chooser* getChooser() { return &_lcc; }
    inline Controller* getController() const { SB_DEBUG_IF_NULL(_c); return _c; }
    inline DataAccessLayer* getDataAccessLayer() { DataAccessLayer* dal=_dbm.dataAccessLayer();SB_DEBUG_IF_NULL(dal); return dal; }
    inline DBManager* getDBManager() { return &_dbm; }
    inline KeyboardEventCatcher* keyboardEventCatcher() { return &_kec; }
    inline MainWindow* getMainWindow() const { SB_DEBUG_IF_NULL(_mw); return _mw; }
    inline Navigator* getNavigator() { return &_nav; }
    inline PlayerController* getPlayerController() { return &_pc; }
    inline SBIDAlbumMgr* getAlbumMgr() { return &_albumMgr; }
    inline SBIDAlbumPerformanceMgr* getAlbumPerformanceMgr() { return &_albumPerformanceMgr; }
    inline SBIDChartMgr* getChartMgr() { return &_chartMgr; }
    inline SBIDChartPerformanceMgr* getChartPerformanceMgr() { return &_chartPerformanceMgr; }
    inline SBIDManagerHelper* managerHelper() { return &_mh; }
    inline SBIDOnlinePerformanceMgr* getOnlinePerformanceMgr() { return &_onlinePerformanceMgr; }
    inline SBIDPerformerMgr* getPerformerMgr() { return &_performerMgr; }
    inline SBIDPlaylistMgr* getPlaylistMgr() { return &_playlistMgr; }
    inline SBIDPlaylistDetailMgr* getPlaylistDetailMgr() { return &_playlistDetailMgr; }
    inline SearchItemModel* searchItemModel() { return _sim; }
    inline PlayManager* getPlayManager() { return &_pm; }
    inline Properties* getProperties() { return &_p; }
    inline ScreenStack* getScreenStack() { return &_st; }
    inline SBModelQueuedSongs* getSBModelQueuedSongs() { return &_mqs; }
    inline SBIDSongMgr* getSongMgr() { return &_songMgr; }
    inline SBIDSongPerformanceMgr* getSongPerformanceMgr() { return &_songPerformanceMgr; }
    inline SBTab* getTab() const { SB_DEBUG_IF_NULL(_tab); return _tab; }
    inline SBTabQueuedSongs* getTabQueuedSongs() const { SB_DEBUG_IF_NULL(_tabQS); return _tabQS; }

    void setBackgroundThread(BackgroundThread* bgt);
    void setController(Controller* c);
    void setMainWindow(MainWindow* mw);
    void setSearchItemModel(SearchItemModel* sim);
    void setTab(SBTab* tab);
    void setTabQueuedSongs(SBTabQueuedSongs* tabQS);

protected:
    friend class Controller;

    void doInit(MainWindow* mw);

private:
    //	Pointers
    BackgroundThread* _bgt;
    Controller* _c;
    MainWindow* _mw;
    SBTab* _tab;
    SBTabQueuedSongs* _tabQS;

    //	Instantiated
    CompleterFactory _cf;
    Chooser _lcc;
    DBManager _dbm;
    KeyboardEventCatcher _kec;
    Navigator _nav;
    PlayerController _pc;
    PlayManager _pm;
    Properties _p;
    SBModelQueuedSongs _mqs;
    SBIDManagerHelper _mh;
    SearchItemModel* _sim;
    ScreenStack _st;

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

    Context();
    ~Context();

    Context(Context const&);
    void operator=(Context const&);

    void _init();
};

#endif // CONTEXT_H
