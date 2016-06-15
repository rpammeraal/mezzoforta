#include <QDebug>

#include "Context.h"

///	PUBLIC
void
Context::setBackgroundThread(BackgroundThread* bgt)
{
    _bgt=bgt;
}

void
Context::setController(Controller* c)
{
    _c=c;
}

void
Context::setDataAccessLayer(DataAccessLayer* dal)
{
    _dal=dal;
}

void
Context::setChooser(Chooser *lcc)
{
    _lcc=lcc;
}

void
Context::setMainWindow(MainWindow* mw)
{
    _mw=mw;
}

void
Context::setNavigator(Navigator* ssh)
{
    _ssh=ssh;
}

void
Context::setPlayerController(PlayerController *pc)
{
    _pc=pc;
}

void
Context::setScreenStack(ScreenStack* st)
{
    _st=st;
}

void
Context::setTab(SBTab *tab)
{
    _tab=tab;
}
void
Context::setTabQueuedSongs(SBTabQueuedSongs* tabQS)
{
    _tabQS=tabQS;
}


///	PRIVATE
Context::Context()
{
    _init();
}

Context::~Context()
{

}

void
Context::_init()
{
    _bgt=NULL;
    _c=NULL;
    _dal=NULL;
    _lcc=NULL;
    _mw=NULL;
    _pc=NULL;
    _ssh=NULL;
    _st=NULL;
    _tab=NULL;
}
