#include "SBTabSongsAll.h"

#include "Context.h"
#include "MainWindow.h"
#include "SBSqlQueryModel.h"

SBTabSongsAll::SBTabSongsAll(QWidget* parent) : SBTab(parent,0)
{
}

bool
SBTabSongsAll::handleEscapeKey()
{
    return 1;
}

void
SBTabSongsAll::preload()
{
    _init();
    //	Allows some data models to be refreshed
    MainWindow* mw=Context::instance()->getMainWindow();
    QSortFilterProxyModel* slP;

    //	Songlist
    SBSqlQueryModel* sm=SBIDSong::retrieveAllSongs();
    QList<bool> dragableColumns;
    dragableColumns.clear();
    //	dragableColumns << 0 << 0 << 0 << 1 << 0 << 0 << 1 << 0 << 0 << 1;
    dragableColumns << 0 << 0 << 1 << 0 << 1 << 0 << 1;
    sm->setDragableColumns(dragableColumns);

    slP=new QSortFilterProxyModel();
    slP->setSourceModel(sm);
    mw->ui.allSongsList->setModel(slP);

    mw->ui.allSongsList->setSortingEnabled(1);
    mw->ui.allSongsList->sortByColumn(2,Qt::AscendingOrder);
    mw->ui.allSongsList->setSelectionMode(QAbstractItemView::SingleSelection);
    mw->ui.allSongsList->setSelectionBehavior(QAbstractItemView::SelectItems);
    mw->ui.allSongsList->setFocusPolicy(Qt::StrongFocus);
    mw->ui.allSongsList->selectionModel();

    QHeaderView* hv;

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

    //	drag & drop revisited.
    QTableView* allSongsList=mw->ui.allSongsList;

    allSongsList->setDragEnabled(true);
    allSongsList->setDropIndicatorShown(true);
}

///	Public slots:
void
SBTabSongsAll::playNow(bool enqueueFlag)
{
    MainWindow* mw=Context::instance()->getMainWindow();
    QTableView* tv=mw->ui.allSongsList;

    QSortFilterProxyModel* pm=dynamic_cast<QSortFilterProxyModel *>(tv->model()); SB_DEBUG_IF_NULL(pm);
    SBSqlQueryModel *sm=dynamic_cast<SBSqlQueryModel* >(pm->sourceModel()); SB_DEBUG_IF_NULL(sm);
    SBKey key=sm->determineKey(_lastClickedIndex);
    PlayManager* pmgr=Context::instance()->getPlayManager();
    pmgr?pmgr->playItemNow(key,enqueueFlag):0;
    SBTab::playNow(enqueueFlag);
}

void
SBTabSongsAll::schemaChanged()
{
    this->preload();
}

void
SBTabSongsAll::showContextMenuLabel(const QPoint &p)
{
    if(_allowPopup(p)==0)
    {
        return;
    }

    const SBIDPtr ptr=SBIDBase::createPtr(currentScreenItem().key());
    SB_RETURN_VOID_IF_NULL(ptr);

    _lastClickedIndex=QModelIndex();

    _menu=new QMenu(NULL);
    _playNowAction->setText(QString("Play '%1' Now").arg(ptr->text()));
    _enqueueAction->setText(QString("Enqueue '%1'").arg(ptr->text()));

    _menu->addAction(_playNowAction);
    _menu->addAction(_enqueueAction);
    _menu->exec(p);
    _recordLastPopup(p);
}

void
SBTabSongsAll::showContextMenuView(const QPoint &p)
{
    const MainWindow* mw=Context::instance()->getMainWindow();
    QTableView* tv=mw->ui.allSongsList;

    QModelIndex idx=tv->indexAt(p);
    QSortFilterProxyModel* pm=dynamic_cast<QSortFilterProxyModel *>(tv->model()); SB_DEBUG_IF_NULL(pm);
    SBSqlQueryModel *sm=dynamic_cast<SBSqlQueryModel* >(pm->sourceModel()); SB_DEBUG_IF_NULL(sm);
    QModelIndex ids=pm->mapToSource(idx);
    SBKey key=sm->determineKey(ids);
    SBIDPtr ptr=SBIDBase::createPtr(key);
    SB_RETURN_VOID_IF_NULL(ptr);

    _lastClickedIndex=ids;

    QPoint gp = mw->ui.currentPlaylistDetailSongList->mapToGlobal(p);

    _menu=new QMenu(NULL);

    _playNowAction->setText(QString("Play '%1' Now").arg(ptr->text()));
    _enqueueAction->setText(QString("Enqueue '%1'").arg(ptr->text()));

    _menu->addAction(_playNowAction);
    _menu->addAction(_enqueueAction);
    _menu->exec(gp);
}

///	Private methods
void
SBTabSongsAll::_init()
{
    if(_initDoneFlag==0)
    {
        MainWindow* mw=Context::instance()->getMainWindow();
        _initDoneFlag=1;


        //	Menu actions
        //		1.	Play Now
        _playNowAction = new QAction(tr("Play Now"), this);
        _playNowAction->setStatusTip(tr("Play Now"));
        connect(_playNowAction, SIGNAL(triggered()),
                this, SLOT(playNow()));

        //		2.	Enqueue
        _enqueueAction = new QAction(tr("Enqueue"), this);
        _enqueueAction->setStatusTip(tr("Enqueue"));
        connect(_enqueueAction, SIGNAL(triggered()),
                this, SLOT(enqueue()));

        //	Tableviews
        QTableView* tv=NULL;

        //		1.	All
        tv=mw->ui.allSongsList;
        connect(tv->horizontalHeader(),SIGNAL(sectionClicked(int)),
                this, SLOT(sortOrderChanged(int)));
        connect(tv, SIGNAL(clicked(QModelIndex)),
                this, SLOT(tableViewCellClicked(QModelIndex)));
        tv->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(tv, SIGNAL(customContextMenuRequested(const QPoint&)),
                this, SLOT(showContextMenuView(QPoint)));

        //	Schema changes
        connect(Context::instance()->getDataAccessLayer(),SIGNAL(schemaChanged()),
                this, SLOT(schemaChanged()));
    }
}

ScreenItem
SBTabSongsAll::_populate(const ScreenItem& si)
{
    return si;
}
