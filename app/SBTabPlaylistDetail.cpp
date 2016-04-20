#include "SBTabPlaylistDetail.h"

#include "Context.h"
#include "Controller.h"
#include "MainWindow.h"
#include "SBModelPlaylist.h"
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
    init();
    SBID fromID=Context::instance()->getScreenStack()->currentScreen();
    SBID assignID=getSBIDSelected(lastClickedIndex);
    if(assignID.sb_item_type()!=SBID::sb_type_invalid)
    {
        SBModelPlaylist* pl=new SBModelPlaylist;
        connect(pl, SIGNAL(si_playlistDetailUpdated(SBID)),
                 this, SLOT(sl_updatePlaylistDetail(SBID)));

        pl->deletePlaylistItem(assignID,fromID);
        refreshTabIfCurrent(fromID);
        QString updateText=QString("Removed %5 %1%2%3 from %6 %1%4%3.")
            .arg(QChar(96))            //	1
            .arg(assignID.getText())   //	2
            .arg(QChar(180))           //	3
            .arg(fromID.getText())     //	4
            .arg(assignID.getType())   //	5
            .arg(fromID.getType());    //	6
        Context::instance()->getController()->updateStatusBarText(updateText);
    }
}

void
SBTabPlaylistDetail::movePlaylistItem(const SBID& fromID, const SBID &toID)
{
    init();
    //	Determine current playlist
    SBID currentID=Context::instance()->getScreenStack()->currentScreen();

    SBModelPlaylist *mpl=new SBModelPlaylist();
    mpl->reorderItem(currentID,fromID,toID);
    refreshTabIfCurrent(currentID);
}

void
SBTabPlaylistDetail::showContextMenuPlaylist(const QPoint &p)
{
    init();
    const MainWindow* mw=Context::instance()->getMainWindow();
    QModelIndex idx=mw->ui.playlistDetailSongList->indexAt(p);

    SBID id=getSBIDSelected(idx);
    if(id.sb_item_type()!=SBID::sb_type_invalid)
    {
        lastClickedIndex=idx;

        QPoint gp = mw->ui.playlistDetailSongList->mapToGlobal(p);

        QMenu menu(NULL);
        menu.addAction(deletePlaylistItemAction);
        menu.exec(gp);
    }
}

///	Private methods
void
SBTabPlaylistDetail::init()
{
    if(_initDoneFlag==0)
    {
        MainWindow* mw=Context::instance()->getMainWindow();

        _initDoneFlag=1;

        connect(mw->ui.playlistDetailSongList->horizontalHeader(), SIGNAL(sectionClicked(int)),
                this, SLOT(sortOrderChanged(int)));

        //	Delete playlist
        deletePlaylistItemAction = new QAction(tr("Delete Item From Playlist "), this);
        deletePlaylistItemAction->setStatusTip(tr("Delete Item From Playlist"));
        connect(deletePlaylistItemAction, SIGNAL(triggered()),
                this, SLOT(deletePlaylistItem()));
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
    init();
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
            id.sb_position=aim->data(idy).toInt();
        }
        else if(text.length()==0)
        {
            text=aim->data(idy).toString();
        }
    }
    id.assign(itemType,itemID);
    return id;
}

SBID
SBTabPlaylistDetail::_populate(const SBID& id)
{
    qDebug() << SB_DEBUG_INFO;
    init();
    const MainWindow* mw=Context::instance()->getMainWindow();
    SBModelPlaylist pl;

    SBID result=pl.getDetail(id);
    if(result.sb_playlist_id==-1)
    {
        //	Not found
        return result;
    }
    mw->ui.labelPlaylistDetailIcon->setSBID(result);

    mw->ui.labelPlaylistDetailPlaylistName->setText(result.playlistName);
    QString detail=QString("%1 items ").arg(result.count1)+QChar(8226)+QString(" %2 playtime").arg(result.duration.toString());
    mw->ui.labelPlaylistDetailPlaylistDetail->setText(detail);

    QTableView* tv=mw->ui.playlistDetailSongList;
    SBSqlQueryModel* qm=pl.getAllItemsByPlaylist(id);
    populateTableView(tv,qm,0);
    connect(tv, SIGNAL(clicked(QModelIndex)),
            this, SLOT(tableViewCellClicked(QModelIndex)));
    connect(qm, SIGNAL(assign(const SBID&,const SBID&)),
            this, SLOT(movePlaylistItem(const SBID&, const SBID&)));

    //	Context menu
    tv->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(tv, SIGNAL(customContextMenuRequested(const QPoint&)),
            this, SLOT(showContextMenuPlaylist(QPoint)));

    //	Drag & drop mw->ui.playlistDetailSongList->setAcceptDrops(1);
    mw->ui.playlistDetailSongList->setDropIndicatorShown(1);
    mw->ui.playlistDetailSongList->viewport()->setAcceptDrops(1);
    mw->ui.playlistDetailSongList->setDefaultDropAction(Qt::MoveAction);

    return result;
}
