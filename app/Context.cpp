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
Context::setSearchItemModel(SearchItemModel *sim)
{
    _sim=sim;
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

    _albumMgr.setName("mgr_a");
    _albumPerformanceMgr.setName("mgr_ap");
    _chartMgr.setName("mgr_c");
    _chartPerformanceMgr.setName("mgr_cm");
    _onlinePerformanceMgr.setName("mgr_op");
    _performerMgr.setName("mgr_p");
    _playlistMgr.setName("mgr_pl");
    _playlistDetailMgr.setName("mgr_pld");
    _songMgr.setName("mgr_s");
    _songPerformanceMgr.setName("mgr_sp");
}
