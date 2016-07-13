#ifndef CONTEXT_H
#define CONTEXT_H

#include <QDebug>

#include "Chooser.h"
#include "Common.h"
#include "DataAccessLayer.h"
#include "Navigator.h"
#include "PlayerController.h"
#include "PlayManager.h"
#include "Properties.h"
#include "SBModelQueuedSongs.h"
#include "ScreenStack.h"

class BackgroundThread;
class Controller;
class DataAccessLayer;
class ExternalData;
class MainWindow;
class SBTab;
class SBTabQueuedSongs;	//	CWIP: remove


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
    //inline Chooser* getChooser() const { SB_DEBUG_IF_NULL(_lcc); return _lcc; }
    inline Chooser* getChooser() { return &_lcc; }
    inline Controller* getController() const { SB_DEBUG_IF_NULL(_c); return _c; }
    inline DataAccessLayer* getDataAccessLayer() const { SB_DEBUG_IF_NULL(_dal); return _dal; }
    inline MainWindow* getMainWindow() const { SB_DEBUG_IF_NULL(_mw); return _mw; }
    inline Navigator* getNavigator() { return &_nav; }
    inline PlayerController* getPlayerController() { return &_pc; }
    inline PlayManager* getPlayManager() { return &_pm; }
    inline Properties* getProperties() { return &_p; }
    inline ScreenStack* getScreenStack() { return &_st; }
    inline SBModelQueuedSongs* getSBModelQueuedSongs() { return &_mqs; }
    inline SBTab* getTab() const { SB_DEBUG_IF_NULL(_tab); return _tab; }
    inline SBTabQueuedSongs* getTabQueuedSongs() const { SB_DEBUG_IF_NULL(_tabQS); return _tabQS; }

    void setBackgroundThread(BackgroundThread* bgt);
    void setController(Controller* c);
    //void setChooser(Chooser* lcc);
    void setDataAccessLayer(DataAccessLayer* dal);
    void setMainWindow(MainWindow* mw);
    void setTab(SBTab* tab);
    void setTabQueuedSongs(SBTabQueuedSongs* tabQS);

protected:
    friend class Controller;

    void doInit(MainWindow* mw,DataAccessLayer* dal);

private:
    //	Pointers
    BackgroundThread* _bgt;
    Controller* _c;
    DataAccessLayer* _dal;
    MainWindow* _mw;
    SBTab* _tab;
    SBTabQueuedSongs* _tabQS;

    //	Instantiated
    Chooser _lcc;
    Navigator _nav;
    PlayerController _pc;
    PlayManager _pm;
    Properties _p;
    SBModelQueuedSongs _mqs;
    ScreenStack _st;

    Context();
    ~Context();

    Context(Context const&);
    void operator=(Context const&);

    void _init();
};

#endif // CONTEXT_H
