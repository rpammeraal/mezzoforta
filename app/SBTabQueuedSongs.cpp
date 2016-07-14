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
#include "PlayManager.h"
#include "PlayerController.h"
#include "SBModelQueuedSongs.h"
#include "SBSortFilterProxyQueuedSongsModel.h"
#include "SBSqlQueryModel.h"


SBTabQueuedSongs::SBTabQueuedSongs(QWidget* parent) : SBTab(parent,0)
{
    Context::instance()->setTabQueuedSongs(this);
}

int
SBTabQueuedSongs::numSongsInPlaylist() const
{
    SBModelQueuedSongs* mqs=Context::instance()->getSBModelQueuedSongs();
    return mqs?mqs->rowCount():0;
}

SBSortFilterProxyQueuedSongsModel*
SBTabQueuedSongs::_proxyModel() const
{
    MainWindow* mw=Context::instance()->getMainWindow();
    QTableView* tv=mw->ui.currentPlaylistDetailSongList;
    SBSortFilterProxyQueuedSongsModel* sm=dynamic_cast<SBSortFilterProxyQueuedSongsModel *>(tv->model());
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
    SBModelQueuedSongs* mqs=Context::instance()->getSBModelQueuedSongs();
    SBID assignID=mqs->getSBIDSelected(_lastClickedIndex);
    if(assignID.sb_item_type()!=SBID::sb_type_invalid)
    {
        mqs->removeRows(_lastClickedIndex.row(),1,QModelIndex());

        QString updateText=QString("Removed %4 %1%2%3 from playlist.")
            .arg(QChar(96))            //	1
            .arg(assignID.getText())   //	2
            .arg(QChar(180))           //	3
            .arg(assignID.getType())   //	4
        ;
        Context::instance()->getController()->updateStatusBarText(updateText);
    }
    _updateDetail();
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
SBTabQueuedSongs::playlistChanged(const SBIDPlaylist &pl)
{
    Q_UNUSED(pl);
    _updateDetail();
}

void
SBTabQueuedSongs::playNow(bool enqueueFlag)
{
    SBSortFilterProxyQueuedSongsModel* sm=_proxyModel();
    int viewPosition1=sm->mapFromSource(_lastClickedIndex).row();
    PlayManager* pm=Context::instance()->getPlayManager();
    pm->playItemNow(viewPosition1);
    SBTab::playNow(enqueueFlag);
    return;
}

void
SBTabQueuedSongs::showContextMenuPlaylist(const QPoint &p)
{
    const MainWindow* mw=Context::instance()->getMainWindow();
    SBSortFilterProxyQueuedSongsModel* sm=_proxyModel();
    QModelIndex idx=sm->mapToSource(mw->ui.currentPlaylistDetailSongList->indexAt(p));

    SBModelQueuedSongs* mqs=Context::instance()->getSBModelQueuedSongs();
    SBID id=mqs->getSBIDSelected(idx);
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
SBTabQueuedSongs::songChanged(const SBIDSong& song)
{
    MainWindow* mw=Context::instance()->getMainWindow();
    SBModelQueuedSongs* mqs=Context::instance()->getSBModelQueuedSongs();
    QModelIndex idx=mqs->index(song.playPosition,SBModelQueuedSongs::sb_column_displayplaylistpositionid);
    SBSortFilterProxyQueuedSongsModel* sm=_proxyModel();
    idx=sm->mapFromSource(idx);
    QTableView* tv=mw->ui.currentPlaylistDetailSongList;
    tv->scrollTo(idx);
}

///	Protected slots
void
SBTabQueuedSongs::setViewLayout()
{
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

///	Private slots
void
SBTabQueuedSongs::tableViewCellClicked(QModelIndex idx)
{
    if((SBModelQueuedSongs::sb_column_type)idx.column()==SBModelQueuedSongs::sb_column_playlistpositionid)
    {
        qDebug() << SB_DEBUG_INFO;
    }
    else
    {
        SBSortFilterProxyQueuedSongsModel* sm=_proxyModel();
        QModelIndex sourceIDX=sm->mapToSource(idx);
        SBModelQueuedSongs* mqs=Context::instance()->getSBModelQueuedSongs();
        SBID item=mqs->getSBIDSelected(sourceIDX);
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
    _playingRadioFlag=0;
    if(_initDoneFlag==0)
    {
        MainWindow* mw=Context::instance()->getMainWindow();
        QTableView* tv=mw->ui.currentPlaylistDetailSongList;

        //	SBModelQueuedSongs
        SBModelQueuedSongs* mqs=Context::instance()->getSBModelQueuedSongs();
        connect(mqs, SIGNAL(listChanged()),
                this, SLOT(setViewLayout()));

        //	Actions on tableview
        connect(tv, SIGNAL(clicked(QModelIndex)),
                this, SLOT(tableViewCellClicked(QModelIndex)));
        connect(tv,SIGNAL(doubleClicked(QModelIndex)),
                this, SLOT(tableViewCellDoubleClicked(QModelIndex)));

        //	If playerController changes song, we want to update our view.
        connect(Context::instance()->getPlayerController(),SIGNAL(songChanged(const SBIDSong &)),
                this, SLOT(songChanged(const SBIDSong &)));

        //	If playManager changes playlist, we need to update the details.
        connect(Context::instance()->getPlayManager(),SIGNAL(playlistChanged(SBIDPlaylist)),
                this, SLOT(playlistChanged(SBIDPlaylist)));

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
            SBModelQueuedSongs* mqs=Context::instance()->getSBModelQueuedSongs();
            SBSortFilterProxyQueuedSongsModel* sm=new SBSortFilterProxyQueuedSongsModel();

            sm->setSourceModel(mqs);
            tv->setModel(sm);
        }

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
    _updateDetail();

    return SBID(SBID::sb_type_current_playlist,-1);
}

void
SBTabQueuedSongs::_populatePost(const SBID &id)
{
    Q_UNUSED(id);
    setViewLayout();
}

void
SBTabQueuedSongs::_updateDetail()
{
    QString detail;

    PlayManager* pm=Context::instance()->getPlayManager();
    if(pm->radioModeFlag())
    {
        //	Don't update if radio is playing
        detail="";
    }
    else
    {
        SBModelQueuedSongs* mqs=Context::instance()->getSBModelQueuedSongs();
        if(mqs)
        {
            const int numSongs=mqs->numSongs();
            if(numSongs)
            {
                Duration totalDuration=mqs->totalDuration();

                detail+=QString("%1 song%2 %3 %4")
                        .arg(numSongs)
                        .arg(numSongs>1?"s":"")
                        .arg(QChar(8226))
                        .arg(totalDuration.toString())
                ;
            }
        }
    }
    detail="<BODY BGCOLOR=\""+QString(SB_BG_COLOR)+"\">"+detail+"</BODY>";
    const MainWindow* mw=Context::instance()->getMainWindow();
    mw->ui.frCurrentPlaylistDetails->setText(detail);
}
