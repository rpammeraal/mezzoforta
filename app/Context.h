#ifndef CONTEXT_H
#define CONTEXT_H

#include <QDebug>

#include "Common.h"

class BackgroundThread;
class Chooser;
class Controller;
class DataAccessLayer;
class ExternalData;
class MainWindow;
class Navigator;
class PlayerController;
class SBTab;
class SBTabQueuedSongs;
class ScreenStack;


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
    inline Chooser* getChooser() const { SB_DEBUG_IF_NULL(_lcc); return _lcc; }
    inline Controller* getController() const { SB_DEBUG_IF_NULL(_c); return _c; }
    inline DataAccessLayer* getDataAccessLayer() const { SB_DEBUG_IF_NULL(_dal); return _dal; }
    inline MainWindow* getMainWindow() const { SB_DEBUG_IF_NULL(_mw); return _mw; }
    inline Navigator* getNavigator() const { SB_DEBUG_IF_NULL(_ssh); return _ssh; }
    inline PlayerController* getPlayerController() const { SB_DEBUG_IF_NULL(_pc); return _pc; }
    inline ScreenStack* getScreenStack() const { SB_DEBUG_IF_NULL(_st); return _st; }
    inline SBTab* getTab() const { SB_DEBUG_IF_NULL(_tab); return _tab; }
    inline SBTabQueuedSongs* getTabQueuedSongs() const { SB_DEBUG_IF_NULL(_tabQS); return _tabQS; }

    void setBackgroundThread(BackgroundThread* bgt);
    void setController(Controller* c);
    void setDataAccessLayer(DataAccessLayer* dal);
    void setChooser(Chooser* lcc);
    void setMainWindow(MainWindow* mw);
    void setNavigator(Navigator* ssh);
    void setPlayerController(PlayerController* pc);
    void setScreenStack(ScreenStack* st);
    void setTab(SBTab* tab);
    void setTabQueuedSongs(SBTabQueuedSongs* tabQS);

private:
    BackgroundThread* _bgt;
    Controller* _c;
    DataAccessLayer* _dal;
    Chooser* _lcc;
    MainWindow* _mw;
    Navigator* _ssh;
    PlayerController* _pc;
    ScreenStack* _st;
    SBTab* _tab;
    SBTabQueuedSongs* _tabQS;

    Context();
    ~Context();

    Context(Context const&);
    void operator=(Context const&);

    void _init();
};

#endif // CONTEXT_H
