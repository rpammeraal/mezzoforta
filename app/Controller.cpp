#include <QDebug>
#include <QScrollBar>
#include <QCompleter>
#include <QFileDialog>
#include <QMessageBox>
#include <QSplashScreen>
#include <QSqlDatabase>
#include <QStyleFactory>
#include <QTimer>

#include "BackgroundThread.h"
#include "Chooser.h"
#include "Common.h"
#include "CompleterFactory.h"
#include "Context.h"
#include "Controller.h"
#include "DataAccessLayer.h"
#include "DBManager.h"
#include "ExternalData.h"
#include "MainWindow.h"
#include "MusicLibrary.h"
#include "Navigator.h"
#include "Network.h"
#include "PlayerController.h"
#include "Properties.h"
#include "SBIDBase.h"
#include "SBIDOnlinePerformance.h"
#include "SBDialogSelectItem.h"
#include "SBMessageBox.h"
#include "SBSqlQueryModel.h"
#include "SBStandardItemModel.h"
#include "SBTabSongsAll.h"
#include "ScreenStack.h"
#include "SetupWizard.h"

Controller::Controller(int argc, char *argv[], QApplication* app) : _app(app)
{
    Q_UNUSED(argc);
    Q_UNUSED(argv);
    Context::instance()->setController(this);

    _initSuccessFull=openMainWindow(1);
}

Controller::~Controller()
{
    backgroundThread.quit();
    backgroundThread.wait();
}

void
Controller::clearAllCaches() const
{
    Context* c=Context::instance();
    c->getAlbumMgr()->clear();
    c->getAlbumPerformanceMgr()->clear();
    c->getChartMgr()->clear();
    c->getChartPerformanceMgr()->clear();
    c->getOnlinePerformanceMgr()->clear();
    c->getPerformerMgr()->clear();
    c->getPlaylistMgr()->clear();
    c->getPlaylistDetailMgr()->clear();
    c->getSongMgr()->clear();
    c->getSongPerformanceMgr()->clear();
}

void
Controller::commitAllCaches(DataAccessLayer* dal) const
{
    Context* c=Context::instance();
    c->getOnlinePerformanceMgr()->commitAll(dal);
    c->getPlaylistMgr()->commitAll(dal);
    c->getPlaylistDetailMgr()->commitAll(dal);
    c->getAlbumPerformanceMgr()->commitAll(dal);
    c->getAlbumMgr()->commitAll(dal);
    //c->getChartMgr()->commitAll(dal);
    //c->getChartPerformanceMgr()->commitAll(dal);
    c->getPerformerMgr()->commitAll(dal);
    c->getSongPerformanceMgr()->commitAll(dal);
    c->getSongMgr()->commitAll(dal);
}


bool
Controller::initSuccessFull() const
{
    return _initSuccessFull;
}

void
Controller::preloadAllSongs() const
{
    MainWindow* mw=Context::instance()->getMainWindow();
    SBTabSongsAll* tsa=mw->ui.tabAllSongs;
    qDebug() << SB_DEBUG_INFO;
    tsa->preload();
}

void
Controller::refreshModels()
{
    //	Allows some data models to be refreshed
    MainWindow* mw=Context::instance()->getMainWindow();

    //	Now that we have a database connection, create a searchItem model.
    SearchItemModel* sim=new SearchItemModel();
    Context::instance()->setSearchItemModel(sim);
    connect(Context::instance()->managerHelper(), SIGNAL(removedSBIDPtr(SBIDPtr)),
            sim, SLOT(remove(SBIDPtr)));
    connect(Context::instance()->managerHelper(), SIGNAL(updatedSBIDPtr(SBIDPtr)),
            sim, SLOT(update(SBIDPtr)));

    Navigator* n=Context::instance()->getNavigator();
    n->resetAllFiltersAndSelections();

    //	Completers
    CompleterFactory* cf=Context::instance()->completerFactory();

    mw->ui.songEditTitle->setCompleter(cf->getCompleterSong());
    mw->ui.songEditPerformerName->setCompleter(cf->getCompleterPerformer());
    mw->ui.performerEditName->setCompleter(cf->getCompleterPerformer());

    //	SEARCH
    QCompleter* c=mw->ui.searchEdit->completer();
    connect(
        c, SIGNAL(activated(const QModelIndex&)),
        Context::instance()->getNavigator(), SLOT(openItemFromCompleter(const QModelIndex&)));
    connect(
        mw->ui.searchEdit,SIGNAL(returnPressed()),
        Context::instance()->getNavigator(),SLOT(applySonglistFilter()));
    connect(
        c, SIGNAL(activated(QString)),
        mw->ui.searchEdit, SLOT(clear()),
        Qt::QueuedConnection);	//	this will clear the search box

    //	Clear caches
    this->clearAllCaches();
    qDebug() << SB_DEBUG_INFO;
}

///	Public slots:
void
Controller::newDatabase()
{
    SetupWizard sw;
    sw.start(0);
}

void
Controller::openDatabase()
{
    //	If song is playing, stop player and proceed.
    PlayerController* pc=Context::instance()->getPlayerController();
    PlayerController::sb_player_state state=pc->playState();

    if(state==PlayerController::sb_player_state_play || state==PlayerController::sb_player_state_pause)
    {
        int action=SBMessageBox::createSBMessageBox("Song is still playing",
                                         "Music will be stopped before selecting another database. Click OK to proceed.",
                                         QMessageBox::Warning,
                                         QMessageBox::Ok | QMessageBox::Cancel,
                                         QMessageBox::Cancel,
                                         QMessageBox::Cancel,
                                         1);

        if(action==QMessageBox::Cancel)
        {
            return;
        }
        else
        {
            PlayManager* pm=Context::instance()->getPlayManager();
            pm->playerStop();
            pm->clearPlaylist();
        }
    }
    openMainWindow(0);
}

void
Controller::setMusicLibraryDirectory()
{
    Context::instance()->getProperties()->userSetMusicLibraryDirectory();
}

void
Controller::rescanMusicLibrary()
{
    MusicLibrary ml;
    ml.rescanMusicLibrary();
}

void
Controller::changeSchema(const QString& newSchema)
{
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();

    if(dal->schema()!=newSchema)
    {
        if(dal->setSchema(newSchema))
        {
            //	refresh all views
            refreshModels();
            emit schemaChanged();
        }
    }
    this->_disableScreenNavigationButtons();
}

void
Controller::updateStatusBarText(const QString &s)
{
    Context::instance()->getMainWindow()->ui.statusBar->setText(s);
    statusBarResetTimer.start(10000);
    statusBarResetTimer.setSingleShot(1);
}

//PROTECTED:

//PRIVATE:

///
/// \brief Controller::openMainWindow
/// \param appStartUpFlag
/// \return
///
/// openMainWindow takes care of:
/// -	making sure a database is selected and running
/// 	-	DatabaseSelector opens a previously used database or asks for one (if startup==1),
/// 		or asks for one (if startup==0).
/// -	sets up the MainWindow
/// -	sets up Data Access
/// -	sets up all models
bool
Controller::openMainWindow(bool appStartUpFlag)
{
    //	Instantiate DatabaseSelector, check if database could be opened.
    DBManager* dbm=Context::instance()->getDBManager();
    if(appStartUpFlag)
    {
        bool openedFlag=dbm->openDefaultDatabase();
        if(!openedFlag)
        {
            //	Start wizard
            qDebug() << SB_DEBUG_INFO << "start wizard";

            //	1.	Open 'Hi!' dialog
            SetupWizard sw;
            sw.start();
        }
    }
    else
    {
        dbm->userOpenDatabase();
    }

    if(appStartUpFlag==0 && dbm->databaseChanged()==0)
    {
        //	no database opened upon appStartUpFlag.
        return 0;
    }
    if(dbm->databaseOpened()==0)
    {
        return 0;
    }

    QPixmap pm(":/images/moose7.2.png");
    QSplashScreen splash(pm);

    if(appStartUpFlag)
    {
        splash.show();
        _app->processEvents();
    }

    MainWindow* oldMW=Context::instance()->getMainWindow();
    if(oldMW)
    {
        Context::instance()->setMainWindow(NULL);
        oldMW->close();
        oldMW=NULL;
    }

    MainWindow* mw=new MainWindow();
    SB_DEBUG_IF_NULL(mw);
    mw->hide();

    //	Install event handler
    _app->installNativeEventFilter(Context::instance()->keyboardEventCatcher());

    Context::instance()->doInit(mw);	//	This has to be done as soon as we have mw

    init();

    refreshModels();

    setupUI();

    configureMenus();

    mw->setWindowTitle(mw->windowTitle() + " - " + dbm->databaseName() + " ("+Context::instance()->getDataAccessLayer()->getDriverName()+")");

    SBIDSong::updateSoundexFields();
    SBIDPerformer::updateSoundexFields();

    _resetStatusBar();

    if(appStartUpFlag)
    {
        splash.finish(mw);
    }

    mw->show();
    Context::instance()->getNavigator()->openOpener();

    //	Kick off import
    Properties* properties=Context::instance()->getProperties();
    if(properties->configValue(Properties::sb_run_import_on_startup_flag)=="1")
    {
        MusicLibrary ml;
        ml.rescanMusicLibrary();
    }
    properties->setConfigValue(Properties::sb_run_import_on_startup_flag,"0");

    return 1;
}

void
Controller::setupUI()
{
    //	Frequently used pointers
    MainWindow* mw=Context::instance()->getMainWindow();

    qDebug() << SB_DEBUG_INFO << "Controller:setupUI:start";

    ///	Statusbar
    mw->ui.statusBar->setReadOnly(true);

    ///	BUTTONS
    this->_disableScreenNavigationButtons();

    Navigator* ssh=Context::instance()->getNavigator();
    connect(mw->ui.buttonBackward, SIGNAL(clicked()),
            ssh, SLOT(tabBackward()));
    connect(mw->ui.buttonForward, SIGNAL(clicked()),
            ssh, SLOT(tabForward()));


    QTreeView* tv=mw->ui.leftColumnChooser;
    tv->setColumnHidden(1,1);
    tv->setColumnHidden(2,1);
    tv->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tv->expandAll();
    connect(tv, SIGNAL(clicked(QModelIndex)),
            Context::instance()->getNavigator(), SLOT(openChooserItem(QModelIndex)));
    tv->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(tv, SIGNAL(customContextMenuRequested(const QPoint&)),
            Context::instance()->getChooser(), SLOT(showContextMenu(QPoint)));


    ///	MISC

    mw->ui.mainTab->tabBar()->hide();
    this->setFontSizes();
    mw->ui.searchEdit->setFocus();

    //	Have SBTabSongsAll start populating
    this->preloadAllSongs();

    //	Set up schema dropdown box
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();

    if(dal->supportSchemas())
    {
        mw->ui.cbSchema->clear();
        mw->ui.cbSchema->insertItems(0,dal->availableSchemas());
        mw->ui.cbSchema->setVisible(1);
        mw->ui.labelSchema->setVisible(1);
        mw->ui.frSchema->setVisible(1);

        connect(mw->ui.cbSchema,SIGNAL(currentIndexChanged(QString)),
                this,SLOT(changeSchema(QString)));

        //	Set current schema properly in schema drop down box
        QString currentSchema=dal->schema();
        for(int i=0;i<mw->ui.cbSchema->count();i++)
        {
            if(mw->ui.cbSchema->itemText(i)==currentSchema)
            {
                mw->ui.cbSchema->setCurrentIndex(i);
            }
        }
    }
    else
    {
        mw->ui.cbSchema->setVisible(0);
        mw->ui.labelSchema->setVisible(0);
        mw->ui.frSchema->setVisible(0);
    }

    qDebug() << SB_DEBUG_INFO << "playground";

//    SBIDSongMgr* smgr=Context::instance()->getSongMgr();

//    QMap<int,QList<SBIDSongPtr>> matches;
//    QList<SBIDSongPtr> l;

//    matches[0]=QList<SBIDSongPtr>();

//    l.clear();
//    l.append(SBIDSong::retrieveSong(31404));
//    l.append(SBIDSong::retrieveSong(31396));
//    l.append(SBIDSong::retrieveSong(31385));
//    matches[1]=l;

//    l.clear();
//    l.append(SBIDSong::retrieveSong(31404));
//    l.append(SBIDSong::retrieveSong(31396));
//    l.append(SBIDSong::retrieveSong(31385));
//    matches[2]=l;

//    QMapIterator<int,QList<SBIDSongPtr>> mIT(matches);
//    while(mIT.hasNext())
//    {
//        mIT.next();
//        QList<SBIDSongPtr> a=mIT.value();
//        QListIterator<SBIDSongPtr> aIT(a);
//        while(aIT.hasNext())
//        {
//            qDebug() << SB_DEBUG_INFO << mIT.key() << *(aIT.next());
//        }
//    }
//    qDebug() << SB_DEBUG_INFO << matches[1].count();
//    qDebug() << SB_DEBUG_INFO << matches[2].count();

//    Common::sb_parameters parameters;
//    parameters.songTitle="I suck";
//    parameters.performerName="The Suckers";

//    SBDialogSelectItem* i=SBDialogSelectItem::selectSong(parameters,SBIDSongPtr(),matches);
//    i->exec();

//    if(i->hasSelectedItem())
//    {
//        SBIDPtr ptr=i->getSelected();
//        if(ptr)
//        {
//            qDebug() << SB_DEBUG_INFO << "SELECTED=" << *ptr;
//        }
//        else
//        {
//            qDebug() << SB_DEBUG_INFO << "SELECTED NEW";
//        }

//    }
//    else
//    {
//        qDebug() << SB_DEBUG_INFO << "NONE SELECTED";
//    }

//    SBIDPerformerPtr u2ptr1=SBIDPerformer::retrievePerformer(2078,1);
//    qDebug() << SB_DEBUG_INFO << u2ptr1->genericDescription();
//    u2ptr1->refreshDependents();
//    qDebug() << SB_DEBUG_INFO << u2ptr1->genericDescription();
//    SBIDPerformerPtr u2ptr2=SBIDPerformer::retrievePerformer(2078);
//    qDebug() << SB_DEBUG_INFO << u2ptr2->genericDescription();


    qDebug() << SB_DEBUG_INFO << "playground end";
    return;
}

void
Controller::configureMenus()
{
    const MainWindow* mw=Context::instance()->getMainWindow();

    configureMenuItems(mw->ui.menuFile->actions());
    configureMenuItems(mw->ui.menuPlaylist->actions());
    configureMenuItems(mw->ui.menuTools->actions());
}

void
Controller::configureMenuItems(const QList<QAction *>& list)
{
    QList<QAction *>::const_iterator it;
    QAction* i;

    for(it=list.begin(); it!=list.end(); ++it)
    {
        i=(*it);
        const QString& itemName=(*it)->objectName();
        if(itemName=="menuNewDatabase")
        {
            connect(i,SIGNAL(triggered()),
                    this,SLOT(newDatabase()));
        }
        else if(itemName=="menuOpenDatabase")
        {
            connect(i,SIGNAL(triggered()),
                    this,SLOT(openDatabase()));
        }
        else if(itemName=="menuNewPlaylist")
        {
            connect(i,SIGNAL(triggered()),
                    Context::instance()->getChooser(), SLOT(playlistNew()));
        }
        else if(itemName=="menuDeletePlaylist")
        {
            connect(i,SIGNAL(triggered()),
                    Context::instance()->getChooser(), SLOT(playlistDelete()));
        }
        else if(itemName=="menuRenamePlaylist")
        {
            connect(i,SIGNAL(triggered()),
                    Context::instance()->getChooser(), SLOT(playlistRename()));
        }
        else if(itemName=="menuRecalculatePlaylist")
        {
            connect(i,SIGNAL(triggered()),
                    Context::instance()->getChooser(), SLOT(recalculateDuration()));
        }
        else if(itemName=="menuSetMusicLibrary")
        {
            connect(i,SIGNAL(triggered()),
                    this, SLOT(setMusicLibraryDirectory()));
        }
        else if(itemName=="menuRescanMusicLibrary")
        {
            connect(i,SIGNAL(triggered()),
                    this, SLOT(rescanMusicLibrary()));
        }
        else
        {
            qDebug() << SB_DEBUG_INFO << "default:objectName=" << (*it)->objectName();
        }
    }
}

void
Controller::setFontSizes() const
{
    if(_app->platformName()=="windows")
    {
        QWidgetList l=_app->allWidgets();
        for(int i=0;i<l.count();i++)
        {
            QWidget* w=l.at(i);
            const QString cn=w->metaObject()->className();
            const QString on=w->objectName();
            if(cn=="QLabel")
            {
                QLabel* l=dynamic_cast<QLabel* >(w);
                if(l)
                {
                    QFont f=l->font();
                    f.setPointSize( (f.pointSize()-12 > 9 ? f.pointSize()-12 : 9));
                    l->setFont(f);
                }
            }
        }
    }
}
void
Controller::init()
{
    slP=NULL;
    connect(&statusBarResetTimer, SIGNAL(timeout()),
            this, SLOT(_resetStatusBar()));

    //	Instantiate background thread
    bgt=new BackgroundThread;
    bgt->moveToThread(&backgroundThread);
    backgroundThread.start();
    Context::instance()->setBackgroundThread(bgt);

    //	Recalculate playlist duration -- NOT USED ANYMORE
    //	CODE LEFT AS EXAMPLE
//    updateAllPlaylistDurationTimer.start(10*60*1000);	//	start recalc in 20s
//    statusBarResetTimer.setSingleShot(0);
//    connect(&updateAllPlaylistDurationTimer, SIGNAL(timeout()),
//            this, SLOT(_updateAllplaylistDurations()));
//    connect(this, SIGNAL(recalculateAllPlaylistDurations()),
//            bgt, SLOT(recalculateAllPlaylistDurations()));
}

///	PRIVATE SLOTS
void
Controller::_disableScreenNavigationButtons()
{
    MainWindow* mw=Context::instance()->getMainWindow();
    mw->ui.buttonBackward->setEnabled(0);
    mw->ui.buttonForward->setEnabled(0);
}

void
Controller::_resetStatusBar()
{
    Context::instance()->getMainWindow()->ui.statusBar->setText(SB_DEFAULT_STATUS);
}

//	NOT USED ANYMORE -- CODE LEFT AS EXAMPLE
//void
//Controller::_updateAllplaylistDurations()
//{
//    updateAllPlaylistDurationTimer.start(60*30*1000);	//	Every half hour
//    emit recalculateAllPlaylistDurations();
//}
