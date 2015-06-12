#include <QDebug>
#include <QKeyEvent>
#include <QScrollBar>
#include <QCompleter>
#include <QDialog>
#include <QMessageBox>
#include <QSqlDatabase>
#include <QProgressDialog>

#include <QStyleFactory>

#include "Common.h"
#include "Context.h"
#include "Controller.h"
#include "MainWindow.h"
#include "DataAccessLayer.h"
#include "DisplayOnlyDelegate.h"
#include "DatabaseSelector.h"
#include "ExternalData.h"
#include "LeftColumnChooser.h"
#include "SBModel.h"
#include "SBModelSong.h"
#include "SBModelSonglist.h"
#include "SBModelPlaylist.h"
#include "SBModelGenrelist.h"
#include "SBID.h"
#include "ScreenStack.h"
#include "SonglistScreenHandler.h"

Controller::Controller(int argc, char *argv[])
{
    Q_UNUSED(argc);
    Q_UNUSED(argv);

    Context::instance()->setController(this);

    _initSuccessFull=openMainWindow(1);
}

Controller::~Controller()
{

}

bool
Controller::initSuccessFull() const
{
    return _initSuccessFull;
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
    }
}

void
Controller::openItemFromCompleter(const QModelIndex& i) const
{
    qDebug() << SB_DEBUG_INFO;
    Context::instance()->getSonglistScreenHandler()->showScreenStack();

    const MainWindow* mw=Context::instance()->getMainWindow();

    //	Retrieve SB_ITEM_TYPE and SB_ITEM_ID from index.
    SBID id;
    id.assign(i.sibling(i.row(), i.column()+2).data().toString(), i.sibling(i.row(), i.column()+1).data().toInt());
    qDebug() << SB_DEBUG_INFO;
    mw->ui.searchEdit->setText("");
    mw->ui.searchEdit->clear();
    qDebug() << SB_DEBUG_INFO;

    Context::instance()->getSonglistScreenHandler()->openScreenByID(id);
    qDebug() << SB_DEBUG_INFO << id;
}

void
Controller::songlistCellSelectionChanged(const QItemSelection &s, const QItemSelection &o)
{
    Q_UNUSED(o);
    qDebug() << SB_DEBUG_INFO << "row=" << s.indexes().at(0).row() << "column=" << s.indexes().at(0).column();
    Context::instance()->getSBModelSonglist()->setSelectedColumn(s.indexes().at(0).column());
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
Controller::updateStatusBar(const QString &s) const
{
    Context::instance()->getMainWindow()->ui.statusBar->setText(s);
}

//PROTECTED:

void
Controller::keyPressEvent(QKeyEvent *event)
{
    MainWindow* mw=Context::instance()->getMainWindow();
    if(event->key()==0x1000000)
    {
        if(currentFilter.length()>0)
        {
            qDebug() << SB_DEBUG_INFO;
            clearSearchFilter();
        }
        else
        {
            qDebug() << SB_DEBUG_INFO;
            //	Catch escape key, blank searchEdit, playlist, genres
            Context::instance()->getSonglistScreenHandler()->showSonglist();
        }
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
    qDebug() << SB_DEBUG_INFO << "openMainWindow:databaseChanged=" << ds->databaseChanged();

    if(ds->databaseChanged() || startup)
    {
        MainWindow* oldMW=Context::instance()->getMainWindow();

        qDebug() << SB_DEBUG_INFO;

        QProgressDialog p("Reading data...",QString(),0,8,oldMW);
        p.setWindowModality(Qt::WindowModal);

        qDebug() << SB_DEBUG_INFO;

        p.setValue(0);

        qDebug() << SB_DEBUG_INFO;

        initAttributes();

        qDebug() << SB_DEBUG_INFO;

        SonglistScreenHandler* ssh=new SonglistScreenHandler();

        qDebug() << SB_DEBUG_INFO;

        Context::instance()->setSonglistScreenHandler(ssh);

        qDebug() << SB_DEBUG_INFO;

        DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
        if(dal)
        {
            delete dal;
            dal=NULL;
        }

        qDebug() << SB_DEBUG_INFO;
        p.setValue(1);

        qDebug() << SB_DEBUG_INFO;
        dal=ds->getDataAccessLayer();
        Context::instance()->setDataAccessLayer(dal);

        qDebug() << SB_DEBUG_INFO;
        p.setValue(2);

        p.setValue(3);
        qDebug() << SB_DEBUG_INFO;
        MainWindow* mw=new MainWindow();
        Context::instance()->setMainWindow(mw);


        qDebug() << SB_DEBUG_INFO;
        p.setValue(4);
        resetAllFiltersAndSelections();

        qDebug() << SB_DEBUG_INFO;
        p.setValue(5);
        setupModels();

        qDebug() << SB_DEBUG_INFO;
        p.setValue(6);
        setupUI();

        qDebug() << SB_DEBUG_INFO;
        configureMenus();

        qDebug() << SB_DEBUG_INFO;
        mw->setWindowTitle(mw->windowTitle() + " - " + ds->databaseName() + " ("+Context::instance()->getDataAccessLayer()->getDriverName()+")");

        qDebug() << SB_DEBUG_INFO;

        p.setValue(7);

        qDebug() << SB_DEBUG_INFO;

        updateStatusBar(SB_DEFAULT_STATUS);

        mw->show();

        qDebug() << SB_DEBUG_INFO;
        mw->ui.songlistTab->setCurrentIndex(5);

        if(ds->databaseChanged() && startup==0)
        {
            //	close main window
            oldMW->close();
        }
        p.setValue(8);
    }
    qDebug() << "openMainWindow:end";
    return 1;
}

void
Controller::setupModels()
{
    MainWindow* mw=Context::instance()->getMainWindow();
    qDebug() << SB_DEBUG_INFO;

    //	songlist
    SBModelSonglist* sm=SBModelSong::getAllSongs();
    Context::instance()->setSBModelSonglist(sm);
    qDebug() << SB_DEBUG_INFO;

    slP=new QSortFilterProxyModel();
    slP->setSourceModel(sm);
    mw->ui.allSongsList->setModel(slP);
    qDebug() << SB_DEBUG_INFO;

    //	completer
    QCompleter* completer=new QCompleter(mw);
    completer->setModel(Context::instance()->getDataAccessLayer()->getCompleterModel());
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setModelSorting(QCompleter::CaseSensitivelySortedModel);
    completer->setFilterMode(Qt::MatchContains);
    mw->ui.searchEdit->setCompleter(completer);
    qDebug() << SB_DEBUG_INFO;

    //	leftColumnChooser
    mw->ui.leftColumnChooser->setModel(LeftColumnChooser::getModel());

}

void
Controller::setupUI()
{
    qDebug() << SB_DEBUG_INFO;

    //	Frequently used pointers
    QHeaderView* hv;
    QItemSelectionModel* sm;
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    MainWindow* mw=Context::instance()->getMainWindow();

    qDebug() << "Controller:setupUI:start";

    ///	SONGLIST

    //	general
    mw->ui.allSongsList->setSortingEnabled(1);
    mw->ui.allSongsList->sortByColumn(0,Qt::AscendingOrder);
    mw->ui.allSongsList->setSelectionMode(QAbstractItemView::SingleSelection);
    mw->ui.allSongsList->setSelectionBehavior(QAbstractItemView::SelectItems);
    mw->ui.allSongsList->setFocusPolicy(Qt::StrongFocus);
    sm=mw->ui.allSongsList->selectionModel();

    //	horizontal header
    hv=mw->ui.allSongsList->horizontalHeader();
    hv->setSortIndicator(2,Qt::AscendingOrder);
    hv->setSortIndicatorShown(1);
    hv->setSectionResizeMode(QHeaderView::Stretch);

    //	vertical header
    hv=mw->ui.allSongsList->verticalHeader();
    hv->setDefaultSectionSize(18);
    hv->hide();
    Common::hideColumns(mw->ui.allSongsList);

    //	set up signals
    sm=mw->ui.allSongsList->selectionModel();
    //		capture which cell is active, so we can set contect for drag/drop
    connect(sm, SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(songlistCellSelectionChanged(QItemSelection,QItemSelection)));
    //		capture double click to open detail page
    connect(mw->ui.allSongsList, SIGNAL(clicked(QModelIndex)),
            Context::instance()->getSonglistScreenHandler(),SLOT(openSonglistItem(QModelIndex)));

    ///	SEARCH

    QCompleter* c=mw->ui.searchEdit->completer();
    connect(
        c, SIGNAL(activated(const QModelIndex&)),
        this, SLOT(openItemFromCompleter(const QModelIndex&)));
    connect(
        mw->ui.searchEdit,SIGNAL(returnPressed()),
        Context::instance()->getSonglistScreenHandler(),SLOT(applySonglistFilter()));

    ///	GENRE

//    //	general
//    mw->ui.genreList->setSelectionMode(QAbstractItemView::SingleSelection);
//    mw->ui.genreList->setSortingEnabled(1);
//    mw->ui.genreList->sortByColumn(0,Qt::AscendingOrder);
//    mw->ui.genreList->setSelectionBehavior(QAbstractItemView::SelectRows);
//
//    //	horizontal header
//    hv=mw->ui.genreList->horizontalHeader();
//    hv->setSortIndicator(0,Qt::AscendingOrder);
//    hv->setSortIndicatorShown(1);
//    hv->resizeSection(0,239);
//    hv->show();
//
//    //	vertical header
//    hv=mw->ui.genreList->verticalHeader();
//    hv->setDefaultSectionSize(18);
//    hv->hide();
//    Common::hideColumns(mw->ui.genreList);
//
//    //	set up signals
//    //	1 - change songlist if genre is selected
//    QItemSelectionModel* n=mw->ui.genreList->selectionModel();
//    connect(
//        n, SIGNAL(selectionChanged(const QItemSelection &,const QItemSelection &)),
//        this, SLOT(applyGenreSelection(const QItemSelection &,const QItemSelection &)));
//    //	2 - genre update
//
//    //connect(
//        //g, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
//        //this, SLOT(updateGenre(QModelIndex,QModelIndex)));


    //	Populate schema dropdown
    QStringList schemas=dal->getAvailableSchemas();
    if(schemas.count()>1)
    {
        mw->ui.schemaComboBox->addItems(schemas);
        mw->ui.schemaComboBox->setCurrentIndex(mw->ui.schemaComboBox->findText(dal->getSchemaName()));
        connect(mw->ui.schemaComboBox, SIGNAL(activated(QString)), this, SLOT(changeSchema(QString)));
    }
    else
    {
        mw->ui.schemaComboBox->hide();
        mw->ui.labelSchemaComboBox->hide();
    }


    //	drag & drop revisited.
    QTableView* allSongsList=mw->ui.allSongsList;
    //QTableView* playList=mw->ui.playlistList;
    //QTableView* genreList=mw->ui.genreList;

    allSongsList->setDragEnabled(true);
    //allSongsList->setAcceptDrops(true);
    allSongsList->setDropIndicatorShown(true);

    //playList->setDragEnabled(true);
    //playList->setAcceptDrops(true);
    //playList->viewport()->setAcceptDrops(true);
    //playList->setDropIndicatorShown(true);

//    //genreList->setDragEnabled(true);
//    genreList->setAcceptDrops(true);
//    genreList->viewport()->setAcceptDrops(true);
//    genreList->setDropIndicatorShown(true);

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

    ///	LeftColumnChooser
    mw->ui.leftColumnChooser->setEditTriggers(QAbstractItemView::NoEditTriggers);
    mw->ui.leftColumnChooser->expandAll();
    connect(mw->ui.leftColumnChooser, SIGNAL(clicked(QModelIndex)),
            Context::instance()->getSonglistScreenHandler(), SLOT(openLeftColumnChooserItem(QModelIndex)));

    ///	COMMON
    QTabBar* tb=mw->ui.songlistTab->tabBar();
    tb->hide();

    //mw->ui.tabSongDetailLists->setCurrentIndex(5);

    //Context::instance()->getSonglistScreenHandler()->showSonglist();
    qDebug() << SB_DEBUG_INFO;
    return;
}

void
Controller::configureMenus()
{
    QList<QAction *> list=Context::instance()->getMainWindow()->ui.menuFile->actions();
    QList<QAction *>::iterator it;
    QAction* i;

    for(it=list.begin(); it!=list.end(); ++it)
    {
        i=(*it);
        const QString& itemName=(*it)->objectName();
        if(itemName=="menuOpenDatabase")
        {
            connect(i,SIGNAL(triggered()),this,SLOT(openDatabase()));
        }
        else
        {
            qDebug() << "default:objectName=" << (*it)->objectName();
        }
    }
}

void
Controller::initAttributes()
{
    currentFilter="";
    slP=NULL;
}
