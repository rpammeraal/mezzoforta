//#include <QAbstractItemModel>
//#include <QMenu>

#include "SBTabPlaylistDetail.h"

#include "CacheManager.h"
#include "Context.h"
#include "Controller.h"
#include "MainWindow.h"
#include "SBSqlQueryModel.h"
#include "SBSortFilterProxyTableModel.h"
#include "SBTableModel.h"

SBTabPlaylistDetail::SBTabPlaylistDetail(QWidget* parent) : SBTabChooser(parent)
{
}

QTableView*
SBTabPlaylistDetail::subtabID2TableView(int subtabID) const
{
    Q_UNUSED(subtabID);
    const MainWindow* mw=Context::instance()->getMainWindow();
    return mw->ui.playlistDetailSongList;
}

///	Public slots
void
SBTabPlaylistDetail::deletePlaylistItem()
{
    _init();
    SBIDPtr ptr=this->currentScreenItem().ptr();
    const MainWindow* mw=Context::instance()->getMainWindow();
    SBTabChooser::PlaylistItem selected=_getSelectedItem(mw->ui.playlistDetailSongList->model(),_lastClickedIndex);
    if(ptr && ptr->itemType()==SBIDBase::sb_type_playlist && selected.playlistPosition>=0)
    {
        SBIDPlaylistPtr playlistPtr=SBIDPlaylist::retrievePlaylist(ptr->itemID());
        if(playlistPtr)
        {
            bool successFlag=playlistPtr->removePlaylistItem(selected.playlistPosition);
            QString updateText;
            if(successFlag)
            {
                updateText=QString("Removed %1%2%3 from %5 %1%4%3.")
                    .arg(QChar(96))            //	1
                    .arg(selected.text)        //	2
                    .arg(QChar(180))           //	3
                    .arg(playlistPtr->text())  //	4
                    .arg(playlistPtr->type()); //	5
            }
            else
            {
                updateText=QString("Unable to remove %1%2%3 from %5 %1%4%3.")
                    .arg(QChar(96))            //	1
                    .arg(selected.text)        //	2
                    .arg(QChar(180))           //	3
                    .arg(playlistPtr->text())  //	4
                    .arg(playlistPtr->type()); //	5
                CacheManager* cm=Context::instance()->cacheManager();
                CachePlaylistMgr* pmgr=cm->playlistMgr();
                playlistPtr->refreshDependents(1,1);
                playlistPtr=pmgr->retrieve(SBIDPlaylist::createKey(playlistPtr->playlistID()),CachePlaylistMgr::open_flag_refresh);
                ptr=playlistPtr;
            }
            refreshTabIfCurrent(ptr);
            Context::instance()->getController()->updateStatusBarText(updateText);

            //	Repopulate the current screen
            _populate(currentScreenItem());
        }
    }
    if(_menu)
    {
        _menu->hide();
    }
}

void
SBTabPlaylistDetail::movePlaylistItem(const SBIDPtr& fromPtr, int row)
{
    _init();

    //	Determine current playlist
    SBIDPtr ptr=this->currentScreenItem().ptr();

    if(ptr && ptr->itemType()==SBIDBase::sb_type_playlist)
    {
        SBIDPlaylistPtr pPtr=SBIDPlaylist::retrievePlaylist(ptr->itemID());
        SBIDPlaylistDetailPtr pdPtr=SBIDPlaylistDetail::retrievePlaylistDetail(fromPtr->itemID());
        if(pPtr && pdPtr)
        {
            bool successFlag=pPtr->moveItem(pdPtr,row);
            if(!successFlag)
            {
                pPtr->refreshDependents(1,1);
            }
        }
    }
    refreshTabIfCurrent(ptr);

    const MainWindow* mw=Context::instance()->getMainWindow();
    QTableView* tv=mw->ui.playlistDetailSongList;
    QAbstractItemModel* m=tv->model();
    SB_DEBUG_IF_NULL(m);

    QModelIndex idx=m->index(row,0);
    tv->scrollTo(idx);
}

void
SBTabPlaylistDetail::playNow(bool enqueueFlag)
{
    const SBIDPtr currentPtr=this->currentScreenItem().ptr();
    const MainWindow* mw=Context::instance()->getMainWindow();
    PlaylistItem selected=_getSelectedItem(mw->ui.playlistDetailSongList->model(),_lastClickedIndex);
    SBIDPtr ptr;

    if(selected.key.length()==0)
    {
        //	Label clicked
        ptr=SBIDPlaylist::retrievePlaylist(currentPtr->itemID());
    }
    else
    {
        ptr=SBIDBase::createPtr(selected.key,1);
    }
    if(ptr)
    {
        PlayManager* pmgr=Context::instance()->getPlayManager();
        pmgr?pmgr->playItemNow(ptr,enqueueFlag):0;
        SBTab::playNow(enqueueFlag);
    }
}

///	Private methods
void
SBTabPlaylistDetail::_init()
{
    SBTab::init();
    if(_initDoneFlag==0)
    {
        MainWindow* mw=Context::instance()->getMainWindow();

        _initDoneFlag=1;

        connect(mw->ui.playlistDetailSongList->horizontalHeader(), SIGNAL(sectionClicked(int)),
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
        _deleteItemAction = new QAction(tr("Delete Item From Playlist "), this);
        _deleteItemAction->setStatusTip(tr("Delete Item From Playlist"));
        connect(_deleteItemAction, SIGNAL(triggered()),
                this, SLOT(deletePlaylistItem()));

        //	Tableviews
        QTableView* tv=NULL;

        //		1.	List of songs
        tv=mw->ui.playlistDetailSongList;
        connect(tv, SIGNAL(clicked(QModelIndex)),
                this, SLOT(tableViewCellClicked(QModelIndex)));

        tv->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(tv, SIGNAL(customContextMenuRequested(const QPoint&)),
                this, SLOT(showContextMenuView(QPoint)));

        //	Icon
        SBLabel* l=mw->ui.labelPlaylistDetailIcon;
        connect(l, SIGNAL(customContextMenuRequested(QPoint)),
                this, SLOT(showContextMenuLabel(QPoint)));
    }
}

ScreenItem
SBTabPlaylistDetail::_populate(const ScreenItem& si)
{
    _init();
    SBIDPlaylistPtr playlistPtr;
    const MainWindow* mw=Context::instance()->getMainWindow();

    //	Get detail
    if(si.ptr() && si.ptr()->itemType()==SBIDBase::sb_type_playlist)
    {
        playlistPtr=SBIDPlaylist::retrievePlaylist(si.ptr()->itemID());
    }

    if(!playlistPtr)
    {
        //	Not found
        return ScreenItem();
    }

    ScreenItem currentScreenItem=si;
    currentScreenItem.updateSBIDBase(playlistPtr);
    mw->ui.labelPlaylistDetailIcon->setPtr(playlistPtr);

    mw->ui.labelPlaylistDetailPlaylistName->setText(playlistPtr->playlistName());
    const QString detail=QString("%1 items ").arg(playlistPtr->numItems())+QChar(8226)+QString(" %2").arg(playlistPtr->duration().toString());
    mw->ui.labelPlaylistDetailPlaylistDetail->setText(detail);

    QTableView* tv=mw->ui.playlistDetailSongList;
    SBTableModel* tm=playlistPtr->tableModelItems();
    QList<bool> dragableColumns;
    dragableColumns.clear();
    dragableColumns << 0 << 0 << 1;
    tm->setDragableColumns(dragableColumns);
    populateTableView(tv,tm,0);
    connect(tm, SIGNAL(assign(const SBIDPtr&,int)),
            this, SLOT(movePlaylistItem(const SBIDPtr&, int)));

    //	Drag & drop mw->ui.playlistDetailSongList->setAcceptDrops(1);
    mw->ui.playlistDetailSongList->setDropIndicatorShown(1);
    mw->ui.playlistDetailSongList->viewport()->setAcceptDrops(1);
    mw->ui.playlistDetailSongList->setDefaultDropAction(Qt::MoveAction);

    return currentScreenItem;
}

