#include "SBTabChartDetail.h"

#include "Context.h"
#include "MainWindow.h"

SBTabChartDetail::SBTabChartDetail(QWidget* parent) : SBTabChooser(parent)
{
}

QTableView*
SBTabChartDetail::subtabID2TableView(int subtabID) const
{
    Q_UNUSED(subtabID);
    const MainWindow* mw=Context::instance()->getMainWindow();
    return mw->ui.chartDetailSongList;
}

///	Public slots
void
SBTabChartDetail::playNow(bool enqueueFlag)
{
    const SBIDPtr currentPtr=this->currentScreenItem().ptr();
    const MainWindow* mw=Context::instance()->getMainWindow();
    PlaylistItem selected=_getSelectedItem(mw->ui.chartDetailSongList->model(),_lastClickedIndex);
    SBIDPtr ptr;

    qDebug() << SB_DEBUG_INFO;
    if(selected.key.length()==0)
    {
        //	Label clicked
    qDebug() << SB_DEBUG_INFO << currentPtr->itemType() << currentPtr->itemID();
        ptr=SBIDChart::retrieveChart(currentPtr->itemID());
    }
    else
    {
    qDebug() << SB_DEBUG_INFO;
        ptr=SBIDBase::createPtr(selected.key,1);
    }
    if(ptr)
    {
    qDebug() << SB_DEBUG_INFO;
        PlayManager* pmgr=Context::instance()->getPlayManager();
        pmgr?pmgr->playItemNow(ptr,enqueueFlag):0;
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
        MainWindow* mw=Context::instance()->getMainWindow();

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
    SBIDChartPtr cPtr;
    const MainWindow* mw=Context::instance()->getMainWindow();

    //	Get detail
    if(si.ptr() && si.ptr()->itemType()==SBIDBase::sb_type_chart)
    {
        cPtr=std::dynamic_pointer_cast<SBIDChart>(si.ptr());
    }

    if(!cPtr)
    {
        //	Not found
        return ScreenItem();
    }

    ScreenItem currentScreenItem=si;
    currentScreenItem.updateSBIDBase(cPtr);
    mw->ui.labelPlaylistDetailIcon->setPtr(cPtr);

    mw->ui.labelChartDetailChartName->setText(cPtr->chartName());
    mw->ui.labelChartDetailChartDetail->setText(cPtr->chartReleaseDate().toString());

    QTableView* tv=mw->ui.chartDetailSongList;
    SBTableModel* tm=cPtr->items();
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
