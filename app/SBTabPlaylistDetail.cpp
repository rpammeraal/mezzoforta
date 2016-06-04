#include <QAbstractItemModel>

#include "SBTabPlaylistDetail.h"

#include "Context.h"
#include "Controller.h"
#include "MainWindow.h"
#include "DataEntityPlaylist.h"
#include "PlayerController.h"
#include "SBModelCurrentPlaylist.h"
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
        DataEntityPlaylist* pl=new DataEntityPlaylist;
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
SBTabPlaylistDetail::movePlaylistItem(const SBID& fromID, int row)
{
    qDebug() << SB_DEBUG_INFO;

    init();
    //	Determine current playlist
    SBID currentID=Context::instance()->getScreenStack()->currentScreen();

    DataEntityPlaylist *mpl=new DataEntityPlaylist();
    mpl->reorderItem(currentID,fromID,row);
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

void
SBTabPlaylistDetail::songChanged(const SBID &newSong)
{
    qDebug() << SB_DEBUG_INFO << newSong;
    //	Go thru all items, reset icon, set icon if newSong is found
    const MainWindow* mw=Context::instance()->getMainWindow();
    QTableView* tv=mw->ui.playlistDetailSongList;
    QAbstractItemModel* aim=tv->model();

    qDebug() << SB_DEBUG_INFO << aim->metaObject()->className();
    for(int i=0;i<aim->rowCount();i++)
    {
        for(int j=0;j<aim->columnCount();j++) {
            QModelIndex idx=aim->index(i,j);
            QVariant v=aim->data(idx);
            qDebug() << SB_DEBUG_INFO << i << j << v.toString();
        }
        SBID id=SBID((SBID::sb_type)aim->data(aim->index(i,1)).toInt(),aim->data(aim->index(i,2)).toInt());
        id.sb_performer_id=aim->data(aim->index(i,9)).toInt();
        qDebug() << SB_DEBUG_INFO << i <<  "found=" << id;
        if(id==newSong)
        {
            qDebug() << SB_DEBUG_INFO << "MATCH";
            QStandardItem* newItem=new QStandardItem(QIcon(":/images/playing.png"),aim->data(aim->index(i,3)).toString());
            //aim->setData(aim->index(i,3),newItem);
        }
    }
}

void
SBTabPlaylistDetail::tableViewCellClicked(QModelIndex idx)
{
    qDebug() << SB_DEBUG_INFO << idx.column() << idx.row();
    if((SBModelCurrentPlaylist::sb_column_type)idx.column()==SBModelCurrentPlaylist::sb_column_playlistpositionid)
    {
        qDebug() << SB_DEBUG_INFO;
    }
    else
    {
        SBID item=getSBIDSelected(idx);
        qDebug() << SB_DEBUG_INFO << item;
        Context::instance()->getNavigator()->openScreenByID(item);
    }
}

void
SBTabPlaylistDetail::playlistReordered()
{
    //	Go through model and reupdate playlist in database.
    DataEntityPlaylist pl;
    MainWindow* mw=Context::instance()->getMainWindow();
    QTableView* tv=mw->ui.playlistDetailSongList;
    SBModelCurrentPlaylist* aem=dynamic_cast<SBModelCurrentPlaylist *>(tv->model());
    QStandardItem* item;
    SBID playlist=Context::instance()->getScreenStack()->currentScreen();
    SBID::sb_type itemType=SBID::sb_type_invalid;
    int itemID=-1;
    SB_DEBUG_IF_NULL(aem);

    for(int i=0;i<aem->rowCount();i++)
    {
        qDebug() << SB_DEBUG_INFO << i
                 <<  "title=" << aem->item(i,SBModelCurrentPlaylist::sb_column_songtitle)->text()
                 <<  "playlistpositionid=" << aem->item(i,SBModelCurrentPlaylist::sb_column_playlistpositionid)->text()
        ;
        item=aem->item(i,SBModelCurrentPlaylist::sb_column_item_type);
        itemType=static_cast<SBID::sb_type>((item!=NULL)?item->text().toInt():-1);
        item=aem->item(i,SBModelCurrentPlaylist::sb_column_item_id);
        itemID=(item!=NULL)?item->text().toInt():-1;
        SBID id(itemType,itemID);

        item=aem->item(i,SBModelCurrentPlaylist::sb_column_songid);
        id.sb_song_id=(item!=NULL)?item->text().toInt():-1;
        item=aem->item(i,SBModelCurrentPlaylist::sb_column_performerid);
        id.sb_performer_id=(item!=NULL)?item->text().toInt():-1;
        item=aem->item(i,SBModelCurrentPlaylist::sb_column_albumid);
        id.sb_album_id=(item!=NULL)?item->text().toInt():-1;
        item=aem->item(i,SBModelCurrentPlaylist::sb_column_position);
        id.sb_position=(item!=NULL)?item->text().toInt():-1;

        pl.reorderItem(playlist,id,i+1);
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

        //connect(mw->ui.playlistDetailSongList->horizontalHeader(), SIGNAL(sectionClicked(int)),
                //this, SLOT(sortOrderChanged(int)));

        //	Delete playlist
        deletePlaylistItemAction = new QAction(tr("Delete Item From Playlist "), this);
        deletePlaylistItemAction->setStatusTip(tr("Delete Item From Playlist"));
        connect(deletePlaylistItemAction, SIGNAL(triggered()),
                this, SLOT(deletePlaylistItem()));

        //	Be aware of song changes
        PlayerController* pc=Context::instance()->getPlayerController();
        if(pc)
        {
            qDebug() << SB_DEBUG_INFO;
            connect(pc,SIGNAL(songChanged(SBID)),
                    this,SLOT(songChanged(SBID)));
        }
        qDebug() << SB_DEBUG_INFO;
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
    SBID id;

    MainWindow* mw=Context::instance()->getMainWindow();
    QTableView* tv=mw->ui.playlistDetailSongList;
    SBModelCurrentPlaylist* aem=dynamic_cast<SBModelCurrentPlaylist *>(tv->model());
    SBID::sb_type itemType=SBID::sb_type_invalid;
    int itemID=-1;
    QStandardItem* item;
    QStandardItem* i2;

    switch((SBModelCurrentPlaylist::sb_column_type)idx.column())
    {
    case SBModelCurrentPlaylist::sb_column_deleteflag:
    case SBModelCurrentPlaylist::sb_column_item_type:
    case SBModelCurrentPlaylist::sb_column_item_id:
    case SBModelCurrentPlaylist::sb_column_playflag:
    case SBModelCurrentPlaylist::sb_column_albumid:
    case SBModelCurrentPlaylist::sb_column_displayplaylistpositionid:
    case SBModelCurrentPlaylist::sb_column_songid:
    case SBModelCurrentPlaylist::sb_column_performerid:
    case SBModelCurrentPlaylist::sb_column_playlistpositionid:
    case SBModelCurrentPlaylist::sb_column_position:
    case SBModelCurrentPlaylist::sb_column_path:
        break;

    case SBModelCurrentPlaylist::sb_column_songtitle:
    case SBModelCurrentPlaylist::sb_column_duration:
        itemType=SBID::sb_type_song;
        item=aem->item(idx.row(),SBModelCurrentPlaylist::sb_column_songid);
        itemID=(item!=NULL)?item->text().toInt():-1;
        break;

    case SBModelCurrentPlaylist::sb_column_performername:
        itemType=SBID::sb_type_performer;
        item=aem->item(idx.row(),SBModelCurrentPlaylist::sb_column_performerid);
        itemID=(item!=NULL)?item->text().toInt():-1;
        break;

    case SBModelCurrentPlaylist::sb_column_albumtitle:
        itemType=SBID::sb_type_album;
        item=aem->item(idx.row(),SBModelCurrentPlaylist::sb_column_albumid);
        itemID=(item!=NULL)?item->text().toInt():-1;
        break;

    case SBModelCurrentPlaylist::sb_column_generic:
        item=aem->item(idx.row(),SBModelCurrentPlaylist::sb_column_item_type);
        itemType=static_cast<SBID::sb_type>((item!=NULL)?item->text().toInt():-1);
        item=aem->item(idx.row(),SBModelCurrentPlaylist::sb_column_item_id);
        itemID=(item!=NULL)?item->text().toInt():-1;
        break;

    }

    id.assign(itemType,itemID);
    qDebug() << SB_DEBUG_INFO << id;
    return id;
}

QMap<int,SBID>
SBTabPlaylistDetail::_getAllItemsByPlaylist(const SBID& id) const
{
    QMap<int,SBID> list;
    DataEntityPlaylist pl;
    SBSqlQueryModel* qm=pl.getAllItemsByPlaylist(id);

    for(int i=0;i<qm->rowCount();i++)
    {
        SBID id=SBID((SBID::sb_type)qm->record(i).value(1).toInt(),qm->record(i).value(2).toInt());
        id.playPosition=qm->record(i).value(0).toInt();
        id.playlistName=qm->record(i).value(3).toString();
        //	qm->record(i).value(3).toString();	CHART
        id.songTitle=qm->record(i).value(5).toString();
        id.sb_album_id=qm->record(i).value(6).toInt();
        id.albumTitle=qm->record(i).value(7).toString();
        id.sb_position=qm->record(i).value(8).toInt();
        id.duration=qm->record(i).value(9).toTime();
        id.sb_performer_id=qm->record(i).value(10).toInt();
        id.performerName=qm->record(i).value(11).toString();

        list[i]=id;
    }

    return list;
}

SBID
SBTabPlaylistDetail::_populate(const SBID& id)
{
    qDebug() << SB_DEBUG_INFO;
    init();
    const MainWindow* mw=Context::instance()->getMainWindow();
    DataEntityPlaylist pl;

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
    //SBSqlQueryModel* qm=pl.getAllItemsByPlaylist(id);
    //populateTableView(tv,qm,0);

    QMap<int,SBID> list=this->_getAllItemsByPlaylist(id);
    SBModelCurrentPlaylist* aem=dynamic_cast<SBModelCurrentPlaylist *>(tv->model());
    if(aem!=NULL)
    {
        qDebug() << SB_DEBUG_INFO;
        delete aem; aem=NULL;
    }
    aem=new SBModelCurrentPlaylist();

    aem->populate(list,0);
    tv->setModel(aem);

    //populateTableView(tv,aem,0);

    connect(tv, SIGNAL(clicked(QModelIndex)),
            this, SLOT(tableViewCellClicked(QModelIndex)));
    connect(aem, SIGNAL(assign(const SBID&,int)),
            this, SLOT(movePlaylistItem(const SBID&, int)));
    connect(aem,SIGNAL(playlistOrderChanged()),
            this,SLOT(playlistReordered()));

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


void
SBTabPlaylistDetail::_populatePost(const SBID& id)
{
    //	1.	Init
    Q_UNUSED(id);
    const MainWindow* mw=Context::instance()->getMainWindow();
    QTableView* tv= mw->ui.playlistDetailSongList;

    //	2.	Set up view
    tv->setSelectionMode(QAbstractItemView::ExtendedSelection);
    tv->setDragEnabled(1);
    tv->setAcceptDrops(1);
    tv->viewport()->setAcceptDrops(1);
    tv->setDropIndicatorShown(1);
    tv->setDragDropMode(QAbstractItemView::InternalMove);
    tv->setDefaultDropAction(Qt::MoveAction);
    tv->setDragDropOverwriteMode(false);

    //	3.	Set visibility
    tv->setColumnHidden(0,1);	//	sb_column_deleteflag
    tv->setColumnHidden(1,1);	//	sb_column_item_type
    tv->setColumnHidden(2,1);	//	sb_column_item_id
    tv->setColumnHidden(3,1);	//	sb_column_playflag
    tv->setColumnHidden(4,1);	//	sb_column_albumid
    tv->setColumnHidden(5,0);	//	sb_column_displayplaylistpositionid
    tv->setColumnHidden(6,1);	//	sb_column_songid
    tv->setColumnHidden(7,1);	//	sb_column_performerid
    tv->setColumnHidden(8,1);	//	sb_column_playlistpositionid
    tv->setColumnHidden(9,1);	//	sb_column_position
    tv->setColumnHidden(10,1);	//	sb_column_path
    tv->setColumnHidden(11,1);	//	sb_column_songtitle
    tv->setColumnHidden(12,1);	//	sb_column_duration
    tv->setColumnHidden(13,1);	//	sb_column_performername
    tv->setColumnHidden(14,1);	//	sb_column_albumtitle
    tv->setColumnHidden(15,0);	//	sb_column_generic

    //	4.	Set headers.
    QHeaderView* hv=NULL;
    hv=tv->horizontalHeader();
    hv->setSectionResizeMode(QHeaderView::ResizeToContents);
    hv->setStretchLastSection(1);
    hv->setSortIndicator(SBModelCurrentPlaylist::sb_column_displayplaylistpositionid,Qt::AscendingOrder);

    hv=tv->verticalHeader();
    hv->hide();
    hv->setDefaultSectionSize(18);

    tv->setEditTriggers(QAbstractItemView::AllEditTriggers);
}
