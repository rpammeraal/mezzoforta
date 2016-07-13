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
#include "DataEntitySong.h"
#include "DataEntityPerformer.h"
#include "DataEntityPlaylist.h"
#include "DatabaseSelector.h"
#include "ExternalData.h"
#include "MainWindow.h"
#include "MusicLibrary.h"
#include "Navigator.h"
#include "PlayerController.h"
#include "Properties.h"
#include "SBID.h"
#include "SBSqlQueryModel.h"
#include "SBStandardItemModel.h"
#include "SBTabSongsAll.h"
#include "ScreenStack.h"

class I : public QThread
{
public:
    static void sleep(unsigned long secs) { QThread::sleep(secs); }
};

Controller::Controller(int argc, char *argv[], QApplication* napp) : app(napp)
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

bool
Controller::initSuccessFull() const
{
    return _initSuccessFull;
}

void
Controller::refreshModels()
{
    //	Allows some data models to be refreshed
    MainWindow* mw=Context::instance()->getMainWindow();

    //	Completers

    mw->ui.searchEdit->setCompleter(CompleterFactory::getCompleterAll());
    mw->ui.songEditTitle->setCompleter(CompleterFactory::getCompleterSong());
    mw->ui.songEditPerformerName->setCompleter(CompleterFactory::getCompleterPerformer());
    mw->ui.performerEditName->setCompleter(CompleterFactory::getCompleterPerformer());

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

}

///	Public slots:
void
Controller::openDatabase()
{
    openMainWindow(0);
}

void
Controller::setMusicLibraryDirectory()
{
    Context::instance()->getProperties()->setMusicLibraryDirectory();
}

void
Controller::rescanMusicLibrary()
{
    qDebug() << SB_DEBUG_INFO;
    MusicLibrary ml;
    ml.rescanMusicLibrary();
}

void
Controller::changeSchema(const QString& newSchema)
{
    if(Context::instance()->getDataAccessLayer()->setSchema(newSchema))
    {
        //	refresh all views
        Context::instance()->getNavigator()->resetAllFiltersAndSelections();
        refreshModels();
        setupUI();
    }
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
/// \param startup
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
Controller::openMainWindow(bool startup)
{
    qDebug() << SB_DEBUG_INFO << "openMainWindow:start";

    //	Instantiate DatabaseSelector, check if database could be opened.
    DatabaseSelector* ds=new DatabaseSelector(startup);

    if(startup && ds->databaseOpen()==0)
    {
        //	no database opened upin startup.
        return 0;
    }

    if(ds->databaseChanged() || startup)
    {
        QPixmap pm(":/images/moose7.2.png");
        qDebug() << SB_DEBUG_INFO << pm.isNull();
        QSplashScreen splash(pm);

        if(startup)
        {
            splash.show();
            app->processEvents();
        }
        qDebug() << SB_DEBUG_INFO;

        MainWindow* oldMW=Context::instance()->getMainWindow();
        if(oldMW)
        {
            Context::instance()->setMainWindow(NULL);
            oldMW->close();
            oldMW=NULL;
        }
        qDebug() << SB_DEBUG_INFO;

        DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
        if(dal)
        {
            delete dal;
            dal=NULL;
        }
        dal=ds->getDataAccessLayer();

        MainWindow* mw=new MainWindow();
        SB_DEBUG_IF_NULL(mw);
        qDebug() << SB_DEBUG_INFO;
        Context::instance()->doInit(mw,dal);	//	This has to be done as soon as we have mw

        init();

        qDebug() << SB_DEBUG_INFO;
        Navigator* n=Context::instance()->getNavigator();
        n->resetAllFiltersAndSelections();

        refreshModels();

        setupUI();

        configureMenus();

        mw->setWindowTitle(mw->windowTitle() + " - " + ds->databaseName() + " ("+Context::instance()->getDataAccessLayer()->getDriverName()+")");

        DataEntitySong::updateSoundexFields();
        DataEntityPerformer::updateSoundexFields();

        _resetStatusBar();

        if(startup)
        {
            splash.finish(mw);
        }
        mw->show();

        Context::instance()->getNavigator()->openOpener();

    }
    qDebug() << SB_DEBUG_INFO << "openMainWindow:end";
    return 1;
}

void
Controller::setupUI()
{
    qDebug() << SB_DEBUG_INFO;

    //	Frequently used pointers
    MainWindow* mw=Context::instance()->getMainWindow();

    qDebug() << "Controller:setupUI:start";

    ///	Statusbar
    mw->ui.statusBar->setReadOnly(true);

    ///	BUTTONS
    mw->ui.buttonBackward->setEnabled(0);
    mw->ui.buttonForward->setEnabled(0);

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
    SBTabSongsAll* tsa=mw->ui.tabAllSongs;
    tsa->preload();

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
        if(itemName=="menuOpenDatabase")
        {
            connect(i,SIGNAL(triggered()),
                    this,SLOT(openDatabase()));
        }
        else if(itemName=="menuNewPlaylist")
        {
            connect(i,SIGNAL(triggered()),
                    Context::instance()->getChooser(), SLOT(newPlaylist()));
        }
        else if(itemName=="menuDeletePlaylist")
        {
            connect(i,SIGNAL(triggered()),
                    Context::instance()->getChooser(), SLOT(deletePlaylist()));
        }
        else if(itemName=="menuRenamePlaylist")
        {
            connect(i,SIGNAL(triggered()),
                    Context::instance()->getChooser(), SLOT(renamePlaylist()));
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
    qDebug() << SB_DEBUG_INFO << app->platformName();
        if(app->platformName()=="windows")
        {
            QWidgetList l=app->allWidgets();
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

    //	Recalculate playlist duration
    updateAllPlaylistDurationTimer.start(10*60*1000);	//	start recalc in 20s
    statusBarResetTimer.setSingleShot(0);
    connect(&updateAllPlaylistDurationTimer, SIGNAL(timeout()),
            this, SLOT(_updateAllplaylistDurations()));
    connect(this, SIGNAL(recalculateAllPlaylistDurations()),
            bgt, SLOT(recalculateAllPlaylistDurations()));
}

///	PRIVATE SLOTS
void
Controller::_resetStatusBar()
{
    qDebug() << SB_DEBUG_INFO;
    Context::instance()->getMainWindow()->ui.statusBar->setText(SB_DEFAULT_STATUS);
}

void
Controller::_updateAllplaylistDurations()
{
    updateAllPlaylistDurationTimer.start(60*30*1000);	//	Every half hour
    qDebug() << SB_DEBUG_INFO << updateAllPlaylistDurationTimer.interval();
    emit recalculateAllPlaylistDurations();
}
