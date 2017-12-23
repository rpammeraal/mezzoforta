#include "SBTabChartDetail.h"

#include "Context.h"
#include "MainWindow.h"
#include "SBTableModel.h"

SBTabChartDetail::SBTabChartDetail(QWidget* parent) : SBTabChooser(parent)
{
}

QTableView*
SBTabChartDetail::subtabID2TableView(int subtabID) const
{
    Q_UNUSED(subtabID);
    const MainWindow* mw=Context::instance()->mainWindow();
    return mw->ui.chartDetailSongList;
}

///	Public slots
void
SBTabChartDetail::playNow(bool enqueueFlag)
{
    SBKey key=this->currentScreenItem().key();
    qDebug() << SB_DEBUG_INFO << key;
    const MainWindow* mw=Context::instance()->mainWindow();
    PlaylistItem selectedPrimary=_getSelectedItem(mw->ui.chartDetailSongList->model(),_lastClickedIndex);

    const QAbstractItemModel* aim=_lastClickedIndex.model();
    QModelIndex secondaryIdx=aim->index(_lastClickedIndex.row(), _lastClickedIndex.column()+2);
    PlaylistItem selectedSecondary=_getSelectedItem(mw->ui.chartDetailSongList->model(),secondaryIdx);
    qDebug() << SB_DEBUG_INFO << selectedPrimary.key << selectedSecondary.key;

    if(selectedSecondary.key.validFlag())
    {
        //	Find song performance with the selected performer, otherwise we fall back on just song.
        SBIDSongPerformancePtr sPtr=SBIDSongPerformance::retrieveSongPerformanceByPerformerID(selectedPrimary.key.itemID(),selectedSecondary.key.itemID());
        if(sPtr)
        {

        }
    }
    if(selectedPrimary.key.validFlag())
    {
        key=selectedPrimary.key;
    }
    if(key.validFlag())
    {
        PlayManager* pmgr=Context::instance()->playManager();
        pmgr?pmgr->playItemNow(key,enqueueFlag):0;
        SBTab::playNow(enqueueFlag);
    }
}


///	Private methods
void
SBTabChartDetail::_init()
{
    SBTab::init();
    if(_initDoneFlag==0)
    {
        MainWindow* mw=Context::instance()->mainWindow();

        _initDoneFlag=1;

        connect(mw->ui.chartDetailSongList->horizontalHeader(), SIGNAL(sectionClicked(int)),
                this, SLOT(sortOrderChanged(int)));

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

        //		3.	Delete item from playlist
//        _deletePlaylistItemAction = new QAction(tr("Delete Item From Playlist "), this);
//        _deletePlaylistItemAction->setStatusTip(tr("Delete Item From Playlist"));
//        connect(_deletePlaylistItemAction, SIGNAL(triggered()),
//                this, SLOT(deletePlaylistItem()));

        //	Tableviews
        QTableView* tv=NULL;

        //		1.	List of songs
        tv=mw->ui.chartDetailSongList;
        connect(tv, SIGNAL(clicked(QModelIndex)),
                this, SLOT(tableViewCellClicked(QModelIndex)));

        tv->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(tv, SIGNAL(customContextMenuRequested(const QPoint&)),
                this, SLOT(showContextMenuView(QPoint)));

        //	Icon
        SBLabel* l=mw->ui.labelChartDetailIcon;
        connect(l, SIGNAL(customContextMenuRequested(QPoint)),
                this, SLOT(showContextMenuLabel(QPoint)));
    }
}

ScreenItem
SBTabChartDetail::_populate(const ScreenItem& si)
{
    _init();
    SBIDChartPtr cPtr=SBIDChart::retrieveChart(si.key());
    SB_RETURN_IF_NULL(cPtr,ScreenItem());

    const MainWindow* mw=Context::instance()->mainWindow();

    ScreenItem currentScreenItem=si;
    currentScreenItem.updateSBIDBase(cPtr->key());
    mw->ui.labelChartDetailIcon->setKey(cPtr->key());

    mw->ui.labelChartDetailChartName->setText(cPtr->chartName());
    mw->ui.labelChartDetailChartDetail->setText(cPtr->chartReleaseDate().toString());

    QTableView* tv=mw->ui.chartDetailSongList;
    SBTableModel* tm=cPtr->tableModelItems();
    QList<bool> dragableColumns;
    dragableColumns.clear();
    dragableColumns << 0 << 0 << 1 << 0 << 1;
    tm->setDragableColumns(dragableColumns);

    populateTableView(tv,tm,0);
    //connect(tm, SIGNAL(assign(const SBIDPtr&,int)),
            //this, SLOT(movePlaylistItem(const SBIDPtr&, int)));

    //	Drag & drop mw->ui.playlistDetailSongList->setAcceptDrops(1);
    mw->ui.chartDetailSongList->setDropIndicatorShown(1);
    mw->ui.chartDetailSongList->viewport()->setAcceptDrops(1);
    mw->ui.chartDetailSongList->setDefaultDropAction(Qt::MoveAction);

    return currentScreenItem;
}
