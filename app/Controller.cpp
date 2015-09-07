    #include <QDebug>
    #include <QKeyEvent>
    #include <QScrollBar>
    #include <QCompleter>
    #include <QDialog>
    #include <QMessageBox>
    #include <QSplashScreen>
    #include <QSqlDatabase>
    #include <QStyleFactory>
#include <QTimer>

#include "BackgroundThread.h"
#include "Common.h"
#include "Context.h"
#include "Controller.h"
#include "MainWindow.h"
#include "DataAccessLayer.h"
#include "DisplayOnlyDelegate.h"
#include "DatabaseSelector.h"
#include "ExternalData.h"
#include "LeftColumnChooser.h"
#include "SBModelSong.h"
#include "SBModelPlaylist.h"
#include "SBModelGenrelist.h"
#include "SBID.h"
#include "SBModelPerformer.h"
#include "SBSqlQueryModel.h"
#include "SBStandardItemModel.h"
#include "ScreenStack.h"
#include "SonglistScreenHandler.h"

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

    //	Songlist
    SBSqlQueryModel* sm=SBModelSong::getAllSongs();
    QList<bool> dragableColumns;
    dragableColumns.clear();
    dragableColumns << 0 << 0 << 0 << 1 << 0 << 0 << 1 << 0 << 0 << 1 << 0 << 0;
    sm->setDragableColumns(dragableColumns);
    qDebug() << SB_DEBUG_INFO;

    slP=new QSortFilterProxyModel();
    slP->setSourceModel(sm);
    mw->ui.allSongsList->setModel(slP);
    qDebug() << SB_DEBUG_INFO;

    //	Completers
    QCompleter* completer;

    //		A.	All items
    completer=new QCompleter(mw);
    completer->setModel(Context::instance()->getDataAccessLayer()->getCompleterModelAll());
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setModelSorting(QCompleter::CaseSensitivelySortedModel);
    completer->setFilterMode(Qt::MatchStartsWith);
    mw->ui.searchEdit->setCompleter(completer);
    qDebug() << SB_DEBUG_INFO;

    //		B.	Songs only
    completer=new QCompleter(mw);
    completer->setModel(Context::instance()->getDataAccessLayer()->getCompleterModelSong());
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setModelSorting(QCompleter::CaseSensitivelySortedModel);
    completer->setFilterMode(Qt::MatchStartsWith);
    mw->ui.songEditTitle->setCompleter(completer);

    //		C.	Performers only
    completer=new QCompleter(mw);
    completer->setModel(Context::instance()->getDataAccessLayer()->getCompleterModelPerformer());
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setModelSorting(QCompleter::CaseSensitivelySortedModel);
    completer->setFilterMode(Qt::MatchStartsWith);

    mw->ui.songEditPerformerName->setCompleter(completer);
    mw->ui.performerEditName->setCompleter(completer);

    //	SEARCH
    QCompleter* c=mw->ui.searchEdit->completer();
    connect(
        c, SIGNAL(activated(const QModelIndex&)),
        this, SLOT(openItemFromCompleter(const QModelIndex&)));
    connect(
        mw->ui.searchEdit,SIGNAL(returnPressed()),
        Context::instance()->getSonglistScreenHandler(),SLOT(applySonglistFilter()));
    connect(
        c, SIGNAL(activated(QString)),
        mw->ui.searchEdit, SLOT(clear()),
        Qt::QueuedConnection);	//	this will clear the search box

}

//SLOTS:

void
Controller::openDatabase()
{
    openMainWindow(0);
}

///
/// \brief Controller::applyGenreSelection
/// \param selected
/// \param deselected
///
void
Controller::applyGenreSelection(const QItemSelection &selected, const QItemSelection &deselected)
{
    Q_UNUSED(selected);
    Q_UNUSED(deselected);
//    MainWindow* mw=Context::instance()->getMainWindow();
//    QModelIndex index;
//    QModelIndexList items = selected.indexes();
//
//    qDebug() << "applyGenreSelection(const QItemSelecti";
//
//    clearSearchFilter();
//    clearPlaylistSelection();
//
//    foreach (index, items)
//    {
//        const int i= index.row();
//        const int j= index.column();
//        const QString& selectedGenre=mw->ui.genreList->model()->data(index.sibling(i,j)).toString();
//        selectedGenres.push_back(selectedGenre);
//        selectedGenres.removeDuplicates();
//    }
//
//    items=deselected.indexes();
//    foreach (index, items)
//    {
//        const int i= index.row();
//        const int j= index.column();
//        const QString& selectedGenre= mw->ui.genreList->model()->data(index.sibling(i,j)).toString();
//        selectedGenres.removeAt(selectedGenres.indexOf(selectedGenre));
//    }
//    qDebug() << "applyGenreSelection:selectedGenres=" << selectedGenres;
//
//    updateCurrentSongList();
}

void
Controller::changeSchema(const QString& newSchema)
{
    qDebug() << "Controller:changeSchema:new schema=" << newSchema;
    if(Context::instance()->getDataAccessLayer()->setSchema(newSchema))
    {
        //	refresh all views
        resetAllFiltersAndSelections();
        setupModels();
        setupUI();
    }
}

void
Controller::openItemFromCompleter(const QModelIndex& i) const
{
    qDebug() << SB_DEBUG_INFO << i;
    qDebug() << SB_DEBUG_INFO << "parameter:index=" << i.row() << i.column();
    Context::instance()->getSonglistScreenHandler()->showScreenStack();

    //	Retrieve SB_ITEM_TYPE and SB_ITEM_ID from index.
    SBID id;
    id.assign(i.sibling(i.row(), i.column()+2).data().toString(), i.sibling(i.row(), i.column()+1).data().toInt());

    Context::instance()->getSonglistScreenHandler()->openScreenByID(id);
    qDebug() << SB_DEBUG_INFO << id;
}

//	Data Updates
void
Controller::updateGenre(QModelIndex i, QModelIndex j)
{
    static int updateInProgress=0;
    Q_UNUSED(i);
    Q_UNUSED(j);
    Q_UNUSED(updateInProgress);

//    if(updateInProgress==0)
//    {
//        //	Poor man's semaphore
//        qDebug() << "Controller::updateGenre:start";
//        updateInProgress=1;
//            QString newGenre=Context::instance()->getDataAccessLayer()->updateGenre(i);
//            Context::instance()->getMainWindow()->ui.genreList->update();
//            clearGenreSelection();
//            updateCurrentSongList();
//
//        updateInProgress=0;
//    }
}

void
Controller::updateStatusBar(const QString &s)
{
    Context::instance()->getMainWindow()->ui.statusBar->setText(s);
    statusBarResetTimer.start(10000);
    statusBarResetTimer.setSingleShot(1);
}

//PROTECTED:

void
Controller::keyPressEvent(QKeyEvent *event)
{
    qDebug() << SB_DEBUG_INFO;
    if(event==NULL)
    {
        qDebug() << SB_DEBUG_NPTR << "*event";
        return;
    }
    MainWindow* mw=Context::instance()->getMainWindow();
    qDebug() << SB_DEBUG_INFO;
    if(event->key()==0x01000004 || event->key()==0x01000005)
    {
    qDebug() << SB_DEBUG_INFO;
        //	Return key
        Context::instance()->getSonglistScreenHandler()->handleEnterKey();

    }
    if(event->key()==0x1000000)
    {
        //	Catch escape key
        //	20150907: always call to clear search filter
        clearSearchFilter();
        Context::instance()->getSonglistScreenHandler()->handleEscapeKey();
        qDebug() << SB_DEBUG_INFO;
    }
    else if(event->key()==76 && event->modifiers() & Qt::ControlModifier)
    {
        //	Set focus to search edit and focus all available text if ctrl-L
        mw->ui.searchEdit->setFocus();
        mw->ui.searchEdit->selectAll();
    }
    else if(event->key()==85 && event->modifiers() & Qt::ControlModifier && mw->ui.searchEdit->hasFocus())
    {
        //	Clear searchedit if ctrl-U and if it has focus
        mw->ui.searchEdit->clear();
    }
    qDebug() << SB_DEBUG_INFO;
}

//PRIVATE:

void
Controller::resetAllFiltersAndSelections()
{
    clearGenreSelection();
    clearSearchFilter();
}

void
Controller::clearGenreSelection()
{
    //Context::instance()->getMainWindow()->ui.genreList->clearSelection();
    //selectedGenres.clear();
}

void
Controller::clearSearchFilter()
{
    currentFilter="";
    Context::instance()->getMainWindow()->ui.searchEdit->setText(tr(""));
    if(slP)
    {
        slP->setFilterFixedString(tr(""));
    }
}

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

        MainWindow* oldMW=Context::instance()->getMainWindow();
        if(oldMW)
        {
            Context::instance()->setMainWindow(NULL);
            oldMW->close();
            oldMW=NULL;
        }

        MainWindow* mw=new MainWindow();
        Context::instance()->setMainWindow(mw);

        init();

        SonglistScreenHandler* ssh=new SonglistScreenHandler();

        Context::instance()->setSonglistScreenHandler(ssh);

        DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
        if(dal)
        {
            delete dal;
            dal=NULL;
        }
        Context::instance()->setDataAccessLayer(NULL);

        dal=ds->getDataAccessLayer();
        Context::instance()->setDataAccessLayer(dal);

        resetAllFiltersAndSelections();

        setupModels();

        setupUI();

        configureMenus();

        mw->setWindowTitle(mw->windowTitle() + " - " + ds->databaseName() + " ("+Context::instance()->getDataAccessLayer()->getDriverName()+")");

        SBModelSong::updateSoundexFields();
        SBModelPerformer::updateSoundexFields();

        _resetStatusBar();

        if(startup)
        {
            I::sleep(1);
            splash.finish(mw);
        }
        mw->show();

        ssh->openOpener(QString());

    }
    qDebug() << SB_DEBUG_INFO << "openMainWindow:end";
    return 1;
}

void
Controller::setupModels()
{
    const MainWindow* mw=Context::instance()->getMainWindow();
    qDebug() << SB_DEBUG_INFO;

    refreshModels();

    ///	LeftColumnChooser
    LeftColumnChooser* lcc=new LeftColumnChooser();
    Context::instance()->setLeftColumnChooser(lcc);

    const SBStandardItemModel *m=Context::instance()->getLeftColumnChooser()->getModel();
    mw->ui.leftColumnChooser->setModel((QStandardItemModel *)m);	//	weird that it won't accept a subclass

}

void
Controller::setupUI()
{
    qDebug() << SB_DEBUG_INFO;

    //	Frequently used pointers
    QHeaderView* hv;
    MainWindow* mw=Context::instance()->getMainWindow();

    qDebug() << "Controller:setupUI:start";

    ///	SONGLIST

    //	general
    mw->ui.allSongsList->setSortingEnabled(1);
    mw->ui.allSongsList->sortByColumn(3,Qt::AscendingOrder);
    mw->ui.allSongsList->setSelectionMode(QAbstractItemView::SingleSelection);
    mw->ui.allSongsList->setSelectionBehavior(QAbstractItemView::SelectItems);
    mw->ui.allSongsList->setFocusPolicy(Qt::StrongFocus);
    mw->ui.allSongsList->selectionModel();

    //	horizontal header
    hv=mw->ui.allSongsList->horizontalHeader();
    hv->setSortIndicator(3,Qt::AscendingOrder);
    hv->setSortIndicatorShown(1);
    hv->setSectionResizeMode(QHeaderView::Stretch);

    //	vertical header
    hv=mw->ui.allSongsList->verticalHeader();
    hv->setDefaultSectionSize(18);
    hv->hide();
    Common::hideColumns(mw->ui.allSongsList);

    //	set up signals
    mw->ui.allSongsList->selectionModel();
    //		capture double click to open detail page
    connect(mw->ui.allSongsList, SIGNAL(clicked(QModelIndex)),
            Context::instance()->getSonglistScreenHandler(),SLOT(openSonglistItem(QModelIndex)));

    //	drag & drop revisited.
    QTableView* allSongsList=mw->ui.allSongsList;

    allSongsList->setDragEnabled(true);
    allSongsList->setDropIndicatorShown(true);

    ///	Statusbar
    mw->ui.statusBar->setReadOnly(true);

    ///	BUTTONS
    mw->ui.buttonBackward->setEnabled(0);
    mw->ui.buttonForward->setEnabled(0);

    SonglistScreenHandler* ssh=Context::instance()->getSonglistScreenHandler();
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
            Context::instance()->getSonglistScreenHandler(), SLOT(openLeftColumnChooserItem(QModelIndex)));
    tv->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(tv, SIGNAL(customContextMenuRequested(const QPoint&)),
            Context::instance()->getLeftColumnChooser(), SLOT(showContextMenu(QPoint)));


    ///	MISC
    QTabBar* tb=mw->ui.songlistTab->tabBar();
    tb->hide();

    this->setFontSizes();

    qDebug() << SB_DEBUG_INFO;
    return;
}

void
Controller::configureMenus()
{
    const MainWindow* mw=Context::instance()->getMainWindow();

    configureMenuItems(mw->ui.menuFile->actions());
    configureMenuItems(mw->ui.menuPlaylist->actions());
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
            connect(i,SIGNAL(triggered()),this,SLOT(openDatabase()));
        }
        else if(itemName=="menuNewPlaylist")
        {
            connect(i,SIGNAL(triggered()),
                    Context::instance()->getLeftColumnChooser(), SLOT(newPlaylist()));
        }
        else if(itemName=="menuDeletePlaylist")
        {
            connect(i,SIGNAL(triggered()),
                    Context::instance()->getLeftColumnChooser(), SLOT(deletePlaylist()));
        }
        else if(itemName=="menuRenamePlaylist")
        {
            connect(i,SIGNAL(triggered()),
                    Context::instance()->getLeftColumnChooser(), SLOT(renamePlaylist()));
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
            qDebug() << SB_DEBUG_INFO << "I hear a lama pooping";
            QWidgetList l=app->allWidgets();
            for(int i=0;i<l.count();i++)
            {
                QWidget* w=l.at(i);
                const QString cn=w->metaObject()->className();
                const QString on=w->objectName();
                if(cn=="QLabel")
                {
                    qDebug() << SB_DEBUG_INFO << cn << on;
                    QLabel* l=dynamic_cast<QLabel* >(w);
                    if(l)
                    {
                        QFont f=l->font();
                        qDebug() << SB_DEBUG_INFO << f.pointSize();
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
    currentFilter="";
    slP=NULL;
    connect(&statusBarResetTimer, SIGNAL(timeout()),
            this, SLOT(_resetStatusBar()));

    //	Instantiate background thread
    qDebug() << SB_DEBUG_INFO;
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

    qDebug() << SB_DEBUG_INFO << updateAllPlaylistDurationTimer.interval();
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
