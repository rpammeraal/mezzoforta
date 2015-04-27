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
#include "Controller.h"
#include "MainWindow.h"
#include "DataAccessLayer.h"
#include "DisplayOnlyDelegate.h"
#include "DatabaseSelector.h"
#include "SBModel.h"
#include "SBModelSonglist.h"
#include "SBModelPlaylist.h"
#include "SBModelGenrelist.h"

Controller::Controller(int argc, char *argv[])
{
    SB_UNUSED(argc);
    SB_UNUSED(argv);

    mw=NULL;
    dal=NULL;
    //songListFilter=NULL;

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
/// \brief Controller::applySongListFilter
/// \param filter
///
///
void
Controller::applySongListFilter(const QString &filter)
{
    qDebug() << SB_DEBUG_INFO << "applySongListFilter:start:filter=" << currentFilter;
    //	Capture text returned by completer
    if(filter.length()>0)
    {
        //	do an exact search on what completer returned
        currentFilter=filter;
        doExactSearch=1;
    }
    else
    {
        //	do a search based on keywords
        currentFilter=mw->ui.searchEdit->text();
        doExactSearch=0;
    }
    updateCurrentSongList();
    qDebug() << SB_DEBUG_INFO << "applySongListFilter:end";
}

////
/// \brief Controller::applyPlaylistSelection
/// \param selected
/// \param deselected
///
void
Controller::applyPlaylistSelection(const QItemSelection &selected, const QItemSelection &deselected)
{
    QModelIndex index;
    QModelIndexList items = selected.indexes();

    SB_UNUSED(deselected);

    qDebug() << "playlistSected:start";

    clearSearchFilter();
    clearGenreSelection();

    foreach (index, items)
    {
        //	Since only one item can be selected, return after 1st item
        const int i= index.row();
        const int j= index.column();
        const long playlistID= mw->ui.playlistList->model()->data(index.sibling(i,0)).toInt();
        qDebug() << "playlistSected"
            << ":i=" << i
            << ":j=" << j
            << ":playlistID=" << playlistID
        ;
        selectedPlaylistID=playlistID;
        updateCurrentSongList();
        return;
    }
}

///
/// \brief Controller::applyGenreSelection
/// \param selected
/// \param deselected
///
void
Controller::applyGenreSelection(const QItemSelection &selected, const QItemSelection &deselected)
{
    QModelIndex index;
    QModelIndexList items = selected.indexes();

    qDebug() << "applyGenreSelection(const QItemSelecti";

    clearSearchFilter();
    clearPlaylistSelection();

    foreach (index, items)
    {
        const int i= index.row();
        const int j= index.column();
        const QString& selectedGenre=mw->ui.genreList->model()->data(index.sibling(i,j)).toString();
        selectedGenres.push_back(selectedGenre);
        selectedGenres.removeDuplicates();
    }

    items=deselected.indexes();
    foreach (index, items)
    {
        const int i= index.row();
        const int j= index.column();
        const QString& selectedGenre= mw->ui.genreList->model()->data(index.sibling(i,j)).toString();
        selectedGenres.removeAt(selectedGenres.indexOf(selectedGenre));
    }
    qDebug() << "applyGenreSelection:selectedGenres=" << selectedGenres;

    updateCurrentSongList();
}

void
Controller::changeSchema(const QString& newSchema)
{
    qDebug() << "Controller:changeSchema:new schema=" << newSchema;
    if(dal->setSchema(newSchema))
    {
        //	refresh all views
        resetAllFiltersAndSelections();
    }
}

//	Data Updates
void
Controller::updateGenre(QModelIndex i, QModelIndex j)
{
    static int updateInProgress=0;
    SB_UNUSED(j);

    if(updateInProgress==0)
    {
        //	Poor man's semaphore
        qDebug() << "Controller::updateGenre:start";
        updateInProgress=1;
            QString newGenre=dal->updateGenre(i);
            mw->ui.genreList->update();
            clearGenreSelection();
            updateCurrentSongList();

        updateInProgress=0;
    }
}

//PROTECTED:

void
Controller::keyPressEvent(QKeyEvent *event)
{
    if(event->key()==0x1000000)
    {
        if(currentFilter.length()>0)
        {
            qDebug() << SB_DEBUG_INFO;
            clearSearchFilter();
            updateCurrentSongList();
        }
        else
        {
            qDebug() << SB_DEBUG_INFO;
            //	Catch escape key, blank searchEdit, playlist, genres
            resetAllFiltersAndSelections();
            updateCurrentSongList();
        }
    }
    else if(event->key()==Qt::Key_Return || event->key()==Qt::Key_Enter)
    {
        //	if genre has a selection, modify
        const int st=getSelectedTab();
        if(st==SB_TAB_PLAYLIST)
        {
            //	playlist tab
            QItemSelectionModel* s=mw->ui.playlistList->selectionModel();

            if(s->hasSelection()==1)
            {
                mw->ui.playlistList->edit(s->currentIndex());
            }
        }
        else if(st==SB_TAB_GENRE)
        {
            //	genre tab
            if(selectedGenres.count()==1)
            {
                QItemSelectionModel* s=mw->ui.genreList->selectionModel();
                mw->ui.genreList->edit(s->currentIndex());
            }
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

///
/// \brief Controller::updateCurrentSongList
///
/// Refreshes the song list based on user made selections on the UI.
void
Controller::updateCurrentSongList()
{
    qDebug() << SB_DEBUG_INFO
        << ":selectedPlaylistID=" << selectedPlaylistID
        << ":selectedGenres=" << selectedGenres
        << ":currentFilter=" << currentFilter
    ;

    //	If currentFilter is not empty, perform search.
    //	This way, selected genre or playlist will be honored.
    if(currentFilter.length()>0)
    {
        //	if search is to be performed, use SortFilterProxy model
        if(doExactSearch==1)
        {
            slP->setFilterFixedString(currentFilter);
        }
        else
        {
            //	search for whole words.
            QString s;
            s=currentFilter.replace(" ","\\b)(?=[^\\r\\n]*\\b");
            s="^(?=[^\\r\\n]*\\b"+s+"\\b)[^\\r\\n]*$";
            qDebug() << "regexp=" << s;
            QRegExp rx(s,Qt::CaseInsensitive);
            slP->setFilterRegExp(rx);
        }
    }
    else
    {
        //	What tab is selected (playlist, genres)
        const int st=getSelectedTab();

        //	If playlist or genre is selected, do requery of data.
        if(st==SB_TAB_PLAYLIST && selectedPlaylistID!=-1)
        {
            //	playlist selected
            clearGenreSelection();
            clearSearchFilter();
            sm->applyFilter(selectedPlaylistID,selectedGenres);//,currentFilter,doExactSearch);

        }
        else if(st==SB_TAB_GENRE && selectedGenres.count()>0)
        {
            //	one or more genres selected
            clearPlaylistSelection();
            clearSearchFilter();
            sm->applyFilter(selectedPlaylistID,selectedGenres);//,currentFilter,doExactSearch);
        }
        else
        {
            //	refresh all
            sm->applyFilter(selectedPlaylistID,selectedGenres);//,currentFilter,doExactSearch);
        }
    }
    return;
}

int
Controller::getSelectedTab()
{
    const int i=mw->ui.playlistGenreTab->currentIndex();
    return (i==0 ? SB_TAB_PLAYLIST : SB_TAB_GENRE);
}


void
Controller::resetAllFiltersAndSelections()
{
    clearGenreSelection();
    clearPlaylistSelection();
    clearSearchFilter();
}

void
Controller::clearPlaylistSelection()
{
    mw->ui.playlistList->clearSelection();
    selectedPlaylistID=-1;
}

void
Controller::clearGenreSelection()
{
    mw->ui.genreList->clearSelection();
    selectedGenres.clear();
}

void
Controller::clearSearchFilter()
{
    currentFilter="";
    mw->ui.searchEdit->setText(tr(""));
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
    qDebug() << "openMainWindow:start";

    //	Instantiate DatabaseSelector, check if database could be opened.
    DatabaseSelector* ds=new DatabaseSelector(startup);

    if(startup && ds->databaseOpen()==0)
    {
        //	no database opened upin startup.
        return 0;
    }
    qDebug() << "openMainWindow:databaseChanged=" << ds->databaseChanged();

    if(ds->databaseChanged() || startup)
    {
        MainWindow* oldMW=mw;

        QProgressDialog p("Reading data...",QString(),0,8,oldMW);
        p.setWindowModality(Qt::WindowModal);

        p.setValue(0);

        initAttributes();
        if(dal)
        {
            delete dal;
            dal=NULL;
        }

        p.setValue(1);
        const QString connectionName=ds->getConnectionName();
        dal=ds->getDataAccessLayer();

        p.setValue(2);

        p.setValue(3);
        mw=new MainWindow(this);

        p.setValue(4);
        resetAllFiltersAndSelections();

        p.setValue(5);
        setupModels();

        p.setValue(6);
        setupUI();

        configureMenus();

        mw->setWindowTitle(mw->windowTitle() + " - " + ds->databaseName() + " ("+dal->getDriverName()+")");

        qDebug() << SB_DEBUG_INFO;

        p.setValue(7);

        qDebug() << SB_DEBUG_INFO;

        mw->show();
        mw->resize(1200,600);

        qDebug() << SB_DEBUG_INFO;

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
    //	songlist
    sm=dal->getAllSongs();
    slP=new QSortFilterProxyModel();
    slP->setSourceModel(sm);
    mw->ui.songList->setModel(slP);

    //	completer
    QCompleter* completer=new QCompleter(mw);
    completer->setModel(dal->getCompleterModel());
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setModelSorting(QCompleter::CaseSensitivelySortedModel);
    completer->setFilterMode(Qt::MatchContains);
    mw->ui.searchEdit->setCompleter(completer);

    //	playlist
    plm=dal->getAllPlaylists();
    pllP=new QSortFilterProxyModel();
    pllP->setSourceModel(plm);
    mw->ui.playlistList->setModel(pllP);

    //	genre
    gm=dal->getAllGenres();
    glP=new QSortFilterProxyModel();
    glP->setSourceModel(gm);
    mw->ui.genreList->setModel(glP);
}

void
Controller::setupUI()
{
    //	Frequently used pointers
    QHeaderView* hv;
    QItemSelectionModel* sm;

    qDebug() << "Controller:setupUI:start";

    ///	SONGLIST

    //	general
    mw->ui.songList->setSortingEnabled(1);
    mw->ui.songList->sortByColumn(0,Qt::AscendingOrder);
    mw->ui.songList->setSelectionMode(QAbstractItemView::SingleSelection);
    mw->ui.songList->setSelectionBehavior(QAbstractItemView::SelectItems);
    mw->ui.songList->setFocusPolicy(Qt::StrongFocus);
    sm=mw->ui.songList->selectionModel();

    //	horizontal header
    hv=mw->ui.songList->horizontalHeader();
    hv->setSortIndicator(2,Qt::AscendingOrder);
    hv->setSortIndicatorShown(1);

    //	vertical header
    hv=mw->ui.songList->verticalHeader();
    hv->setDefaultSectionSize(18);
    hv->hide();
    mw->hideColumns(mw->ui.songList);


    ///	COMPLETER

    QCompleter* c=mw->ui.searchEdit->completer();
    connect(
        c, SIGNAL(activated(QString)),
        this, SLOT(applySongListFilter(QString)));
    connect(
        mw->ui.searchEdit,SIGNAL(returnPressed()),
        this, SLOT(applySongListFilter()));



    ///	PLAYLIST

    //	general
    mw->ui.playlistList->setSelectionMode(QAbstractItemView::SingleSelection);
    mw->ui.playlistList->hideColumn(0);
    mw->ui.playlistList->setSortingEnabled(1);
    mw->ui.playlistList->sortByColumn(1,Qt::AscendingOrder);
    mw->ui.playlistList->setSelectionBehavior(QAbstractItemView::SelectRows);

    //	horizontal header
    hv=mw->ui.playlistList->horizontalHeader();
    hv->setSortIndicator(1,Qt::AscendingOrder);
    hv->setSortIndicatorShown(1);
    hv->resizeSection(1,164);
    hv->resizeSection(2,75);
    hv->show();

    //	vertical header
    hv=mw->ui.playlistList->verticalHeader();
    hv->setDefaultSectionSize(18);
    hv->hide();

    //	set duration r/o
    DisplayOnlyDelegate* dod=new DisplayOnlyDelegate(2);	//	duration ro
    mw->ui.playlistList->setItemDelegate(dod);

    //	set up signals
    //	1 - change songlist if playlist is selected
    sm=mw->ui.playlistList->selectionModel();
    connect(
        sm, SIGNAL(selectionChanged(const QItemSelection &,const QItemSelection &)),
        this, SLOT(applyPlaylistSelection(const QItemSelection &,const QItemSelection &)));


    ///	GENRE

    //	general
    mw->ui.genreList->setSelectionMode(QAbstractItemView::SingleSelection);
    mw->ui.genreList->setSortingEnabled(1);
    mw->ui.genreList->sortByColumn(0,Qt::AscendingOrder);
    mw->ui.genreList->setSelectionBehavior(QAbstractItemView::SelectRows);

    //	horizontal header
    hv=mw->ui.genreList->horizontalHeader();
    hv->setSortIndicator(0,Qt::AscendingOrder);
    hv->setSortIndicatorShown(1);
    hv->resizeSection(0,239);
    hv->show();

    //	vertical header
    hv=mw->ui.genreList->verticalHeader();
    hv->setDefaultSectionSize(18);
    hv->hide();
    mw->hideColumns(mw->ui.genreList);

    //	set up signals
    //	1 - change songlist if genre is selected
    QItemSelectionModel* n=mw->ui.genreList->selectionModel();
    connect(
        n, SIGNAL(selectionChanged(const QItemSelection &,const QItemSelection &)),
        this, SLOT(applyGenreSelection(const QItemSelection &,const QItemSelection &)));
    //	2 - genre update

    //connect(
        //g, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
        //this, SLOT(updateGenre(QModelIndex,QModelIndex)));


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

    mw->ui.playlistGenreTab->setCurrentIndex(0);

    //	drag & drop revisited.
    QTableView* songList=mw->ui.songList;
    QTableView* playList=mw->ui.playlistList;
    QTableView* genreList=mw->ui.genreList;

    songList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    songList->setDragEnabled(true);
    //songList->setAcceptDrops(true);
    songList->setDropIndicatorShown(true);

    playList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    //playList->setDragEnabled(true);
    playList->setAcceptDrops(true);
    playList->setDropIndicatorShown(true);

    genreList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    //genreList->setDragEnabled(true);
    genreList->setAcceptDrops(true);
    genreList->setDropIndicatorShown(true);

    return;
}

void
Controller::configureMenus()
{
    QList<QAction *> list=mw->ui.menuFile->actions();
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
    selectedPlaylistID=-1;
    selectedGenres.clear();
    currentFilter="";
    slP=NULL;
    pllP=NULL;
    glP=NULL;

    sm=NULL;
    plm=NULL;
    gm=NULL;
}
