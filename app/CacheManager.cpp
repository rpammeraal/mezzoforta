#include <QCompleter>

#include "CacheManager.h"
#include "Context.h"
#include "Controller.h"
#include "MainWindow.h"
#include "SearchItemModel.h"

CacheManager::CacheManager()
{
    _init();
}

void
CacheManager::clearAllCaches()
{
    this->albumMgr()->clear();
    this->albumPerformanceMgr()->clear();
    this->chartMgr()->clear();
    this->chartPerformanceMgr()->clear();
    this->onlinePerformanceMgr()->clear();
    this->performerMgr()->clear();
    this->playlistMgr()->clear();
    this->playlistDetailMgr()->clear();
    this->songMgr()->clear();
    this->songPerformanceMgr()->clear();
}

bool
CacheManager::commitAllCaches()
{
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    bool successFlag=1;

    //	CWIP: transactions

    //	Apply inserts.
    if(successFlag) { successFlag=this->songMgr()->commitAll(dal,Common::db_insert); }
    if(successFlag) { successFlag=this->performerMgr()->commitAll(dal,Common::db_insert); }
    if(successFlag) { successFlag=this->songPerformanceMgr()->commitAll(dal,Common::db_insert); }
    if(successFlag) { successFlag=this->albumMgr()->commitAll(dal,Common::db_insert); }
    if(successFlag) { successFlag=this->albumPerformanceMgr()->commitAll(dal,Common::db_insert); }
    if(successFlag) { successFlag=this->playlistMgr()->commitAll(dal,Common::db_insert); }
    if(successFlag) { successFlag=this->playlistDetailMgr()->commitAll(dal,Common::db_insert); }
    if(successFlag) { successFlag=this->onlinePerformanceMgr()->commitAll(dal,Common::db_insert); }
    //c->getChartMgr()->commitAll(dal,Common::db_insert);
    //c->getChartPerformanceMgr()->commitAll(dal,Common::db_insert);

    //	Apply updates.
    if(successFlag) { successFlag=this->onlinePerformanceMgr()->commitAll(dal,Common::db_update); }
    if(successFlag) { successFlag=this->playlistMgr()->commitAll(dal,Common::db_update); }
    if(successFlag) { successFlag=this->playlistDetailMgr()->commitAll(dal,Common::db_update); }
    if(successFlag) { successFlag=this->albumPerformanceMgr()->commitAll(dal,Common::db_update); }
    if(successFlag) { successFlag=this->albumMgr()->commitAll(dal,Common::db_update); }
    //c->getChartMgr()->commitAll(dal,Common::db_update);
    //c->getChartPerformanceMgr()->commitAll(dal,Common::db_update);
    if(successFlag) { successFlag=this->songPerformanceMgr()->commitAll(dal,Common::db_update); }
    if(successFlag) { successFlag=this->performerMgr()->commitAll(dal,Common::db_update); }
    if(successFlag) { successFlag=this->songMgr()->commitAll(dal,Common::db_update); }

    //	Apply deletes.
    if(successFlag) { successFlag=this->onlinePerformanceMgr()->commitAll(dal,Common::db_delete); }
    if(successFlag) { successFlag=this->playlistMgr()->commitAll(dal,Common::db_delete); }
    if(successFlag) { successFlag=this->playlistDetailMgr()->commitAll(dal,Common::db_delete); }
    if(successFlag) { successFlag=this->albumPerformanceMgr()->commitAll(dal,Common::db_delete); }
    if(successFlag) { successFlag=this->albumMgr()->commitAll(dal,Common::db_delete); }
    //c->getChartMgr()->commitAll(dal,Common::db_delete);
    //c->getChartPerformanceMgr()->commitAll(dal,Common::db_delete);
    if(successFlag) { successFlag=this->songPerformanceMgr()->commitAll(dal,Common::db_delete); }
    if(successFlag) { successFlag=this->performerMgr()->commitAll(dal,Common::db_delete); }
    if(successFlag) { successFlag=this->songMgr()->commitAll(dal,Common::db_delete); }

    //	CWIP: remove when database is cached
    SearchItemModel* oldSim=Context::instance()->searchItemModel();
    SearchItemModel* newSim=new SearchItemModel();

    QLineEdit* lineEdit=Context::instance()->getMainWindow()->ui.searchEdit;
    QCompleter* completer=lineEdit->completer();
    completer->setModel(newSim);

    delete(oldSim); oldSim=NULL;
    Context::instance()->setSearchItemModel(newSim);

    //	CWIP: remove when database is cached
    Context::instance()->getController()->preloadAllSongs();

    //	Once we get all mgr's to use the same transaction, this could be a meaningful value.
    return successFlag;
}


void
CacheManager::_init()
{
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
