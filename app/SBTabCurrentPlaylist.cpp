#include "SBTabCurrentPlaylist.h"

#include <QProgressDialog>

#include "Common.h"
#include "Context.h"
#include "Controller.h"
#include "SBCurrentPlaylistModel.h"
#include "MainWindow.h"
#include "PlayerController.h"
#include "DataEntityCurrentPlaylist.h"
#include "DataEntityPlaylist.h"
#include "DataEntitySong.h"
#include "SBSqlQueryModel.h"
#include "Navigator.h"


SBTabCurrentPlaylist::SBTabCurrentPlaylist(QWidget* parent) : SBTab(parent,0)
{
}

QTableView*
SBTabCurrentPlaylist::subtabID2TableView(int subtabID) const
{
    Q_UNUSED(subtabID);
    const MainWindow* mw=Context::instance()->getMainWindow();
    return mw->ui.currentPlaylistDetailSongList;
}

///	Public slots
void
SBTabCurrentPlaylist::deletePlaylistItem()
{
    SBID assignID=getSBIDSelected(_lastClickedIndex);
    if(assignID.sb_item_type()!=SBID::sb_type_invalid)
    {
        MainWindow* mw=Context::instance()->getMainWindow();
        QTableView* tv=mw->ui.currentPlaylistDetailSongList;
        SBCurrentPlaylistModel* aem=dynamic_cast<SBCurrentPlaylistModel *>(tv->model());
        aem->removeRows(_lastClickedIndex.row(),1,QModelIndex());
        qDebug() << SB_DEBUG_INFO << _lastClickedIndex << _lastClickedIndex.row() << _lastClickedIndex.column();

        QString updateText=QString("Removed %4 %1%2%3 from playlist.")
            .arg(QChar(96))            //	1
            .arg(assignID.getText())   //	2
            .arg(QChar(180))           //	3
            .arg(assignID.getType())   //	4
        ;
        Context::instance()->getController()->updateStatusBarText(updateText);
    }
}

void
SBTabCurrentPlaylist::movePlaylistItem(const SBID& fromID, const SBID &toID)
{
    //	Determine current playlist
    SBID currentID=Context::instance()->getScreenStack()->currentScreen();
    qDebug() << SB_DEBUG_INFO << currentID << fromID << toID;
    return;

    DataEntityPlaylist *mpl=new DataEntityPlaylist();
    mpl->reorderItem(currentID,fromID,toID);
    refreshTabIfCurrent(currentID);
}

void
SBTabCurrentPlaylist::playSong()
{
    qDebug() << SB_DEBUG_INFO << _lastClickedIndex;
    PlayerController* pc=Context::instance()->getPlayerController();

    pc->playerStop();
    pc->playerPlay(_lastClickedIndex.row());
}

void
SBTabCurrentPlaylist::showContextMenuPlaylist(const QPoint &p)
{
    const MainWindow* mw=Context::instance()->getMainWindow();
    QModelIndex idx=mw->ui.currentPlaylistDetailSongList->indexAt(p);

    SBID id=getSBIDSelected(idx);
    qDebug() << SB_DEBUG_INFO << id;
    if(id.sb_item_type()!=SBID::sb_type_invalid)
    {
        _lastClickedIndex=idx;

        QPoint gp = mw->ui.currentPlaylistDetailSongList->mapToGlobal(p);

        QMenu menu(NULL);
        menu.addAction(deletePlaylistItemAction);
        menu.addAction(playSongNowAction);
        menu.exec(gp);
    }
}

void
SBTabCurrentPlaylist::songChanged(int playID)
{
    qDebug() << SB_DEBUG_INFO << playID;
    MainWindow* mw=Context::instance()->getMainWindow();
    QTableView* tv=mw->ui.currentPlaylistDetailSongList;
    SBCurrentPlaylistModel* aem=dynamic_cast<SBCurrentPlaylistModel *>(tv->model());
    QModelIndex idx=aem->setSongPlaying(playID);
    qDebug() << SB_DEBUG_INFO << idx << idx.row() << idx.column();
    tv->scrollTo(idx);
}

///	Private slots
void
SBTabCurrentPlaylist::clearPlaylist()
{
    qDebug() << SB_DEBUG_INFO;
    MainWindow* mw=Context::instance()->getMainWindow();
    QTableView* tv=mw->ui.currentPlaylistDetailSongList;
    PlayerController* pc=Context::instance()->getPlayerController();
    SBCurrentPlaylistModel* aem=dynamic_cast<SBCurrentPlaylistModel *>(tv->model());
    aem->clear();
    pc->clearPlaylist();
}

void
SBTabCurrentPlaylist::shufflePlaylist()
{
    qDebug() << SB_DEBUG_INFO;
    MainWindow* mw=Context::instance()->getMainWindow();
    QTableView* tv=mw->ui.currentPlaylistDetailSongList;
    SBCurrentPlaylistModel* aem=dynamic_cast<SBCurrentPlaylistModel *>(tv->model());
    aem->shuffle();
    tv->sortByColumn(5,Qt::AscendingOrder);
    aem->repaintAll();
}

void
SBTabCurrentPlaylist::startRadio()
{
    MainWindow* mw=Context::instance()->getMainWindow();
    QTableView* tv=mw->ui.currentPlaylistDetailSongList;
    SBCurrentPlaylistModel* aem=dynamic_cast<SBCurrentPlaylistModel *>(tv->model());
    PlayerController* pc=Context::instance()->getPlayerController();
    const int firstBatchNumber=5;
    bool firstBatchLoaded=false;

    qDebug() << SB_DEBUG_INFO;
    this->clearPlaylist();

    QMap<int,SBID> playList;
    QList<int> indexCovered;

    int progressStep=0;
    QProgressDialog pd("Starting Auto DJ",QString(),0,11);
    pd.setWindowModality(Qt::WindowModal);
    pd.show();
    pd.raise();
    pd.activateWindow();
    QCoreApplication::processEvents();
    pd.setValue(0);
    QCoreApplication::processEvents();

    SBSqlQueryModel* qm=DataEntityCurrentPlaylist::getAllOnlineSongs();
    pd.setValue(++progressStep);
    QCoreApplication::processEvents();

    int numSongs=qm->rowCount();
    const int maxNumberAttempts=50;
    int songInterval=numSongs/10;

    qDebug() << SB_DEBUG_INFO << "randomizing " << numSongs << "songs";
    bool found=1;
    int nextOpenSlotIndex=0;
    for(;found && nextOpenSlotIndex<numSongs;nextOpenSlotIndex++)
    {
        found=0;
        int idx=-1;

        for(int j=maxNumberAttempts;j && !found;j--)
        {
            idx=Common::randomOldestFirst(numSongs);
            if(indexCovered.contains(idx)==0)
            {
                found=1;
                indexCovered.append(idx);
            }
        }

        if(!found)
        {
            //	If we can't get a random index after n tries, get the first
            //	not-used index
            for(int i=0;i<numSongs && found==0;i++)
            {
                if(indexCovered.contains(i)==0)
                {
                    idx=i;
                    found=1;
                }
            }
        }

        SBID item=SBID(SBID::sb_type_song,qm->record(idx).value(0).toInt());

        item.songTitle=qm->record(idx).value(1).toString();
        item.sb_performer_id=qm->record(idx).value(2).toInt();
        item.performerName=qm->record(idx).value(3).toString();
        item.sb_album_id=qm->record(idx).value(4).toInt();
        item.albumTitle=qm->record(idx).value(5).toString();
        item.sb_position=qm->record(idx).value(6).toInt();
        item.path=qm->record(idx).value(7).toString();
        item.duration=qm->record(idx).value(8).toTime();

        playList[nextOpenSlotIndex]=item;

        if(nextOpenSlotIndex%songInterval==0 || nextOpenSlotIndex+1==numSongs)
        {
            //	Update progress
            pd.setValue(++progressStep);
            QCoreApplication::processEvents();
        }

        //	Load the 1st n songs as soon as we get n songs or load the remainder after all songs are retrieved
        if(nextOpenSlotIndex+1==firstBatchNumber || nextOpenSlotIndex+1==numSongs)
        {
            if(!firstBatchLoaded)
            {
                qDebug() << SB_DEBUG_INFO << "sending to aem first batch:playList.count()=" << playList.count();
                aem->populate(playList);
                this->_populatePost(SBID());

                //	This code could be reused in other situations.
                //	Stop player, tell playerController that we have a new playlist and start player.
                pc->playerStop();
                pc->loadPlaylist(playList,firstBatchLoaded);
                //pc->playerPlay();
                pc->playerNext();
                //	End reuseable

                firstBatchLoaded=true;
            }
            else
            {
                qDebug() << SB_DEBUG_INFO << "sending to aem 2nd batch:playList.count()=" << playList.count();
                pc->loadPlaylist(playList,firstBatchLoaded);
                aem->populate(playList,firstBatchLoaded);
                qDebug() << SB_DEBUG_INFO << "after sending to aem 2nd batch:playList.count()=" << playList.count();
            }
        }
//        qDebug() << SB_DEBUG_INFO
//                 << ":nextOpenSlotIndex=" << nextOpenSlotIndex
//                 << ":found=" << found
//                 << ":numSongs=" << numSongs
//        ;
    }

    QString allIDX=" ";
    for(int i=0;i<indexCovered.count();i++)
    {
        if(indexCovered.contains(i))
        {
            allIDX+=QString("%1 ").arg(i);
        }
    }
    qDebug() << SB_DEBUG_INFO << "allIDX=" << allIDX;
    qDebug() << SB_DEBUG_INFO << "Populated" << playList.count() << "of" << numSongs;
    qDebug() << SB_DEBUG_INFO << "indexCovered.count" << indexCovered.count();

//    qDebug() << SB_DEBUG_INFO << "contents playlist";
//    for(int i=0;i<playList.count();i++)
//    {
//        qDebug() << SB_DEBUG_INFO << i << playList[i];
//    }
}

void
SBTabCurrentPlaylist::tableViewCellClicked(QModelIndex idx)
{
    qDebug() << SB_DEBUG_INFO << idx.column() << idx.row();
    if((SBCurrentPlaylistModel::sb_column_type)idx.column()==SBCurrentPlaylistModel::sb_column_playlistid)
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
SBTabCurrentPlaylist::tableViewCellDoubleClicked(QModelIndex idx)
{
    qDebug() << SB_DEBUG_INFO << idx.row();
    //	CWIP:PLAY
    //Context::instance()->getPlayerController()->playerPlayNow(idx.row());
}

///	Private methods

void
SBTabCurrentPlaylist::init()
{
    _pm=NULL;
    _playlistLoadedFlag=0;
    if(_initDoneFlag==0)
    {
        MainWindow* mw=Context::instance()->getMainWindow();
        QTableView* tv=mw->ui.currentPlaylistDetailSongList;

        //	Actions on tableview
        connect(tv, SIGNAL(clicked(QModelIndex)),
                this, SLOT(tableViewCellClicked(QModelIndex)));
        connect(tv,SIGNAL(doubleClicked(QModelIndex)),
                this, SLOT(tableViewCellDoubleClicked(QModelIndex)));

        //	Buttons
        connect(mw->ui.pbClearPlaylist, SIGNAL(clicked(bool)),
                this, SLOT(clearPlaylist()));
        connect(mw->ui.pbShufflePlaylist, SIGNAL(clicked(bool)),
                this, SLOT(shufflePlaylist()));
        connect(mw->ui.pbStartRadio, SIGNAL(clicked(bool)),
                this, SLOT(startRadio()));

        //	If playerController changes song, we want to update our view.
        connect(Context::instance()->getPlayerController(),SIGNAL(songChanged(int)),
                this, SLOT(songChanged(int)));

        //	Context menu
        tv->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(tv, SIGNAL(customContextMenuRequested(const QPoint&)),
                this, SLOT(showContextMenuPlaylist(QPoint)));

        //	Delete playlist
        deletePlaylistItemAction = new QAction(tr("Delete Item From Playlist "), this);
        deletePlaylistItemAction->setStatusTip(tr("Delete Item From Playlist"));
        connect(deletePlaylistItemAction, SIGNAL(triggered()),
                this, SLOT(deletePlaylistItem()));

        //	Play song now
        playSongNowAction = new QAction(tr("Play Song"), this);
        playSongNowAction->setStatusTip(tr("Play Song"));
        connect(playSongNowAction, SIGNAL(triggered(bool)),
                this, SLOT(playSong()));

        _initDoneFlag=1;
    }
}


//	Due to the nature of drag/drop, this view differs from others.
SBID
SBTabCurrentPlaylist::getSBIDSelected(const QModelIndex &idx)
{
    qDebug() << SB_DEBUG_INFO << idx;
    SBID id;

    MainWindow* mw=Context::instance()->getMainWindow();
    QTableView* tv=mw->ui.currentPlaylistDetailSongList;
    SBCurrentPlaylistModel* aem=dynamic_cast<SBCurrentPlaylistModel *>(tv->model());
    SBID::sb_type itemType=SBID::sb_type_invalid;
    QStandardItem* item;
    QStandardItem* i2;
    int itemID=-1;

    qDebug() << SB_DEBUG_INFO << "showing 1st 5 items";
    for(int i=0;i<5;i++)
    {
        item=aem->item(i,SBCurrentPlaylistModel::sb_column_songtitle);
        i2=aem->item(i,SBCurrentPlaylistModel::sb_column_songid);
        qDebug() << SB_DEBUG_INFO << i << item->text() << i2->text();
    }
    qDebug() << SB_DEBUG_INFO << "end:showing 1st 5 items";


    switch((SBCurrentPlaylistModel::sb_column_type)idx.column())
    {
    case SBCurrentPlaylistModel::sb_column_deleteflag:
    case SBCurrentPlaylistModel::sb_column_albumid:
    case SBCurrentPlaylistModel::sb_column_displayplaylistid:
    case SBCurrentPlaylistModel::sb_column_songid:
    case SBCurrentPlaylistModel::sb_column_performerid:
    case SBCurrentPlaylistModel::sb_column_playlistid:
    case SBCurrentPlaylistModel::sb_column_position:
    case SBCurrentPlaylistModel::sb_column_path:
        break;

    case SBCurrentPlaylistModel::sb_column_songtitle:
    case SBCurrentPlaylistModel::sb_column_duration:
        itemType=SBID::sb_type_song;
        item=aem->item(idx.row(),SBCurrentPlaylistModel::sb_column_songid);
        itemID=(item!=NULL)?item->text().toInt():-1;
        break;

    case SBCurrentPlaylistModel::sb_column_performername:
        itemType=SBID::sb_type_performer;
        item=aem->item(idx.row(),SBCurrentPlaylistModel::sb_column_performerid);
        itemID=(item!=NULL)?item->text().toInt():-1;
        break;

    case SBCurrentPlaylistModel::sb_column_albumtitle:
        itemType=SBID::sb_type_album;
        item=aem->item(idx.row(),SBCurrentPlaylistModel::sb_column_albumid);
        itemID=(item!=NULL)?item->text().toInt():-1;
        break;

    }

    id.assign(itemType,itemID);
    qDebug() << SB_DEBUG_INFO << id;
    return id;
}

///
/// \brief SBTabCurrentPlaylist::_populate
/// \param id
/// \return
///
/// Populates a regular playlist from database
///
SBID
SBTabCurrentPlaylist::_populate(const SBID& id)
{
    Q_UNUSED(id);
    qDebug() << SB_DEBUG_INFO;
    init();
    const MainWindow* mw=Context::instance()->getMainWindow();
    QTableView* tv=mw->ui.currentPlaylistDetailSongList;
    PlayerController* pc=Context::instance()->getPlayerController();

    //	Populate playlist

    SBCurrentPlaylistModel* plm=dynamic_cast<SBCurrentPlaylistModel *>(tv->model());
    if(plm==NULL)
    {
        QMap<int,SBID> playList;

        plm=new SBCurrentPlaylistModel();
        playList=plm->populate();
        tv->setModel(plm);
        pc->loadPlaylist(playList);
    }

    _playlistLoadedFlag=1;
    return SBID(SBID::sb_type_current_playlist,-1);
}

void
SBTabCurrentPlaylist::_populatePost(const SBID &id)
{
    Q_UNUSED(id);
    const MainWindow* mw=Context::instance()->getMainWindow();
    QTableView* tv=mw->ui.currentPlaylistDetailSongList;

    //	2.	Set up view
    tv->setSelectionMode(QAbstractItemView::ExtendedSelection);
    tv->setDragEnabled(1);
    tv->setAcceptDrops(1);
    tv->viewport()->setAcceptDrops(1);
    tv->setDropIndicatorShown(1);
    tv->setDragDropMode(QAbstractItemView::InternalMove);
    tv->setDefaultDropAction(Qt::MoveAction);
    tv->setDragDropOverwriteMode(false);
    tv->setColumnHidden(0,1);	//	sb_column_deleteflag
    tv->setColumnHidden(1,1);	//	sb_column_albumid
    tv->setColumnHidden(3,1);	//	sb_column_songid
    tv->setColumnHidden(4,1);	//	sb_column_performerid
    tv->setColumnHidden(5,1);	//	sb_column_playlistid
    tv->setColumnHidden(6,1);	//	sb_column_position
    tv->setColumnHidden(7,1);	//	sb_column_path

    QHeaderView* hv=NULL;
    hv=tv->horizontalHeader();
    hv->setSectionResizeMode(QHeaderView::ResizeToContents);
    hv->setStretchLastSection(1);

    hv=tv->verticalHeader();
    hv->hide();
    hv->setDefaultSectionSize(18);

    tv->setEditTriggers(QAbstractItemView::AllEditTriggers);
}
