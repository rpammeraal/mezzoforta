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
    const SBKey key=this->currentScreenItem().key();
    const MainWindow* mw=Context::instance()->getMainWindow();
    SBTabChooser::PlaylistItem selected=_getSelectedItem(mw->ui.playlistDetailSongList->model(),_lastClickedIndex);
    if(selected.playlistPosition>=0)
    {
        SBIDPlaylistPtr playlistPtr=SBIDPlaylist::retrievePlaylist(key);

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
                playlistPtr->refreshDependents(0,1);
            }
            refreshTabIfCurrent(key);
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
SBTabPlaylistDetail::movePlaylistItem(const SBKey from, int row)
{
    _init();

    //	Determine current playlist
    const SBKey key=currentScreenItem().key();
    SBIDPlaylistPtr pPtr=SBIDPlaylist::retrievePlaylist(key);
    SB_RETURN_VOID_IF_NULL(pPtr);

    SBIDPlaylistDetailPtr pdPtr=SBIDPlaylistDetail::retrievePlaylistDetail(from);
    if(pPtr && pdPtr)
    {
        bool successFlag=pPtr->moveItem(pdPtr,row);
        if(!successFlag)
        {
            pPtr->refreshDependents(1,1);
        }
    }
    refreshTabIfCurrent(key);

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
    SBKey key=this->currentScreenItem().key();
    const MainWindow* mw=Context::instance()->getMainWindow();
    PlaylistItem selected=_getSelectedItem(mw->ui.playlistDetailSongList->model(),_lastClickedIndex);

    if(selected.key.validFlag())
    {
        key=selected.key;
    }

    if(key.validFlag())
    {
        PlayManager* pmgr=Context::instance()->getPlayManager();
        pmgr?pmgr->playItemNow(key,enqueueFlag):0;
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
    SBIDPlaylistPtr playlistPtr=SBIDPlaylist::retrievePlaylist(si.key());
    SB_RETURN_IF_NULL(playlistPtr,ScreenItem());
    const MainWindow* mw=Context::instance()->getMainWindow();

    ScreenItem currentScreenItem=si;
    currentScreenItem.updateSBIDBase(playlistPtr->key());
    mw->ui.labelPlaylistDetailIcon->setKey(playlistPtr->key());

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
    connect(tm, SIGNAL(assign(const SBKey,int)),
            this, SLOT(movePlaylistItem(const SBKey, int)));

    //	Drag & drop mw->ui.playlistDetailSongList->setAcceptDrops(1);
    mw->ui.playlistDetailSongList->setDropIndicatorShown(1);
    mw->ui.playlistDetailSongList->viewport()->setAcceptDrops(1);
    mw->ui.playlistDetailSongList->setDefaultDropAction(Qt::MoveAction);

    return currentScreenItem;
}

