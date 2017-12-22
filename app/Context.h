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

    inline BackgroundThread* backgroundThread() const { SB_DEBUG_IF_NULL(_bgt); return _bgt; }
    inline CacheManager* cacheManager() { return _cm; }
    inline CompleterFactory* completerFactory() { return &_cf; }
    inline Chooser* chooser() { return &_lcc; }
    inline Controller* controller() const { SB_DEBUG_IF_NULL(_c); return _c; }
    inline DataAccessLayer* dataAccessLayer() { DataAccessLayer* dal=_dbm.dataAccessLayer();SB_DEBUG_IF_NULL(dal); return dal; }
    inline DBManager* dbManager() { return &_dbm; }
    inline KeyboardEventCatcher* keyboardEventCatcher() { return &_kec; }
    inline MainWindow* mainWindow() const { SB_DEBUG_IF_NULL(_mw); return _mw; }
    inline Navigator* navigator() { return &_nav; }
    inline PlayerController* playerController() { return &_pc; }
    inline SearchItemModel* searchItemModel() { return _sim; }
    inline PlayManager* playManager() { return &_pm; }
    inline Properties* properties() { return &_p; }
    inline ScreenStack* screenStack() { return &_st; }
    inline SBModelQueuedSongs* sbModelQueuedSongs() { return &_mqs; }
    inline SBTab* tab() const { SB_DEBUG_IF_NULL(_tab); return _tab; }
    inline SBTabQueuedSongs* tabQueuedSongs() const { SB_DEBUG_IF_NULL(_tabQS); return _tabQS; }

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
