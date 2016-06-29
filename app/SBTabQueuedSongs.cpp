#include "SBTabQueuedSongs.h"

#include <QProgressDialog>

#include "Common.h"
#include "Context.h"
#include "Controller.h"
#include "DataEntityAlbum.h"
#include "DataEntityCurrentPlaylist.h"
#include "DataEntityPerformer.h"
#include "DataEntityPlaylist.h"
#include "DataEntitySong.h"
#include "MainWindow.h"
#include "Navigator.h"
#include "PlayerController.h"
#include "SBModelQueuedSongs.h"
#include "SBSortFilterProxyQueuedSongsModel.h"
#include "SBSqlQueryModel.h"


SBTabQueuedSongs::SBTabQueuedSongs(QWidget* parent) : SBTab(parent,0)
{
    Context::instance()->setTabQueuedSongs(this);
}

void
SBTabQueuedSongs::playItemNow(const SBID &toPlay,const bool enqueueFlag)
{
    _init();
    PlayerController* pc=Context::instance()->getPlayerController();
    SBModelQueuedSongs* aem=model();
    _playingRadioFlag=0;
    QList<SBIDSong> songsInQueue;

    if(enqueueFlag==0)
    {
        this->clearPlaylist();
        pc->playerStop();
    }
    else
    {
        songsInQueue=aem->getAllSongs(); //	Get list of all songs currently in model
    }

    QMap<int,SBID> list;
    switch(toPlay.sb_item_type())
    {
    case SBID::sb_type_playlist:
        qDebug() << SB_DEBUG_INFO;
        list=_retrievePlaylistItems(toPlay);
        break;

    case SBID::sb_type_song:
        //	toPlay *must* have path populated.
        list[0]=toPlay;
        break;

    case SBID::sb_type_album:
        {
            SBSqlQueryModel* qm=DataEntityAlbum::getAllSongs(toPlay);
            for(int i=0;i<qm->rowCount();i++)
            {
                SBID song=SBID(SBID::sb_type_song,qm->data(qm->index(i,5)).toInt());
                song.sb_position=qm->data(qm->index(i,1)).toInt();
                song.songTitle=qm->data(qm->index(i,6)).toString();
                song.duration=qm->data(qm->index(i,7)).toTime();
                song.sb_performer_id=qm->data(qm->index(i,9)).toInt();
                song.performerName=qm->data(qm->index(i,10)).toString();
                song.path=qm->data(qm->index(i,13)).toString();
                song.sb_album_id=toPlay.sb_album_id;
                song.albumTitle=toPlay.albumTitle;
                list[list.count()]=song;

                qDebug() << SB_DEBUG_INFO << song.sb_song_id << song.sb_performer_id << song.sb_album_id << song.sb_position << song.albumTitle;
            }
        }
        break;

    case SBID::sb_type_performer:
        {
            DataEntityPerformer dep;
            SBSqlQueryModel* qm=dep.getAllOnlineSongs(toPlay);
            for(int i=0;i<qm->rowCount();i++)
            {
                SBID song=SBID(SBID::sb_type_song,qm->data(qm->index(i,0)).toInt());
                song.sb_performer_id=qm->data(qm->index(i,1)).toInt();
                song.sb_album_id=qm->data(qm->index(i,2)).toInt();
                song.sb_position=qm->data(qm->index(i,3)).toInt();
                song.songTitle=qm->data(qm->index(i,4)).toString();
                song.performerName=qm->data(qm->index(i,5)).toString();
                song.albumTitle=qm->data(qm->index(i,6)).toString();
                song.duration=qm->data(qm->index(i,7)).toTime();
                song.path=qm->data(qm->index(i,8)).toString();
                list[list.count()]=song;

                qDebug() << SB_DEBUG_INFO << song.sb_song_id << song.sb_performer_id << song.sb_album_id << song.sb_position << song.albumTitle;
            }
        }
        break;

    default:
        qDebug() << SB_DEBUG_ERROR << "playType" << toPlay.sb_item_type() << "not supported for " << toPlay;
    }

    qDebug() << SB_DEBUG_INFO << list.count();

    //	Check for dups
    QMap<int,SBID> toAdd;
    int j=0;
    for(int i=0;i<list.count();i++)
    {
        const SBIDSong id=SBIDSong(list[i]);
        if(songsInQueue.contains(id)==0)
        {
            toAdd[j++]=(SBID)(id);
            songsInQueue.append(id);
        }
        else
        {
            qDebug() << SB_DEBUG_INFO << "already exists" << id;
        }
    }

    //	Send to model
    aem->populate(toAdd,enqueueFlag);
    qDebug() << SB_DEBUG_INFO;
    this->_populatePost(toPlay);

    if(enqueueFlag==0)
    {
        bool isPlayingFlag=pc->playerPlayNonRadio(toPlay);
        if(isPlayingFlag==0)
        {
            pc->playerNext();
        }
    }
    qDebug() << SB_DEBUG_INFO;
    updateDetail();

    //	Update status bar if enqueued
    if(enqueueFlag)
    {
        Context::instance()->getController()->updateStatusBarText(QString("Queued %1 '%2'").arg(toPlay.getType()).arg(toPlay.getText()));
    }
}

SBModelQueuedSongs*
SBTabQueuedSongs::model() const
{
    SBSortFilterProxyQueuedSongsModel* sm=proxyModel();
    SBModelQueuedSongs* aem=dynamic_cast<SBModelQueuedSongs *>(sm->sourceModel());
    SB_RETURN_NULL_IF_NULL(aem);

    return aem;
}

SBSortFilterProxyQueuedSongsModel*
SBTabQueuedSongs::proxyModel() const
{
    MainWindow* mw=Context::instance()->getMainWindow();
    SB_RETURN_NULL_IF_NULL(mw);
    QTableView* tv=mw->ui.currentPlaylistDetailSongList;
    SB_RETURN_NULL_IF_NULL(tv);
    SBSortFilterProxyQueuedSongsModel* sm=dynamic_cast<SBSortFilterProxyQueuedSongsModel *>(tv->model());
    SB_RETURN_NULL_IF_NULL(sm);
    return sm;
}

QTableView*
SBTabQueuedSongs::subtabID2TableView(int subtabID) const
{
    Q_UNUSED(subtabID);
    const MainWindow* mw=Context::instance()->getMainWindow();
    return mw->ui.currentPlaylistDetailSongList;
}

///	Public slots
void
SBTabQueuedSongs::deletePlaylistItem()
{
    SBID assignID=this->model()->getSBIDSelected(_lastClickedIndex);
    if(assignID.sb_item_type()!=SBID::sb_type_invalid)
    {
        SBModelQueuedSongs* aem=model();
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
    updateDetail();
}

void
SBTabQueuedSongs::movePlaylistItem(const SBID& fromID, const SBID &toID)
{
    //	Determine current playlist
    SBID currentID=this->currentID();
    qDebug() << SB_DEBUG_INFO << currentID << fromID << toID;
    return;

    DataEntityPlaylist mpl;
    mpl.reorderItem(currentID,fromID,toID);
    refreshTabIfCurrent(currentID);
}

void
SBTabQueuedSongs::playNow(bool enqueueFlag)
{
    Q_UNUSED(enqueueFlag);
    qDebug() << SB_DEBUG_INFO << _lastClickedIndex;
    PlayerController* pc=Context::instance()->getPlayerController();
    SBSortFilterProxyQueuedSongsModel* sm=proxyModel();
    int viewPosition1=sm->mapFromSource(_lastClickedIndex).row();

    pc->playerStop();
    SBModelQueuedSongs* aem=model();
    aem->_populateMapPlaylistPosition2ViewPosition();
    aem->setCurrentSongByID(viewPosition1);

    bool isPlaying=0;
    isPlaying=pc->playerPlay();
    if(isPlaying==0)
    {
        pc->playerNext();
    }
    SBTab::playNow(enqueueFlag);
}

void
SBTabQueuedSongs::showContextMenuPlaylist(const QPoint &p)
{
    const MainWindow* mw=Context::instance()->getMainWindow();
    SBSortFilterProxyQueuedSongsModel* sm=proxyModel();
    QModelIndex idx=sm->mapToSource(mw->ui.currentPlaylistDetailSongList->indexAt(p));
    qDebug() << SB_DEBUG_INFO;

    SBID id=this->model()->getSBIDSelected(idx);
    qDebug() << SB_DEBUG_INFO << id;
    if(id.sb_item_type()!=SBID::sb_type_invalid)
    {
        _lastClickedIndex=idx;

        QPoint gp = mw->ui.currentPlaylistDetailSongList->mapToGlobal(p);

        QMenu menu(NULL);
        menu.addAction(_deletePlaylistAction);
        menu.addAction(_playNowAction);
        menu.exec(gp);
    }
}

void
SBTabQueuedSongs::songChanged(const SBID& song)
{
    qDebug() << SB_DEBUG_INFO << song << song.playPosition;
    MainWindow* mw=Context::instance()->getMainWindow();
    QTableView* tv=mw->ui.currentPlaylistDetailSongList;
    SBModelQueuedSongs* aem=model();
    QModelIndex idx=aem->setCurrentSongByID(song.playPosition);
    SBSortFilterProxyQueuedSongsModel* sm=proxyModel();
    idx=sm->mapFromSource(idx);
    qDebug() << SB_DEBUG_INFO << idx;
    tv->scrollTo(idx);
}

///	Private slots
void
SBTabQueuedSongs::clearPlaylist()
{
    qDebug() << SB_DEBUG_INFO;
    SBModelQueuedSongs* aem=model();
    if(aem)
    {
        aem->clear();
    }
    updateDetail();
}

void
SBTabQueuedSongs::shufflePlaylist()
{
    qDebug() << SB_DEBUG_INFO;
    MainWindow* mw=Context::instance()->getMainWindow();
    QTableView* tv=mw->ui.currentPlaylistDetailSongList;
    PlayerController* pc=Context::instance()->getPlayerController();
    SBModelQueuedSongs* aem=model();
    aem->shuffle(pc->radioPlayingFlag());
    tv->sortByColumn(SBModelQueuedSongs::sb_column_playlistpositionid,Qt::AscendingOrder);
    aem->repaintAll();
    _populatePost(SBID());
}

void
SBTabQueuedSongs::startRadio()
{
    _init();
    qDebug() << SB_DEBUG_INFO;
    SBModelQueuedSongs* aem=model();
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
    if(numSongs>100)
    {
        //	DataEntityCurrentPlaylist::getAllOnlineSongs() may return more than 100,
        //	limit this to a 100 to make the view not too large.
        numSongs=100;
    }
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
            idx=Common::randomOldestFirst(qm->rowCount());
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
SBTabQueuedSongs::updateDetail() const
{
    Duration totalDuration;
    QString detail;

    PlayerController* pc=Context::instance()->getPlayerController();
    if(pc && pc->radioPlayingFlag())
    {
        //	Don't update if radio is playing
        return;
    }
    qDebug() << SB_DEBUG_INFO;
    SBModelQueuedSongs* aem=model();
    if(aem)
    {
        qDebug() << SB_DEBUG_INFO;
        const int numSongs=aem->numSongs();
        if(numSongs)
        {
            Duration totalDuration=aem->totalDuration();
            qDebug() << SB_DEBUG_INFO << totalDuration.toString();

            detail+=QString("%1 song%2 %3 %4")
                    .arg(numSongs)
                    .arg(numSongs>1?"s":"")
                    .arg(QChar(8226))
                    .arg(totalDuration.toString())
            ;
        }
    }
    detail="<BODY BGCOLOR=\""+QString(SB_BG_COLOR)+"\">"+detail+"</BODY>";
    qDebug() << SB_DEBUG_INFO << detail;
    const MainWindow* mw=Context::instance()->getMainWindow();
    mw->ui.frCurrentPlaylistDetails->setText(detail);
}

void
SBTabQueuedSongs::tableViewCellClicked(QModelIndex idx)
{
    qDebug() << SB_DEBUG_INFO << idx.column() << idx.row();
    if((SBModelQueuedSongs::sb_column_type)idx.column()==SBModelQueuedSongs::sb_column_playlistpositionid)
    {
        qDebug() << SB_DEBUG_INFO;
    }
    else
    {
        SBSortFilterProxyQueuedSongsModel* sm=proxyModel();
        QModelIndex sourceIDX=sm->mapToSource(idx);
        SBID item=this->model()->getSBIDSelected(sourceIDX);
        qDebug() << SB_DEBUG_INFO << item;
        Context::instance()->getNavigator()->openScreenByID(item);
    }
}
void
SBTabQueuedSongs::tableViewCellDoubleClicked(QModelIndex idx)
{
    qDebug() << SB_DEBUG_INFO << idx.row();
}

///	Private methods

void
SBTabQueuedSongs::_init()
{
    qDebug() << SB_DEBUG_INFO;

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
        _deletePlaylistAction = new QAction(tr("Delete Item From Playlist "), this);
        _deletePlaylistAction->setStatusTip(tr("Delete Item From Playlist"));
        connect(_deletePlaylistAction, SIGNAL(triggered()),
                this, SLOT(deletePlaylistItem()));

        //	Play song now
        _playNowAction = new QAction(tr("Play Song"), this);
        _playNowAction->setStatusTip(tr("Play Song"));
        connect(_playNowAction, SIGNAL(triggered(bool)),
                this, SLOT(playNow()));

        //	Set up model
        QAbstractItemModel* m=tv->model();
        if(m==NULL)
        {
            PlayerController* pc=Context::instance()->getPlayerController();

            SB_DEBUG_IF_NULL(pc);
            SBModelQueuedSongs* aem=new SBModelQueuedSongs();
            SBSortFilterProxyQueuedSongsModel* sm=new SBSortFilterProxyQueuedSongsModel();

            sm->setSourceModel(aem);
            pc->setModelCurrentPlaylist(aem);
            tv->setModel(sm);
        }
        qDebug() << SB_DEBUG_INFO;

        _initDoneFlag=1;
    }
}


///
/// \brief SBTabQueuedSongs::_populate
/// \param id
/// \return
///
/// The _populate() method with other screens is used to actively populate a screen
/// with the specified SBID.
/// With SBTabQueuedSongs things are slightly different, as the playlist is
/// pre-populated in the database. Population happens at a different time from when
/// the current playlist (aka Songs in Queue) is opened.
///
SBID
SBTabQueuedSongs::_populate(const SBID& id)
{
    Q_UNUSED(id);
    _init();
    updateDetail();

    return SBID(SBID::sb_type_current_playlist,-1);
}

void
SBTabQueuedSongs::_populatePost(const SBID &id)
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
    hv->setSortIndicator(SBModelQueuedSongs::sb_column_displayplaylistpositionid,Qt::AscendingOrder);

    hv=tv->verticalHeader();
    hv->hide();
    hv->setDefaultSectionSize(18);

    tv->setEditTriggers(QAbstractItemView::AllEditTriggers);
}

QMap<int,SBID>
SBTabQueuedSongs::_retrievePlaylistItems(const SBID &id)
{
    QList<SBID> compositesTraversed;
    QList<SBID> allSongs;
    QMap<int,SBID> playList;

    //	Get all songs
    qDebug() << SB_DEBUG_INFO;
    compositesTraversed.clear();
    allSongs.clear();
    DataEntityPlaylist mpl;
    mpl.getAllItemsByPlaylistRecursive(compositesTraversed,allSongs,id);

    //	Populate playlist
    for(int i=0;i<allSongs.count();i++)
    {
        playList[i]=allSongs.at(i);
    }
    return playList;
}
