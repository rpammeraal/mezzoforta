#include <QDebug>
#include <QKeyEvent>
#include <QScrollBar>
#include <QCompleter>

#include "Common.h"
#include "Controller.h"
#include "MainWindow.h"
#include "DataAccessLayer.h"
#include "DisplayOnlyDelegate.h"

Controller::Controller(MainWindow *nmw,DataAccessLayer* ndal) : QObject(nmw)
{
    qDebug() << "Controller:ctor";
    mw=nmw;
    dal=ndal;
    doExactSearch=0;

    resetAllFiltersAndSelections();
    populateUI();
}

Controller::~Controller()
{

}

//SLOTS:

///
/// \brief Controller::applySongListFilter
/// \param filter
///
///
void
Controller::applySongListFilter(const QString &filter)
{
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
    qDebug() << "applySongListFilter(2)::filter=" << currentFilter;
    updateCurrentSongList();
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
Controller::changeCurrentTab(const int index)
{
    SB_UNUSED(index);
    //	NOOP
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
            dal->updateGenre(i);
            mw->ui.genreList->update();
            clearGenreSelection();
            configGenreData();	//	may have duplicates now, need to completely reupdate
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
        //	Catch escape key, blank searchEdit, playlist, genres
        resetAllFiltersAndSelections();
        updateCurrentSongList();
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

void
Controller::updateCurrentSongList()
{
    //	What tab is selected (playlist, genres)
    const int st=getSelectedTab();

    if(st==SB_TAB_PLAYLIST && selectedPlaylistID!=-1)
    {
        //	playlist selected
        clearGenreSelection();
    }
    else if(st==SB_TAB_GENRE && selectedGenres.count()>0)
    {
        //	one or more genres selected
        clearPlaylistSelection();
    }
    else
    {
        //	search on complete song list
        clearGenreSelection();
        clearPlaylistSelection();
    }

    //	go get stuff
    QSqlQueryModel* songListSource=NULL;
    songListSource=dal->getSonglist(selectedPlaylistID,selectedGenres,currentFilter,doExactSearch);
    songListFilter->setSourceModel(songListSource);
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
}

void
Controller::populateUI()
{
    //	Frequently used pointers
    QHeaderView* hv;

    qDebug() << "createLayout:start";

    //	access data source
    QSqlQueryModel* songListSource=dal->getSonglist(selectedPlaylistID,selectedGenres,currentFilter,doExactSearch);

    qDebug() << "createLayout:set up search & sort";

    //	set up search & sort
    //SBSortFilterProxyModel* songListFilter=new SBSortFilterProxyModel(this);
    songListFilter=new QSortFilterProxyModel(this);

    //	connect data source -> proxy (filter et al)
    songListFilter->setSourceModel(songListSource);
    songListFilter->setFilterCaseSensitivity(Qt::CaseInsensitive);
    songListFilter->setDynamicSortFilter(1);
    songListFilter->setFilterKeyColumn(3);

    //	filter using QT
    //connect(this->searchEdit, SIGNAL(textChanged(QString)), songListFilter, SLOT(setFilterFixedString(QString)));
    //	filter using database
    //connect(mw->ui.searchEdit, SIGNAL(textChanged(QString)), this, SLOT(applySongListFilter(QString)));

    //	connect proxy -> table view
    mw->ui.songList->setModel(songListFilter);

    //	set sorting
    mw->ui.songList->setSortingEnabled(1);
    mw->ui.songList->sortByColumn(0,Qt::AscendingOrder);

    //	hide row number, index column
    mw->ui.songList->verticalHeader()->hide();
    mw->hideColumns(mw->ui.songList);

    //	completer
    QCompleter* completer=new QCompleter(mw);
    completer->setModel(dal->getCompleterModel());
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    //completer->setCompletionRole(Qt::DisplayRole);
    completer->setModelSorting(QCompleter::CaseSensitivelySortedModel);
    completer->setFilterMode(Qt::MatchContains);
    mw->ui.searchEdit->setCompleter(completer);
    connect(
        completer, SIGNAL(activated(QString)),
        this, SLOT(applySongListFilter(QString)));
    connect(
        mw->ui.searchEdit,SIGNAL(returnPressed()),
        this, SLOT(applySongListFilter()));

    //	Drag/drop
    mw->ui.songList->setDragDropMode(QAbstractItemView::DragOnly);
    mw->ui.songList->setSelectionMode(QAbstractItemView::SingleSelection);
    mw->ui.songList->setDragEnabled(1);
    mw->ui.songList->setAcceptDrops(true);
    //mw->ui.songList->viewport()->setAcceptDrops(true);
    mw->ui.songList->setDropIndicatorShown(true);


    //	playlist
    //QSqlQueryModel* playlistSource=dal->getAllPlaylists();
    QSqlTableModel* playlistSource=dal->getAllPlaylists();
    mw->ui.playlistList->setModel(playlistSource);
    mw->ui.playlistList->setSelectionMode(QAbstractItemView::SingleSelection);
    mw->ui.playlistList->hideColumn(0);
    mw->ui.playlistList->hideColumn(2);
    mw->ui.playlistList->hideColumn(3);
    mw->ui.playlistList->hideColumn(5);
    mw->ui.playlistList->hideColumn(6);
    mw->ui.playlistList->setSortingEnabled(1);
    mw->ui.playlistList->setSelectionBehavior(QAbstractItemView::SelectRows);

    hv=mw->ui.playlistList->horizontalHeader();
    hv->setSortIndicator(1,Qt::AscendingOrder);
    hv->setSortIndicatorShown(1);
    hv->resizeSection(1,179);
    hv->resizeSection(4,75);
    hv->show();

    DisplayOnlyDelegate* dod=new DisplayOnlyDelegate(4);
    mw->ui.playlistList->setItemDelegate(dod);

    //	Drag/drop
    mw->ui.playlistList->setDragDropMode(QAbstractItemView::InternalMove);
    mw->ui.playlistList->setSelectionMode(QAbstractItemView::SingleSelection);
    mw->ui.playlistList->setDragEnabled(1);
    mw->ui.playlistList->setAcceptDrops(true);
    mw->ui.playlistList->setDropIndicatorShown(true);

    QItemSelectionModel* m=mw->ui.playlistList->selectionModel();
    connect(m, SIGNAL(selectionChanged(const QItemSelection &,const QItemSelection &)), this, SLOT(applyPlaylistSelection(const QItemSelection &,const QItemSelection &)));

    //	set rowHeight
    hv=mw->ui.playlistList->verticalHeader();
    hv->setDefaultSectionSize(18);
    hv->hide();

    //	Genre
    QStandardItemModel* g=configGenreData();

    connect(g, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(updateGenre(QModelIndex,QModelIndex)));
    QItemSelectionModel* n=mw->ui.genreList->selectionModel();
    connect(
        n, SIGNAL(selectionChanged(const QItemSelection &,const QItemSelection &)),
        this, SLOT(applyGenreSelection(const QItemSelection &,const QItemSelection &)));

    //	Connect tab to slot
    connect(
        mw->ui.playlistGenreTab, SIGNAL(currentChanged(int)),
        this, SLOT(changeCurrentTab(int)));

    return;
}

int
Controller::getSelectedTab()
{
    const int i=mw->ui.playlistGenreTab->currentIndex();
    return (i==0 ? SB_TAB_PLAYLIST : SB_TAB_GENRE);
}

QStandardItemModel*
Controller::configGenreData()
{
    QHeaderView* hv;

    QStandardItemModel *g=dal->getGenres();
    mw->ui.genreList->setModel(g);
    mw->ui.genreList->hideColumn(1);
    mw->ui.genreList->setSortingEnabled(1);

    hv=mw->ui.genreList->verticalHeader();
    hv->setDefaultSectionSize(18);
    hv->hide();

    hv=mw->ui.genreList->horizontalHeader();
    hv->setSortIndicator(0,Qt::AscendingOrder);
    hv->setSortIndicatorShown(1);
    hv->resizeSection(0,260);
    hv->show();

    return g;
}
