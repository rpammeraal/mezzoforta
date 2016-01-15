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

    inline BackgroundThread* getBackgroundThread() const { return bgt; }
    inline Chooser* getChooser() const { return lcc; }
    inline Controller* getController() const { return c; }
    inline DataAccessLayer* getDataAccessLayer() const { return dal; }
    inline MainWindow* getMainWindow() const { return mw; }
    inline Navigator* getNavigator() const { if(!ssh) { qDebug() << SB_DEBUG_NPTR; } return ssh; }
    inline PlayerController* getPlayerController() const { if(!pc) { qDebug() << SB_DEBUG_NPTR; } return pc; }
    inline SBTab* getTab() const { if(!tab) { qDebug() << SB_DEBUG_NPTR; } return tab; }
    inline ScreenStack* getScreenStack() const { if(!st) { qDebug() << SB_DEBUG_NPTR; } return st; }

    void setBackgroundThread(BackgroundThread* nbgt);
    void setController(Controller* nc);
    void setDataAccessLayer(DataAccessLayer* ndal);
    void setChooser(Chooser* nlcc);
    void setMainWindow(MainWindow* nmw);
    void setNavigator(Navigator* nssh);
    void setPlayerController(PlayerController* npc);
    void setScreenStack(ScreenStack* st);
    void setTab(SBTab* tab);

private:
    BackgroundThread* bgt;
    Controller* c;
    DataAccessLayer* dal;
    Chooser* lcc;
    MainWindow* mw;
    Navigator* ssh;
    PlayerController* pc;
    ScreenStack* st;
    SBTab* tab;

    Context();
    ~Context();

    Context(Context const&);
    void operator=(Context const&);

    void init();
};

#endif // CONTEXT_H
