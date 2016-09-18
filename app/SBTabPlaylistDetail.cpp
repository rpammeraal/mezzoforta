//#include <QAbstractItemModel>
//#include <QMenu>

#include "SBTabPlaylistDetail.h"

#include "Context.h"
#include "Controller.h"
#include "MainWindow.h"
#include "DataEntityPlaylist.h"
#include "SBSqlQueryModel.h"

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
    _init();
    SBIDPlaylist currentID=this->currentScreenItem().base();
    SBIDPlaylist assignID=_getSBIDSelected(_lastClickedIndex);
    if(assignID.itemType()!=SBIDBase::sb_type_invalid)
    {
        DataEntityPlaylist pl;

        pl.deletePlaylistItem(assignID,currentID);
        refreshTabIfCurrent(currentID);
        QString updateText=QString("Removed %5 %1%2%3 from %6 %1%4%3.")
            .arg(QChar(96))            //	1
            .arg(assignID.text())   //	2
            .arg(QChar(180))           //	3
            .arg(currentID.text())     //	4
            .arg(assignID.type())   //	5
            .arg(currentID.type());    //	6
        Context::instance()->getController()->updateStatusBarText(updateText);

        _populate(currentID);
    }
    if(_menu)
    {
        _menu->hide();
    }
}

void
SBTabPlaylistDetail::movePlaylistItem(const SBIDBase& fromID, int row)
{
    _init();
    //	Determine current playlist
    SBIDPlaylist currentID=this->currentScreenItem().base();

    DataEntityPlaylist mpl;
    mpl.reorderItem(currentID,fromID,row);
    refreshTabIfCurrent(currentID);

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
    const SBIDPlaylist currentID=this->currentScreenItem().base();
    SBIDPlaylist selectedID=_getSBIDSelected(_lastClickedIndex);

    if(selectedID.itemType()==SBIDBase::sb_type_invalid)
    {
        //	Label clicked
        selectedID=currentID;
    }
    else
    {
        if(selectedID.itemType()==SBIDBase::sb_type_song)
        {
            DataEntityPlaylist pl;

            selectedID.setPlaylistID(currentID.playlistID());
            selectedID=pl.getDetailPlaylistItemSong(selectedID);	//	gets path, fills in 4 field titles
        }
    }
    PlayManager* pmgr=Context::instance()->getPlayManager();
    pmgr?pmgr->playItemNow(selectedID,enqueueFlag):0;
    SBTab::playNow(enqueueFlag);
}

void
SBTabPlaylistDetail::showContextMenuLabel(const QPoint &p)
{
    const SBIDPlaylist currentID=this->currentScreenItem().base();
    _lastClickedIndex=QModelIndex();

    _menu=new QMenu(NULL);

    _playNowAction->setText(QString("Play '%1' Now").arg(currentID.text()));
    _enqueueAction->setText(QString("Enqueue '%1'").arg(currentID.text()));

    _menu->addAction(_playNowAction);
    _menu->addAction(_enqueueAction);
    _menu->exec(p);
}

void
SBTabPlaylistDetail::showContextMenuView(const QPoint &p)
{
    _init();
    const MainWindow* mw=Context::instance()->getMainWindow();
    QModelIndex idx=mw->ui.playlistDetailSongList->indexAt(p);

    SBIDPlaylist selectedID=_getSBIDSelected(idx);

    //	title etc not populated
    if(selectedID.itemType()!=SBIDBase::sb_type_invalid)
    {
        _lastClickedIndex=idx;

        QPoint gp = mw->ui.playlistDetailSongList->mapToGlobal(p);

        if(_menu)
        {
            delete _menu;
        }
        _menu=new QMenu(NULL);

        _playNowAction->setText(QString("Play '%1' Now").arg(selectedID.text()));
        _enqueueAction->setText(QString("Enqueue '%1'").arg(selectedID.text()));

        _menu->addAction(_playNowAction);
        _menu->addAction(_enqueueAction);
        _menu->addAction(_deletePlaylistItemAction);
        _menu->exec(gp);
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
SBIDBase
SBTabPlaylistDetail::_getSBIDSelected(const QModelIndex &idx)
{
    static QModelIndex lastIdx;
    static SBIDBase lastItem;
    int position=-1;

    _init();
    SBIDBase id;

    MainWindow* mw=Context::instance()->getMainWindow();
    QAbstractItemModel* aim=mw->ui.playlistDetailSongList->model();

    QString text;
    int itemID=-1;
    SBIDBase::sb_type itemType=SBIDBase::sb_type_invalid;
    for(int i=0; i<aim->columnCount();i++)
    {
        QString header=aim->headerData(i, Qt::Horizontal).toString();
        header=header.toLower();
        QModelIndex idy=idx.sibling(idx.row(),i);

        if(header=="sb_item_type")
        {
            itemType=static_cast<SBIDBase::sb_type>(aim->data(idy).toInt());
        }
        else if(header=="sb_item_id")
        {
            itemID=aim->data(idy).toInt();
        }
        else if(header=="#")
        {
            position=aim->data(idy).toInt();
        }
        else if(text.length()==0)
        {
            text=aim->data(idy).toString();
        }
    }
    qDebug() << SB_DEBUG_INFO << itemType << itemID << position << text;
    switch(itemType)
    {
    case SBIDBase::sb_type_album:
        id=SBIDAlbum(itemID);
        break;

    case SBIDBase::sb_type_performer:
        id=SBIDPerformer(itemID);
        break;

    case SBIDBase::sb_type_playlist:
        id=SBIDPlaylist(itemID);
        break;

    case SBIDBase::sb_type_song:
        id=SBIDSong(itemID);
        break;

    case SBIDBase::sb_type_invalid:
    case SBIDBase::sb_type_chart:
        break;
    }

    lastIdx=idx;
    lastItem=id;
    qDebug() << SB_DEBUG_INFO << (long)&id;
    id.setText(text);
    qDebug() << SB_DEBUG_INFO << id.text();
    qDebug() << SB_DEBUG_INFO << (long)&id;
    return id;
}

ScreenItem
SBTabPlaylistDetail::_populate(const ScreenItem& si)
{
    _init();
    const MainWindow* mw=Context::instance()->getMainWindow();
    DataEntityPlaylist pl;

    //	Get detail
    SBIDBase base=pl.getDetail(si.base());
    if(base.validFlag()==0)
    {
        //	Not found
        return base;
    }
    ScreenItem currentScreenItem=si;
    //SBTab::_setCurrentScreenItem(currentScreenItem);
    mw->ui.labelPlaylistDetailIcon->setSBID(base);

    mw->ui.labelPlaylistDetailPlaylistName->setText(base.playlistName());
    const QString detail=QString("%1 items ").arg(base.count1())+QChar(8226)+QString(" %2").arg(base.duration().toString());
    mw->ui.labelPlaylistDetailPlaylistDetail->setText(detail);

    QTableView* tv=mw->ui.playlistDetailSongList;
    SBSqlQueryModel* qm=pl.getAllItemsByPlaylist(base);
    populateTableView(tv,qm,0);
    connect(qm, SIGNAL(assign(const SBIDBase&,int)),
            this, SLOT(movePlaylistItem(const SBIDBase&, int)));

    //	Drag & drop mw->ui.playlistDetailSongList->setAcceptDrops(1);
    mw->ui.playlistDetailSongList->setDropIndicatorShown(1);
    mw->ui.playlistDetailSongList->viewport()->setAcceptDrops(1);
    mw->ui.playlistDetailSongList->setDefaultDropAction(Qt::MoveAction);

    return currentScreenItem;
}

