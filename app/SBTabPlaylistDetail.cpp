#include <QAbstractItemModel>
#include <QMenu>

#include "SBTabPlaylistDetail.h"

#include "Context.h"
#include "Controller.h"
#include "MainWindow.h"
#include "DataEntityPlaylist.h"
#include "PlayerController.h"
#include "PlayManager.h"
#include "SBSqlQueryModel.h"
#include "Navigator.h"

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
    SBID currentID=SBTab::currentID();
    SBID assignID=getSBIDSelected(_lastClickedIndex);
    if(assignID.sb_item_type()!=SBID::sb_type_invalid)
    {
        DataEntityPlaylist pl;

        pl.deletePlaylistItem(assignID,currentID);
        refreshTabIfCurrent(currentID);
        QString updateText=QString("Removed %5 %1%2%3 from %6 %1%4%3.")
            .arg(QChar(96))            //	1
            .arg(assignID.getText())   //	2
            .arg(QChar(180))           //	3
            .arg(currentID.getText())     //	4
            .arg(assignID.getType())   //	5
            .arg(currentID.getType());    //	6
        Context::instance()->getController()->updateStatusBarText(updateText);

        _populate(currentID);
    }
    if(_menu)
    {
        _menu->hide();
    }
}

void
SBTabPlaylistDetail::movePlaylistItem(const SBID& fromID, int row)
{
    _init();
    //	Determine current playlist
    SBID currentID=SBTab::currentID();

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
    const SBID currentID=SBTab::currentID();
    SBID selectedID=getSBIDSelected(_lastClickedIndex);

    if(selectedID.sb_item_type()==SBID::sb_type_invalid)
    {
        //	Label clicked
        selectedID=currentID;
    }
    else
    {
        if(selectedID.sb_item_type()==SBID::sb_type_song)
        {
            DataEntityPlaylist pl;

            selectedID.sb_playlist_id=currentID.sb_playlist_id;
            selectedID=pl.getDetailPlaylistItemSong(selectedID);	//	gets path, fills in 4 field titles
        }
    }
    PlayManager* pmgr=Context::instance()->getPlayManager();
    pmgr?pmgr->playItemNow(selectedID,enqueueFlag):NULL;
    SBTab::playNow(enqueueFlag);
}

void
SBTabPlaylistDetail::showContextMenuLabel(const QPoint &p)
{
    const SBID currentID=SBTab::currentID();
    _lastClickedIndex=QModelIndex();

    _menu=new QMenu(NULL);

    _playNowAction->setText(QString("Play '%1' Now").arg(currentID.getText()));
    _enqueueAction->setText(QString("Enqueue '%1'").arg(currentID.getText()));

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

    SBID selectedID=getSBIDSelected(idx);

    //	title etc not populated
    if(selectedID.sb_item_type()!=SBID::sb_type_invalid)
    {
        _lastClickedIndex=idx;

        QPoint gp = mw->ui.playlistDetailSongList->mapToGlobal(p);

        if(_menu)
        {
            delete _menu;
        }
        _menu=new QMenu(NULL);

        _playNowAction->setText(QString("Play '%1' Now").arg(selectedID.getText()));
        _enqueueAction->setText(QString("Enqueue '%1'").arg(selectedID.getText()));

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
SBID
SBTabPlaylistDetail::getSBIDSelected(const QModelIndex &idx)
{
    static QModelIndex lastIdx;
    static SBID lastSong;

    _init();
    SBID id;

    MainWindow* mw=Context::instance()->getMainWindow();
    QAbstractItemModel* aim=mw->ui.playlistDetailSongList->model();

    QString text;
    int itemID=-1;
    SBID::sb_type itemType=SBID::sb_type_invalid;
    for(int i=0; i<aim->columnCount();i++)
    {
        QString header=aim->headerData(i, Qt::Horizontal).toString();
        header=header.toLower();
        QModelIndex idy=idx.sibling(idx.row(),i);

        if(header=="sb_item_type")
        {
            itemType=static_cast<SBID::sb_type>(aim->data(idy).toInt());
        }
        else if(header=="sb_item_id")
        {
            itemID=aim->data(idy).toInt();
        }
        else if(header=="#")
        {
            id.sb_playlist_position=aim->data(idy).toInt();
        }
        else if(text.length()==0)
        {
            text=aim->data(idy).toString();
        }
    }
    id.assign(itemType,itemID);
    lastIdx=idx;
    lastSong=id;
    id.setText(text);
    return id;
}

SBID
SBTabPlaylistDetail::_populate(const SBID& id)
{
    _init();
    const MainWindow* mw=Context::instance()->getMainWindow();
    DataEntityPlaylist pl;

    SBID result=pl.getDetail(id);
    if(result.sb_playlist_id==-1)
    {
        //	Not found
        return result;
    }
    SBTab::_populate(result);
    mw->ui.labelPlaylistDetailIcon->setSBID(result);

    mw->ui.labelPlaylistDetailPlaylistName->setText(result.playlistName);
    const QString detail=QString("%1 items ").arg(result.count1)+QChar(8226)+QString(" %2").arg(result.duration.toString());
    mw->ui.labelPlaylistDetailPlaylistDetail->setText(detail);

    QTableView* tv=mw->ui.playlistDetailSongList;
    SBSqlQueryModel* qm=pl.getAllItemsByPlaylist(id);
    populateTableView(tv,qm,0);
    connect(qm, SIGNAL(assign(const SBID&,int)),
            this, SLOT(movePlaylistItem(const SBID&, int)));

    //	Drag & drop mw->ui.playlistDetailSongList->setAcceptDrops(1);
    mw->ui.playlistDetailSongList->setDropIndicatorShown(1);
    mw->ui.playlistDetailSongList->viewport()->setAcceptDrops(1);
    mw->ui.playlistDetailSongList->setDefaultDropAction(Qt::MoveAction);

    return result;
}


void
SBTabPlaylistDetail::_populatePost(const SBID& id)
{
    Q_UNUSED(id);
}
