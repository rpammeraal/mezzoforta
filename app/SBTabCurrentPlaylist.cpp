#include "SBTabCurrentPlaylist.h"

#include <QProgressDialog>

#include "Common.h"
#include "Context.h"
#include "Controller.h"
#include "DataEntityCurrentPlaylist.h"
#include "DataEntityPlaylist.h"
#include "DataEntitySong.h"
#include "MainWindow.h"
#include "Navigator.h"
#include "PlayerController.h"
#include "SBModelCurrentPlaylist.h"
#include "SBSqlQueryModel.h"


SBTabCurrentPlaylist::SBTabCurrentPlaylist(QWidget* parent) : SBTab(parent,0)
{
}

void
SBTabCurrentPlaylist::playPlaylist(const SBID &playlistID)
{
    _init();
    MainWindow* mw=Context::instance()->getMainWindow();
    QTableView* tv=mw->ui.currentPlaylistDetailSongList;
    PlayerController* pc=Context::instance()->getPlayerController();
    SBModelCurrentPlaylist* aem=dynamic_cast<SBModelCurrentPlaylist *>(tv->model());
    _playingRadioFlag=0;

    this->clearPlaylist();
    QList<SBID> compositesTraversed;
    QList<SBID> allSongs;
    QMap<int,SBID> playList;

    //	Get all songs
    qDebug() << SB_DEBUG_INFO;
    compositesTraversed.clear();
    allSongs.clear();
    DataEntityPlaylist mpl;
    mpl.getAllItemsByPlaylistRecursive(compositesTraversed,allSongs,playlistID);

    //	Populate playlist
    for(int i=0;i<allSongs.count();i++)
    {
        playList[i]=allSongs.at(i);
    }

    pc->playerStop();

    SB_DEBUG_IF_NULL(aem);
    aem->populate(playList);

    bool isPlayingFlag=pc->playerPlayInPlaylist(playlistID);
    if(isPlayingFlag==0)
    {
        pc->playerNext();
    }
}

void
SBTabCurrentPlaylist::enqueuePlaylist(const SBID &playlistID)
{
    MainWindow* mw=Context::instance()->getMainWindow();
    QTableView* tv=mw->ui.currentPlaylistDetailSongList;
    SBModelCurrentPlaylist* aem=dynamic_cast<SBModelCurrentPlaylist *>(tv->model());

    QList<SBID> allSongs=aem->getAllSongs(); //	Get list of all songs currently in model
    QList<SBID> compositesTraversed;
    QMap<int,SBID> playList;

    //	Get all songs
    qDebug() << SB_DEBUG_INFO << "currently in queue=" << allSongs.count();
    compositesTraversed.clear();
    allSongs.clear();
    DataEntityPlaylist mpl;
    mpl.getAllItemsByPlaylistRecursive(compositesTraversed,allSongs,playlistID);

    //	Populate playlist
    for(int i=0;i<allSongs.count();i++)
    {
        playList[i]=allSongs.at(i);
        qDebug() << SB_DEBUG_INFO << "Adding" << allSongs.at(i);
    }

    aem->populate(playList,1);
    this->_populatePost(playlistID);
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
        SBModelCurrentPlaylist* aem=dynamic_cast<SBModelCurrentPlaylist *>(tv->model());
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

    DataEntityPlaylist mpl;
    mpl.reorderItem(currentID,fromID,toID);
    refreshTabIfCurrent(currentID);
}

void
SBTabCurrentPlaylist::handleItemHighlight(QModelIndex &idx)
{
    qDebug() << SB_DEBUG_INFO << idx;
}

void
SBTabCurrentPlaylist::playSong()
{
    qDebug() << SB_DEBUG_INFO << _lastClickedIndex;
    PlayerController* pc=Context::instance()->getPlayerController();

    pc->playerStop();
    MainWindow* mw=Context::instance()->getMainWindow();
    QTableView* tv=mw->ui.currentPlaylistDetailSongList;
    SBModelCurrentPlaylist* aem=dynamic_cast<SBModelCurrentPlaylist *>(tv->model());
    aem->setCurrentSongByID(_lastClickedIndex.row());

    bool isPlaying=0;
    isPlaying=pc->playerPlay();
    if(isPlaying==0)
    {
        pc->playerNext();
    }
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
SBTabCurrentPlaylist::songChanged(const SBID& song)
{
    qDebug() << SB_DEBUG_INFO << song << song.playPosition;
    MainWindow* mw=Context::instance()->getMainWindow();
    QTableView* tv=mw->ui.currentPlaylistDetailSongList;
    SBModelCurrentPlaylist* aem=dynamic_cast<SBModelCurrentPlaylist *>(tv->model());
    QModelIndex idx=aem->setCurrentSongByID(song.playPosition);
    qDebug() << SB_DEBUG_INFO << idx;
    tv->scrollTo(idx);
}

///	Private slots
void
SBTabCurrentPlaylist::clearPlaylist()
{
    qDebug() << SB_DEBUG_INFO;
    MainWindow* mw=Context::instance()->getMainWindow();
    QTableView* tv=mw->ui.currentPlaylistDetailSongList;
    SBModelCurrentPlaylist* aem=dynamic_cast<SBModelCurrentPlaylist *>(tv->model());
    if(aem)
    {
        aem->clear();
    }
    //DataEntityCurrentPlaylist::clearPlaylist();
}

void
SBTabCurrentPlaylist::shufflePlaylist()
{
    qDebug() << SB_DEBUG_INFO;
    MainWindow* mw=Context::instance()->getMainWindow();
    QTableView* tv=mw->ui.currentPlaylistDetailSongList;
    SBModelCurrentPlaylist* aem=dynamic_cast<SBModelCurrentPlaylist *>(tv->model());
    aem->shuffle();
    tv->sortByColumn(SBModelCurrentPlaylist::sb_column_playlistpositionid,Qt::AscendingOrder);
    aem->repaintAll();
}

void
SBTabCurrentPlaylist::startRadio()
{
    _init();
    qDebug() << SB_DEBUG_INFO;
    MainWindow* mw=Context::instance()->getMainWindow();
    QTableView* tv=mw->ui.currentPlaylistDetailSongList;
    SBModelCurrentPlaylist* aem=dynamic_cast<SBModelCurrentPlaylist *>(tv->model());
    PlayerController* pc=Context::instance()->getPlayerController();
    const int firstBatchNumber=5;
    bool firstBatchLoaded=false;
    _playingRadioFlag=1;

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
    _populatePost(SBID());
    int index=0;
    while(index<numSongs)
    {
        qDebug() << SB_DEBUG_INFO << nextOpenSlotIndex;
        found=0;
        int idx=-1;

        for(int j=maxNumberAttempts;j && !found;j--)
        {
            idx=Common::randomOldestFirst(numSongs);
            if(indexCovered.contains(idx)==0)
            {
                found=1;
                indexCovered.append(idx);
                qDebug() << SB_DEBUG_INFO << "idx=" << idx << "not used yet";
            }
            else
            {
                qDebug() << SB_DEBUG_INFO << "idx=" << idx << "already used";
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
                    indexCovered.append(idx);
                    qDebug() << SB_DEBUG_INFO << "taking idx=" << idx;
                }
            }
        }

        qDebug() << SB_DEBUG_INFO << "random is " << idx;

        SBID item=SBID(SBID::sb_type_song,qm->record(idx).value(0).toInt());

        item.songTitle=qm->record(idx).value(1).toString();
        item.sb_performer_id=qm->record(idx).value(2).toInt();
        item.performerName=qm->record(idx).value(3).toString();
        item.sb_album_id=qm->record(idx).value(4).toInt();
        item.albumTitle=qm->record(idx).value(5).toString();
        item.sb_position=qm->record(idx).value(6).toInt();
        item.path=qm->record(idx).value(7).toString();
        item.duration=qm->record(idx).value(8).toTime();

        playList[nextOpenSlotIndex++]=item;
        qDebug() << SB_DEBUG_INFO << nextOpenSlotIndex-1 << playList[nextOpenSlotIndex-1];

        if(index%songInterval==0 || index+1==numSongs)
        {
            //	Update progress
            pd.setValue(++progressStep);
            QCoreApplication::processEvents();
        }

        //	Load the 1st n songs as soon as we get n songs or load the remainder after all songs are retrieved
        if(index+1==firstBatchNumber || index+1==numSongs)
        {
            if(!firstBatchLoaded)
            {
                qDebug() << SB_DEBUG_INFO << "sending to aem first batch:playList.count()=" << playList.count();
                SB_DEBUG_IF_NULL(aem);
                aem->populate(playList);
                this->_populatePost(SBID());

                pc->playerStop();
                bool isPlayingFlag=pc->playerPlayInRadio();
                if(isPlayingFlag==0)
                {
                    pc->playerNext();
                }

                firstBatchLoaded=true;

                //	Got the first batch loaded, clear playList and reset nextOpenSlotIndex
                playList.clear();
                nextOpenSlotIndex=0;
            }
            else
            {
                qDebug() << SB_DEBUG_INFO << "sending to aem 2nd batch:playList.count()=" << playList.count();
                aem->populate(playList,firstBatchLoaded);
                qDebug() << SB_DEBUG_INFO << "after sending to aem 2nd batch:playList.count()=" << playList.count();
            }
        }
        index++;
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

    qDebug() << SB_DEBUG_INFO << "contents playlist";
    for(int i=0;i<playList.count();i++)
    {
        qDebug() << SB_DEBUG_INFO << i << playList[i];
    }
}

void
SBTabCurrentPlaylist::tableViewCellClicked(QModelIndex idx)
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
SBTabCurrentPlaylist::tableViewCellDoubleClicked(QModelIndex idx)
{
    qDebug() << SB_DEBUG_INFO << idx.row();
}

///	Private methods

void
SBTabCurrentPlaylist::_init()
{
    qDebug() << SB_DEBUG_INFO;

    _pm=NULL;
    _playingRadioFlag=0;
    if(_initDoneFlag==0)
    {
        MainWindow* mw=Context::instance()->getMainWindow();
        QTableView* tv=mw->ui.currentPlaylistDetailSongList;

        //	Actions on tableview
        connect(tv, SIGNAL(clicked(QModelIndex)),
                this, SLOT(tableViewCellClicked(QModelIndex)));
        connect(tv,SIGNAL(doubleClicked(QModelIndex)),
                this, SLOT(tableViewCellDoubleClicked(QModelIndex)));
        connect(tv,SIGNAL(viewportEntered()),
                this, SLOT(handleItemHighlight(QModelIndex&)));

        //	Buttons
        connect(mw->ui.pbClearPlaylist, SIGNAL(clicked(bool)),
                this, SLOT(clearPlaylist()));
        connect(mw->ui.pbShufflePlaylist, SIGNAL(clicked(bool)),
                this, SLOT(shufflePlaylist()));
        connect(mw->ui.pbStartRadio, SIGNAL(clicked(bool)),
                this, SLOT(startRadio()));

        //	If playerController changes song, we want to update our view.
        connect(Context::instance()->getPlayerController(),SIGNAL(songChanged(const SBID &)),
                this, SLOT(songChanged(const SBID &)));

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

        //	Set up model
        SBModelCurrentPlaylist* aem=dynamic_cast<SBModelCurrentPlaylist *>(tv->model());
        if(aem==NULL)
        {
            qDebug() << SB_DEBUG_INFO;
            PlayerController* pc=Context::instance()->getPlayerController();

            SB_DEBUG_IF_NULL(pc);
            aem=new SBModelCurrentPlaylist();
            pc->setModelCurrentPlaylist(aem);
            tv->setModel(aem);
        }
        qDebug() << SB_DEBUG_INFO;

        _initDoneFlag=1;
    }
}


//	Due to the nature of drag/drop, this view differs from others.
SBID
SBTabCurrentPlaylist::getSBIDSelected(const QModelIndex &idx) const
{
    qDebug() << SB_DEBUG_INFO << idx;
    SBID id;

    MainWindow* mw=Context::instance()->getMainWindow();
    QTableView* tv=mw->ui.currentPlaylistDetailSongList;
    SBModelCurrentPlaylist* aem=dynamic_cast<SBModelCurrentPlaylist *>(tv->model());
    SBID::sb_type itemType=SBID::sb_type_invalid;
    QStandardItem* item;
    int itemID=-1;


    switch((SBModelCurrentPlaylist::sb_column_type)idx.column())
    {
    case SBModelCurrentPlaylist::sb_column_deleteflag:
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
/// The _populate() method with other screens is used to actively populate a screen
/// with the specified SBID.
/// With SBTabCurrentPlaylist things are slightly different, as the playlist is
/// pre-populated in the database. Population happens at a different time from when
/// the current playlist (aka Songs in Queue) is opened.
///
SBID
SBTabCurrentPlaylist::_populate(const SBID& id)
{
    Q_UNUSED(id);
    _init();
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

    //	3.	Set visibility
    tv->setColumnHidden(0,1);	//	sb_column_deleteflag
    tv->setColumnHidden(1,1);	//	sb_column_playflag
    tv->setColumnHidden(2,1);	//	sb_column_albumid
    tv->setColumnHidden(3,0);	//	sb_column_displayplaylistpositionid
    tv->setColumnHidden(4,1);	//	sb_column_songid
    tv->setColumnHidden(5,1);	//	sb_column_performerid
    tv->setColumnHidden(6,1);	//	sb_column_playlistpositionid
    tv->setColumnHidden(7,1);	//	sb_column_position
    tv->setColumnHidden(8,1);	//	sb_column_path

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
