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
#include "BuildDatetime.h"
#include "Chooser.h"
#include "Common.h"
#include "Context.h"
#include "Controller.h"
#include "DataAccessLayer.h"
#include "DBManager.h"
#include "MainWindow.h"
#include "MusicLibrary.h"
#include "Navigator.h"
#include "PlayerController.h"
#include "Preloader.h"
#include "Properties.h"
#include "SBIDBase.h"
#include "SBIDOnlinePerformance.h"
#include "SBMessageBox.h"
#include "SBTabSongsAll.h"
#include "SearchItemModel.h"
#include "SetupWizard.h"
#include "Preferences.h"

Controller::Controller(int argc, char *argv[], QApplication* app) : _app(app)
{
    Q_UNUSED(argc);
    Q_UNUSED(argv);
    Context::instance()->setController(this);

    _initSuccessFull=openMainWindow(1);
}

Controller::~Controller()
{
    _backgroundThread.quit();
    _backgroundThread.wait();
}
bool
Controller::initSuccessFull() const
{
    return _initSuccessFull;
}

void
Controller::preloadAllSongs() const
{
    MainWindow* mw=Context::instance()->mainWindow();
    SBTabSongsAll* tsa=mw->ui.tabAllSongs;
    tsa->preload();
}

void
Controller::refreshModels()
{
    Navigator* n=Context::instance()->navigator();
    n->resetAllFiltersAndSelections();

}

///	Public slots:
void
Controller::preferences()
{
    Preferences p;
}

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
    PlayerController* pc=Context::instance()->playerController();
    QMediaPlayer::PlaybackState state=pc->playState();

    if(state==QMediaPlayer::PlaybackState::PlayingState || state==QMediaPlayer::PlaybackState::PausedState)
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
            PlayManager* pm=Context::instance()->playManager();
            pm->playerStop();
            pm->clearPlaylist();
        }
    }
    openMainWindow(0);
}

void
Controller::setMusicLibraryDirectory()
{
    Context::instance()->properties()->userSetMusicLibraryDirectory();
}

void
Controller::checkConsistency()
{
    MusicLibrary ml;
    ml.consistencyCheck();
}

void
Controller::rescanMusicLibrary()
{
    MusicLibrary ml;
    ml.rescanMusicLibrary();
}

void
Controller::changeCurrentDatabaseSchema(const QString& newSchema)
{
    PropertiesPtr properties=Context::instance()->properties();

    if(properties->currentDatabaseSchema()!=newSchema)
    {
        if(properties->setCurrentDatabaseSchema(newSchema))
        {
            //	Clear caches
            CacheManager* cm=Context::instance()->cacheManager();
            cm->clearAllCaches();

            //	refresh all views
            refreshModels();
            emit databaseSchemaChanged();
        }
    }
    this->_disableScreenNavigationButtons();
}

void
Controller::updateStatusBarText(const QString &s)
{
    Context::instance()->mainWindow()->ui.statusBar->setText(s);
    _statusBarResetTimer.start(10000);
    _statusBarResetTimer.setSingleShot(1);
}

void
Controller::logSongPlayedHistory(bool radioModeFlag,SBKey onlinePerformanceKey)
{
    _logSongPlayedHistory.start(13000);
    _logSongPlayedHistory.setSingleShot(1);
    _logSongPlayedRadioModeFlag=radioModeFlag;
    _logOnlinePerformanceKey=onlinePerformanceKey;
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
    CacheManager* cm=new CacheManager();
    Context::instance()->setCacheManager(cm);

    //	Instantiate DatabaseSelector, check if database could be opened.
    DBManager* dbm=Context::instance()->dbManager();
    if(appStartUpFlag)
    {
        bool openedFlag=dbm->openDefaultDatabase();
        if(!openedFlag)
        {
            //	Start wizard
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

    QPixmap pm(":/images/splash.png");
    QSplashScreen splash(pm);

    if(appStartUpFlag)
    {
        splash.show();
        _app->processEvents();
    }

    MainWindow* oldMW=Context::instance()->mainWindow(0);
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

    setupModels();

    refreshModels();

    setupUI();

    configureMenus();

    mw->setWindowTitle(
        mw->windowTitle() + " - " +
        dbm->databaseName() + " ("+Context::instance()->dataAccessLayer()->getDriverName()+")" + " - "
        "Build: " + COMPILE_TIME
    );

    SBIDSong::updateSoundexFields();
    SBIDPerformer::updateSoundexFields();

    _resetStatusBar();

    if(appStartUpFlag)
    {
        splash.finish(mw);
        qDebug() << SB_DEBUG_INFO << "splash screen done";
    }

    mw->show();
    Context::instance()->navigator()->openOpener();

    //	Kick off import
    PropertiesPtr properties=Context::instance()->properties();
    if(properties->configValue(Configuration::sb_run_import_on_startup_flag)=="1")
    {
        MusicLibrary ml;
        ml.rescanMusicLibrary();
    }
    properties->setConfigValue(Configuration::sb_run_import_on_startup_flag,"0");

    return 1;
}

void
Controller::setupUI()
{
    //	Frequently used pointers
    MainWindow* mw=Context::instance()->mainWindow();


    ///	Statusbar
    mw->ui.statusBar->setReadOnly(true);

    ///	BUTTONS
    this->_disableScreenNavigationButtons();

    Navigator* ssh=Context::instance()->navigator();
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
            Context::instance()->navigator(), SLOT(openChooserItem(QModelIndex)));
    tv->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(tv, SIGNAL(customContextMenuRequested(const QPoint&)),
            Context::instance()->chooser(), SLOT(showContextMenu(QPoint)));


    ///	MISC

    mw->ui.mainTab->tabBar()->hide();
    this->setFontSizes();
    mw->ui.searchEdit->setFocus();

    //	Have SBTabSongsAll start populating
    this->preloadAllSongs();

    //	Set up schema dropdown box
    DataAccessLayer* dal=Context::instance()->dataAccessLayer();

    if(dal->supportSchemas())
    {
        mw->ui.cbSchema->clear();
        mw->ui.cbSchema->insertItems(0,dal->availableSchemas());
        mw->ui.cbSchema->setVisible(1);
        mw->ui.labelSchema->setVisible(1);
        mw->ui.frSchema->setVisible(1);

        connect(mw->ui.cbSchema,SIGNAL(currentTextChanged(QString)),
                this,SLOT(changeCurrentDatabaseSchema(QString)));

        PropertiesPtr properties=Context::instance()->properties();

        //	Set current schema properly in schema drop down box
        QString currentSchema=properties->currentDatabaseSchema();
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

    Preloader::loadAll();


    /*
    qDebug() << SB_DEBUG_INFO << "playground start";
    {
        int max=100;
        quint64 smallest=max;
        quint64 biggest=0;
        int maxBuckets=10;
        QVector<int> v(maxBuckets);
        for(int i=0; i<200000;i++)
        {
            quint64 x=Common::randomOldestFirst(max);
            v[x/maxBuckets]++;
            smallest=x<smallest?x:smallest;
            biggest=x>biggest?x:biggest;
        }
        int sum=0;
        for(int i=0; i<v.length(); i++)
        {
            sum+=v[i];
        }
        for(int i=0; i<v.length(); i++)
        {
            QString t=QString("*").repeated(v[i]*100/sum);
            qDebug() << SB_DEBUG_INFO << i << t;
        }
        qDebug() << SB_DEBUG_INFO << smallest << biggest;
    }
    */

    if(0)
    {
        int maxNumberToRandomize=500;
        int numPerformances=2*26+10;
        QString indexCovered=QString(".").repeated(maxNumberToRandomize);
        indexCovered+=QString("");
        int index=0;
        while(index<numPerformances)
        {
            bool found=0;
            int idx=-1;

            int rnd=Common::randomOldestFirst(maxNumberToRandomize);

            //	Find first untaken spot, counting untaken spots.
            idx=0;
            for(int i=0;i<maxNumberToRandomize && !found;i++)
            {
                //	qDebug() << SB_DEBUG_INFO << index << indexCovered.left(100) << i << idx << rnd;
                //	QString ptr=QString("%1%2").arg(QString(".").repeated(i)).arg("^");
                //	qDebug() << SB_DEBUG_INFO << index << ptr.left(100);

                if(indexCovered.at(i)=='.')
                {
                    if(idx==rnd)
                    {
                        QChar c;
                        if(index>=0 && index<10)
                        {
                            c=QChar(48+index);
                        }
                        else if(index>=10 && index<36)
                        {
                            c=QChar(65+index-10);
                        }
                        else
                        {
                            c=QChar(97+index-36);
                        }
                        indexCovered.replace(i,1,c);
                        found=1;
                    }
                    idx++;
                }
            }
            index++;
        }
    }

    if(0)
    {
        const qsizetype maxNumberToRandomize=5000;
        const qsizetype numIterations=50000000;

        QHash<qsizetype,qsizetype> d5000;
        QHash<qsizetype,qsizetype> d500;
        QHash<qsizetype,qsizetype> d50;
        QHash<qsizetype,qsizetype> d5;

        for(qsizetype i=0;i<5000;i++)
        {
            d5000[i]=0;
        }
        for(qsizetype i=0;i<500;i++)
        {
            d500[i]=0;
        }
        for(qsizetype i=0;i<50;i++)
        {
            d50[i]=0;
        }
        for(qsizetype i=0;i<5;i++)
        {
            d5[i]=0;
        }

        qDebug() << SB_DEBUG_INFO;
        for(qsizetype i=0;i<numIterations;i++)
        {
            if(i % 100000==0)
            {
                qDebug() << SB_DEBUG_INFO << "Generated" << i << "random numbers";
            }
            qsizetype rnd=Common::randomOldestFirst(maxNumberToRandomize);

            d5000[rnd]=d5000[rnd]+1;
            rnd=rnd/10;
            d500[rnd]=d500[rnd]+1;
            rnd=rnd/10;
            d50[rnd]=d50[rnd]+1;
            rnd=rnd/10;
            d5[rnd]=d5[rnd]+1;
        }
        for(qsizetype i=0;i<10;i++)
        {
            qDebug() << SB_DEBUG_INFO << i << d5000[i];
        }
        qDebug() << SB_DEBUG_INFO << d5000.size() << d500.size() << d50.size() << d5.size();

        // 50 -> 50/10 ->
        const qsizetype lineWidth=200;
        qsizetype divider;
        qsizetype total=0;
        divider=d5000[0]/lineWidth;
        qDebug() << SB_DEBUG_INFO << divider << d5000[0];
        total=0;
        for(qsizetype i=0;i<5000;i++)
        {
            qsizetype numChars=d5000[i]/divider;
            QString indexCovered=QString("#").repeated(numChars);
            total+=d5000[i];
            if(i<200)
            {
                qDebug() << SB_DEBUG_INFO << QString::asprintf("%07d",i) << QString::asprintf("%07d",d5000[i]) << indexCovered;
            }
        }
        qDebug() << SB_DEBUG_INFO << total;
        divider=d50[0]/lineWidth;
        total=0;
        for(qsizetype i=0;i<d50.size();i++)
        {
            qsizetype numChars=d50[i]/divider;
            QString indexCovered=QString("#").repeated(numChars);
            total+=d50[i];
            qDebug() << SB_DEBUG_INFO << QString::asprintf("%07d",i) << QString::asprintf("%07d",d50[i]) << indexCovered;
        }
        qDebug() << SB_DEBUG_INFO << total;
    }

    /*
     * key testing
    SBKey key;
    QByteArray k;

    k="2:100"; key=SBKey(k); qDebug() << SB_DEBUG_INFO << k << key << key.itemType() << key.itemID() << key.validFlag();
    k="12:100"; key=SBKey(k); qDebug() << SB_DEBUG_INFO << k << key << key.itemType() << key.itemID() << key.validFlag();
    k="-1:100"; key=SBKey(k); qDebug() << SB_DEBUG_INFO << k << key << key.itemType() << key.itemID() << key.validFlag();
    k="2:-100"; key=SBKey(k); qDebug() << SB_DEBUG_INFO << k << key << key.itemType() << key.itemID() << key.validFlag();
     */


    /*
    qDebug() << SB_DEBUG_INFO << "natural logaritm";
    int maxRandom=0;
    int maxSongs=5000;
    for(int i=0;i<=maxSongs;i++)
    {
        int j= 1 + (-80 * log(i+1))+(80 * log(maxSongs+1));
        maxRandom+=j;
        qDebug() << SB_DEBUG_INFO << i << j << maxRandom;
    }
    qDebug() << SB_DEBUG_INFO << maxRandom;
    */

	/*
    QString t;
    t="Goo Goo Dolls";
    qDebug() << SB_DEBUG_INFO << Common::removeNonAlphanumericIncludingSpaces(t);
    t="Goo Goo Dolls!";
    qDebug() << SB_DEBUG_INFO << Common::removeNonAlphanumericIncludingSpaces(t);
	*/

	/*
    t=Network::hostName();
    qDebug() << SB_DEBUG_INFO << t;

    qDebug() << SB_DEBUG_INFO << "playground end";
	*/
    return;
}

void
Controller::configureMenus()
{
    const MainWindow* mw=Context::instance()->mainWindow();

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
		if(itemName=="menuPreferences")
		{
            connect(i,SIGNAL(triggered()),
                    this,SLOT(preferences()));
		}
        else if(itemName=="menuNewDatabase")
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
                    Context::instance()->chooser(), SLOT(playlistNew()));
        }
        else if(itemName=="menuDeletePlaylist")
        {
            connect(i,SIGNAL(triggered()),
                    Context::instance()->chooser(), SLOT(playlistDelete()));
        }
        else if(itemName=="menuRenamePlaylist")
        {
            connect(i,SIGNAL(triggered()),
                    Context::instance()->chooser(), SLOT(playlistRename()));
        }
        else if(itemName=="menuRecalculatePlaylist")
        {
            connect(i,SIGNAL(triggered()),
                    Context::instance()->chooser(), SLOT(recalculateDuration()));
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
        else if(itemName=="menuConsistencyChecker")
        {
            connect(i,SIGNAL(triggered()),
                    this, SLOT(checkConsistency()));
        }
        else
        {
            qDebug() << SB_DEBUG_WARNING << "default:objectName=" << (*it)->objectName();
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
Controller::setupModels()
{
    MainWindow* mw=Context::instance()->mainWindow();
    Navigator* n=Context::instance()->navigator();

    SearchItemModel* sim=new SearchItemModel();
    Context::instance()->setSearchItemModel(sim);

    n->resetAllFiltersAndSelections();

    //	SEARCH
    QCompleter* c=mw->ui.searchEdit->completer();
    connect(
        c, SIGNAL(activated(const QModelIndex&)),
        Context::instance()->navigator(), SLOT(openItemFromCompleter(const QModelIndex&)));
    connect(
        mw->ui.searchEdit,SIGNAL(returnPressed()),
        Context::instance()->navigator(),SLOT(applySonglistFilter()));
    connect(
        c, SIGNAL(activated(QString)),
        mw->ui.searchEdit, SLOT(clear()),
        Qt::QueuedConnection);	//	this will clear the search box

}

void
Controller::init()
{
    slP=NULL;
    connect(&_statusBarResetTimer, SIGNAL(timeout()),
            this, SLOT(_resetStatusBar()));
    connect(&_logSongPlayedHistory, SIGNAL(timeout()),
            this, SLOT(_performLogSongPlayedHistory()));
}

///	PRIVATE SLOTS
void
Controller::_disableScreenNavigationButtons()
{
    MainWindow* mw=Context::instance()->mainWindow();
    mw->ui.buttonBackward->setEnabled(0);
    mw->ui.buttonForward->setEnabled(0);
}

void
Controller::_performLogSongPlayedHistory()
{
    DataAccessLayer* dal=Context::instance()->dataAccessLayer();
    SBIDOnlinePerformancePtr opPtr=SBIDOnlinePerformance::retrieveOnlinePerformance(this->_logOnlinePerformanceKey);
    dal->logSongPlayed(_logSongPlayedRadioModeFlag,opPtr);
}

void
Controller::_resetStatusBar()
{
    Context::instance()->mainWindow()->ui.statusBar->setText(SB_DEFAULT_STATUS);
}

//	NOT USED ANYMORE -- CODE LEFT AS EXAMPLE
//void
//Controller::_updateAllplaylistDurations()
//{
//    _updateAllPlaylistDurationTimer.start(60*30*1000);	//	Every half hour
//    emit recalculateAllPlaylistDurations();
//}
