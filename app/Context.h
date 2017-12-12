#ifndef CONTEXT_H
#define CONTEXT_H

#include <QDebug>

#include "Chooser.h"
#include "CompleterFactory.h"
#include "DBManager.h"
#include "KeyboardEventCatcher.h"
#include "Navigator.h"
#include "PlayerController.h"
#include "PlayManager.h"
#include "Properties.h"
#include "CacheManagerHelper.h"
#include "SBModelQueuedSongs.h"

class BackgroundThread;
class CacheManager;
class Controller;
class DataAccessLayer;
class ExternalData;
class MainWindow;
class SBTab;
class SBTabQueuedSongs;	//	CWIP: remove
class ScreenStack;
class SearchItemModel;

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
    inline CacheManager* cacheManager() { return _cm; }
    inline CompleterFactory* completerFactory() { return &_cf; }
    inline Chooser* getChooser() { return &_lcc; }
    inline Controller* getController() const { SB_DEBUG_IF_NULL(_c); return _c; }
    inline DataAccessLayer* getDataAccessLayer() { DataAccessLayer* dal=_dbm.dataAccessLayer();SB_DEBUG_IF_NULL(dal); return dal; }
    inline DBManager* getDBManager() { return &_dbm; }
    inline KeyboardEventCatcher* keyboardEventCatcher() { return &_kec; }
    inline MainWindow* getMainWindow() const { SB_DEBUG_IF_NULL(_mw); return _mw; }
    inline Navigator* getNavigator() { return &_nav; }
    inline PlayerController* getPlayerController() { return &_pc; }
    inline SearchItemModel* searchItemModel() { return _sim; }
    inline PlayManager* getPlayManager() { return &_pm; }
    inline Properties* getProperties() { return &_p; }
    inline ScreenStack* getScreenStack() { return &_st; }
    inline SBModelQueuedSongs* getSBModelQueuedSongs() { return &_mqs; }
    inline SBTab* getTab() const { SB_DEBUG_IF_NULL(_tab); return _tab; }
    inline SBTabQueuedSongs* getTabQueuedSongs() const { SB_DEBUG_IF_NULL(_tabQS); return _tabQS; }

    void setBackgroundThread(BackgroundThread* bgt);
    void setCacheManager(CacheManager* cm);
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
    CacheManager* _cm;
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
    CacheManagerHelper _mh;
    SearchItemModel* _sim;
    ScreenStack _st;

    Context();
    ~Context();

    Context(Context const&);
    void operator=(Context const&);

    void _init();
};

#endif // CONTEXT_H
