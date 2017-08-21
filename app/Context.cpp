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

//void
//Context::setChooser(Chooser *lcc)
//{
//    _lcc=lcc;
//}

void
Context::setMainWindow(MainWindow* mw)
{
    _mw=mw;
}

//void
//Context::setNavigator(Navigator* ssh)
//{
//    _ssh=ssh;
//}

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

///	PROTECTED
void
Context::doInit(MainWindow* mw)
{
    setMainWindow(mw);

    _pc.doInit();   //	no dep
    _pm.doInit();   //	dependency on PlayerController
    _lcc.doInit();  //	dependency on PlayerController
    _mqs.doInit();  //	no dep
    _st.doInit();   //	no dep
    _nav.doInit();  //	no dep
    qDebug() << SB_DEBUG_INFO;
    _p.doInit();    //	no dep
    _dbm.doInit();	//	no dep
    _kec.doInit();	//	no dep
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
    //_lcc=NULL;
    _mw=NULL;
    _tab=NULL;
}
