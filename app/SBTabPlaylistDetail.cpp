//#include <QAbstractItemModel>
//#include <QMenu>

#include "SBTabPlaylistDetail.h"

#include "Context.h"
#include "Controller.h"
#include "MainWindow.h"
#include "SBSqlQueryModel.h"
#include "SBSortFilterProxyTableModel.h"

SBTabPlaylistDetail::SBTabPlaylistDetail(QWidget* parent) : SBTab(parent,0)
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
    /*
    qDebug() << SB_DEBUG_INFO;
    _init();
    SBIDPtr ptr=this->currentScreenItem().ptr();
    PlaylistItem selected=_getSelectedItem(_lastClickedIndex);
    if(ptr && ptr->itemType()==SBIDBase::sb_type_playlist && selected.itemPtr)
    {
        SBIDPlaylistPtr playlistPtr=std::dynamic_pointer_cast<SBIDPlaylist>(ptr);
        if(playlistPtr)
        {
            playlistPtr->deletePlaylistItem(selected.itemType,selected.playlistPosition);
            refreshTabIfCurrent(*ptr);
            QString updateText=QString("Removed %5 %1%2%3 from %6 %1%4%3.")
                .arg(QChar(96))            //	1
                .arg(selected.text)        //	2
                .arg(QChar(180))           //	3
                .arg(playlistPtr->text())  //	4
                .arg(selected.itemType)    //	5
                .arg(playlistPtr->type()); //	6
            Context::instance()->getController()->updateStatusBarText(updateText);

            //	Repopulate the current screen
            _populate(currentScreenItem());
        }
    }
    if(_menu)
    {
        _menu->hide();
    }
    */
}

void
SBTabPlaylistDetail::movePlaylistItem(const SBIDPtr& fromIDPtr, int row)
{
    _init();
    //	Determine current playlist
    SBIDPtr ptr=this->currentScreenItem().ptr();

    if(ptr && ptr->itemType()==SBIDBase::sb_type_playlist)
    {
        SBIDPlaylistPtr playlistPtr;

        playlistPtr=SBIDPlaylist::retrievePlaylist(ptr->itemID());
        if(playlistPtr)
        {
            playlistPtr->reorderItem(fromIDPtr,row);
        }
    }
    refreshTabIfCurrent(*ptr);

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
    PlaylistItem selected=_getSelectedItem(_lastClickedIndex);
    SBIDPtr ptr;

    if(selected.key.length()==0)
    {
        //	Label clicked
        ptr=SBIDPlaylist::retrievePlaylist(currentPtr->itemID());
    }
    else
    {
        ptr=SBIDBase::createPtr(selected.key);
    }
    if(ptr)
    {
        PlayManager* pmgr=Context::instance()->getPlayManager();
        pmgr?pmgr->playItemNow(ptr,enqueueFlag):0;
        SBTab::playNow(enqueueFlag);
    }
}

void
SBTabPlaylistDetail::showContextMenuLabel(const QPoint &p)
{
    if(_allowPopup(p)==0)
    {
        return;
    }

    const SBIDPtr ptr=this->currentScreenItem().ptr();
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
SBTabPlaylistDetail::showContextMenuView(const QPoint& p)
{
    if(_allowPopup(p)==0)
    {
        return;
    }

    _init();
    const MainWindow* mw=Context::instance()->getMainWindow();
    QModelIndex idx=mw->ui.playlistDetailSongList->indexAt(p);

    PlaylistItem selected=_getSelectedItem(idx);

    //	title etc not populated
    if(selected.key.length()>0)
    {
        _lastClickedIndex=idx;

        QPoint gp = mw->ui.playlistDetailSongList->mapToGlobal(p);

        if(_menu)
        {
            delete _menu;
        }
        _menu=new QMenu(NULL);

        _playNowAction->setText(QString("Play '%1' Now").arg(selected.text));
        _enqueueAction->setText(QString("Enqueue '%1'").arg(selected.text));

        _menu->addAction(_playNowAction);
        _menu->addAction(_enqueueAction);
        _menu->addAction(_deletePlaylistItemAction);
        _menu->exec(gp);
        _recordLastPopup(p);
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
        _deletePlaylistItemAction = new QAction(tr("Delete Item From Playlist "), this);
        _deletePlaylistItemAction->setStatusTip(tr("Delete Item From Playlist"));
        connect(_deletePlaylistItemAction, SIGNAL(triggered()),
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

//	There is a SBSqlQueryModel::determineSBID -- that is geared for AllSongs
//	This one is geared more for the lists that appears for each item (song, artist, etc).
//	NOTE:
//	A resultSet is assumed to contain the following columns (in random order):
//	-	'#'	position (optional)
//	-	SB_ITEM_TYPE
//	-	SB_ITEM_ID
//	The next field after this is assumed to contain the main item (e.g.: song title, album name, etc).
SBTabPlaylistDetail::PlaylistItem
SBTabPlaylistDetail::_getSelectedItem(const QModelIndex &idx)
{
    PlaylistItem currentPlaylistItem;

    _init();

    MainWindow* mw=Context::instance()->getMainWindow();
    QAbstractItemModel* aim=mw->ui.playlistDetailSongList->model();

    for(int i=0; i<aim->columnCount();i++)
    {
        QString header=aim->headerData(i, Qt::Horizontal).toString();
        header=header.toLower();
        QModelIndex idy=idx.sibling(idx.row(),i);

        if(header=="sb_item_key")
        {
            currentPlaylistItem.key=aim->data(idy).toString();
        }
        else if(header=="#")
        {
            currentPlaylistItem.playlistPosition=aim->data(idy).toInt();
        }
        else if(currentPlaylistItem.text.length()==0)
        {
            currentPlaylistItem.text=aim->data(idy).toString();
        }
    }

    return currentPlaylistItem;
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
        playlistPtr=std::dynamic_pointer_cast<SBIDPlaylist>(si.ptr());
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
    SBTableModel* tm=playlistPtr->items();
    populateTableView(tv,tm,0);
    connect(tm, SIGNAL(assign(const SBIDPtr&,int)),
            this, SLOT(movePlaylistItem(const SBIDPtr&, int)));

    //	Drag & drop mw->ui.playlistDetailSongList->setAcceptDrops(1);
    mw->ui.playlistDetailSongList->setDropIndicatorShown(1);
    mw->ui.playlistDetailSongList->viewport()->setAcceptDrops(1);
    mw->ui.playlistDetailSongList->setDefaultDropAction(Qt::MoveAction);

    return currentScreenItem;
}

