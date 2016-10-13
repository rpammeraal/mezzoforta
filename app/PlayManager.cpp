#include "QProgressDialog"

#include "PlayManager.h"

#include "Context.h"
#include "Controller.h"
#include "MainWindow.h"
#include "Navigator.h"
#include "PlayerController.h"
#include "SBMessageBox.h"
#include "SBModelQueuedSongs.h"
#include "SBSqlQueryModel.h"

///	Public methods
PlayManager::PlayManager(QObject *parent) : QObject(parent)
{
}

///	Public slots:
void
PlayManager::playerPrevious()
{
    playerNext(1);
}

///
/// \brief PlayManager::playerPlay
/// \return
///
/// Handles play/pause button.
/// If playlist is empty, start radio.
/// If song is playing, delegate to PlayerController
bool
PlayManager::playerPlay()
{
    PlayerController* pc=Context::instance()->getPlayerController();
    PlayerController::sb_player_state currentPlayState=pc?pc->playState():PlayerController::sb_player_state_stopped;

    switch(currentPlayState)
    {
    case PlayerController::sb_player_state_stopped:
        {
            SBModelQueuedSongs* mqs=Context::instance()->getSBModelQueuedSongs();
            int numSongs=mqs?mqs->numSongs():0;
            if(numSongs==0)
            {
                startRadio();	//	No need to do anything else.
                return 1;
            }
        }

    case PlayerController::sb_player_state_play:
    case PlayerController::sb_player_state_pause:
        return (pc?pc->playerPlay():0);
        break;

    case PlayerController::sb_player_state_changing_media:
    default:
        break;
    }
    return 0;
}

bool
PlayManager::playerNext(bool previousFlag)
{
    PlayerController* pc=Context::instance()->getPlayerController();
    SBModelQueuedSongs* mqs=Context::instance()->getSBModelQueuedSongs();
    int numSongs=mqs?mqs->numSongs():0;
    int numTries=numSongs;
    bool isPlayingFlag=0;
    bool exitLoopFlag=0;	//	meta indicator to avoid infinite loops

    if(previousFlag && currentPlayID()==0)
    {
        //	Skip to start of song if first song is active
        pc->playerSeek(0);
        return 0;
    }

    if(previousFlag)
    {
        numTries=currentPlayID();
    }
    else
    {
        numTries=numSongs-currentPlayID()-1;
    }
    if(numTries>5)
    {
        numTries=5;
    }

    pc->playerStop();
    while(numTries>0 && isPlayingFlag==0 && exitLoopFlag==0)
    {
        int nextCurrentPlayID=previousFlag?currentPlayID()-1:currentPlayID()+1;

        //	Handle end of the list
        if(nextCurrentPlayID>=numSongs)
        {
            _resetCurrentPlayID();

            if(radioModeFlag())
            {
                //	CWIP: possible infinite loop radio restart
                //	Check with exitLoopFlag
                startRadio();
                return 0;
            }
        }
        isPlayingFlag=playItemNow(nextCurrentPlayID);

        //	If previous and first song is not playing reverse directions
        if(isPlayingFlag==0)
        {
            if(previousFlag==1 && nextCurrentPlayID==0)
            {
                previousFlag=0;
                numTries=numSongs;
            }
            if(previousFlag==0 && nextCurrentPlayID==numSongs-1)
            {
                //	We may not have any songs at all, exit loop
                exitLoopFlag=1;
            }
        }
        numTries--;
    }
    if(exitLoopFlag)
    {
        qDebug() << SB_DEBUG_ERROR << "No files found at all";
        SBMessageBox::createSBMessageBox("No playable songs were found.",
            "Is your music library set up correctly?",
            QMessageBox::Warning,
            QMessageBox::Ok,
            QMessageBox::Ok,
            QMessageBox::Ok,
            1);
    }
    return isPlayingFlag;
}

void
PlayManager::playerStop()
{
    Context::instance()->getPlayerController()->playerStop();
}

void
PlayManager::changeSchema()
{
    this->clearPlaylist();
}

void
PlayManager::clearPlaylist()
{
    SBModelQueuedSongs* mqs=Context::instance()->getSBModelQueuedSongs();
    mqs->clear();
    _resetCurrentPlayID();

    emit playlistChanged(-1);
}

bool
PlayManager::playItemNow(const SBIDPtr& ptr, const bool enqueueFlag)
{
    bool isPlayingFlag=0;
    PlayerController* pc=Context::instance()->getPlayerController();

    if(enqueueFlag==0)
    {
        this->clearPlaylist();
        pc->playerStop();
    }
    ptr->sendToPlayQueue(enqueueFlag);
    _radioModeFlag=0;
    if(ptr && ptr->itemType()==SBIDBase::sb_type_playlist && enqueueFlag==0)
    {
        emit playlistChanged(-1);
    }
    else
    {
        emit playlistChanged(-1);
    }

    if(enqueueFlag==0)
    {
        isPlayingFlag=this->playerNext();
    }
    return isPlayingFlag;
}

///
/// \brief PlayManager::playItemNow
/// \param playlistIndex
/// \return
///
/// ::playItemNow(unsigned int) is the lowest level function that will call PlayerController
/// to play a song.
bool
PlayManager::playItemNow(unsigned int playlistIndex)
{
    //	Check if music library directory is set up prior to playing.
    Context::instance()->getProperties()->musicLibraryDirectory();

    PlayerController* pc=Context::instance()->getPlayerController();
    QString errorMsg;
    bool isPlayingFlag=0;
    _setCurrentPlayID(playlistIndex);

    //	CWIP: change to SBIDSong
    SBIDSong song=_songAt(currentPlayID());

    if(song.itemType()==SBIDBase::sb_type_invalid)
    {
        errorMsg=song.errorMessage();
    }
    else
    {
        //	Song is valid, go and play
        song.setPlayPosition(this->currentPlayID());
        isPlayingFlag=pc->playSong(song);
        if(isPlayingFlag==0)
        {
            errorMsg=song.errorMessage();
        }
        else if(_radioModeFlag)
        {
            song.updateLastPlayDate();
        }
    }

    if(errorMsg.length()!=0)
    {
        qDebug() << SB_DEBUG_ERROR << song << errorMsg;
        SBMessageBox::createSBMessageBox("Error:",
            errorMsg,
            QMessageBox::Warning,
            QMessageBox::Ok,
            QMessageBox::Ok,
            QMessageBox::Ok,
            0);
    }
    return isPlayingFlag;
}

void
PlayManager::shufflePlaylist()
{
    SBModelQueuedSongs* mqs=Context::instance()->getSBModelQueuedSongs();
    const int newPlayID=mqs->shuffle(1);	//	always leave played songs untouched.
    _setCurrentPlayID(newPlayID);
    emit setRowVisible(newPlayID+1);
}

void
PlayManager::startRadio()
{
    _resetCurrentPlayID();
    _radioModeFlag=1;

    PlayerController* pc=Context::instance()->getPlayerController();

    //	stop player
    pc->playerStop();

    //	load queue
    _loadRadio();
    emit playlistChanged(-1);

    //	show Songs in Queue tab
    Context::instance()->getNavigator()->showCurrentPlaylist();
}

///	Protected methods
void
PlayManager::doInit()
{
    _init();
}

///	Private methods
void
PlayManager::_init()
{
    _currentPlayID=-1;
    _radioModeFlag=0;

    const MainWindow* mw=Context::instance()->getMainWindow();

    PlayerController* pc=Context::instance()->getPlayerController();
    connect(pc, SIGNAL(playNextSong()),
            this, SLOT(playerNext()));

    //	Player controls
    connect(mw->ui.pbStartRadio, SIGNAL(clicked(bool)),
            this, SLOT(startRadio()));
    connect(mw->ui.pbClearPlaylist, SIGNAL(clicked(bool)),
            this, SLOT(clearPlaylist()));
    connect(mw->ui.pbShufflePlaylist, SIGNAL(clicked(bool)),
            this, SLOT(shufflePlaylist()));

    //	Left player
    connect(mw->ui.pbMusicPlayerControlLeftPREV, SIGNAL(clicked(bool)),
            this, SLOT(playerPrevious()));
    connect(mw->ui.pbMusicPlayerControlLeftPLAY, SIGNAL(clicked(bool)),
            this, SLOT(playerPlay()));
    connect(mw->ui.pbMusicPlayerControlLeftNEXT, SIGNAL(clicked(bool)),
            this, SLOT(playerNext()));

    //	Right player
    connect(mw->ui.pbMusicPlayerControlRightPREV, SIGNAL(clicked(bool)),
            this, SLOT(playerPrevious()));
    connect(mw->ui.pbMusicPlayerControlRightPLAY, SIGNAL(clicked(bool)),
            this, SLOT(playerPlay()));
    connect(mw->ui.pbMusicPlayerControlRightNEXT, SIGNAL(clicked(bool)),
            this, SLOT(playerNext()));

    //	Schema changed
    connect(Context::instance()->getController(), SIGNAL(schemaChanged()),
            this, SLOT(changeSchema()));
}

void
PlayManager::_loadRadio()
{
    SBModelQueuedSongs* mqs=Context::instance()->getSBModelQueuedSongs();
    SBTabQueuedSongs* tqs=Context::instance()->getTabQueuedSongs();
    const int firstBatchNumber=5;
    bool firstBatchLoaded=false;
    _radioModeFlag=1;

    QMap<int,SBIDBase> playList;
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

    SBSqlQueryModel* qm=SBIDSong::getOnlineSongs(100);
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

    bool found=1;
    int nextOpenSlotIndex=0;
    tqs->setViewLayout();
    int index=0;
    while(index<numSongs)
    {
        found=0;
        int idx=-1;

        for(int j=maxNumberAttempts;j && !found;j--)
        {
            idx=Common::randomOldestFirst(qm->rowCount());
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
                    indexCovered.append(idx);
                }
            }
        }

        SBIDSong song=SBIDSong(qm->record(idx).value(0).toInt());

        song.setSongTitle(qm->record(idx).value(1).toString());
        song.setSongPerformerID(qm->record(idx).value(2).toInt());
        song.setSongPerformerName(qm->record(idx).value(3).toString());
        song.setAlbumID(qm->record(idx).value(4).toInt());
        song.setAlbumTitle(qm->record(idx).value(5).toString());
        song.setAlbumPosition(qm->record(idx).value(6).toInt());
        song.setPath(qm->record(idx).value(7).toString());
        song.setDuration(qm->record(idx).value(8).toTime());

        playList[nextOpenSlotIndex++]=song;

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
                mqs->populate(playList);
                tqs->setViewLayout();

                this->playerNext();
                emit playlistChanged(-1);

                firstBatchLoaded=true;

                //	Got the first batch loaded, clear playList and reset nextOpenSlotIndex
                playList.clear();
                nextOpenSlotIndex=0;
            }
            else
            {
                mqs->populate(playList,firstBatchLoaded);
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
}

void
PlayManager::_resetCurrentPlayID()
{
    _setCurrentPlayID(-1);
}

SBIDSong
PlayManager::_songAt(int index) const
{
    SBModelQueuedSongs* mqs=Context::instance()->getSBModelQueuedSongs();
    return mqs?mqs->songAt(index):SBIDBase();
}

void
PlayManager::_setCurrentPlayID(int currentPlayID)
{
    SBModelQueuedSongs* mqs=Context::instance()->getSBModelQueuedSongs();
    mqs->setCurrentPlayID(currentPlayID);
    _currentPlayID=currentPlayID;
    return;
}
