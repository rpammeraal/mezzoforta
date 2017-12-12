#include "QProgressDialog"

#include "PlayManager.h"

#include "Context.h"
#include "Controller.h"
#include "MainWindow.h"
#include "Navigator.h"
#include "PlayerController.h"
#include "SBIDOnlinePerformance.h"
#include "SBMessageBox.h"
#include "SBModelQueuedSongs.h"
#include "SBSqlQueryModel.h"

///	Public methods
PlayManager::PlayManager(QObject *parent) : QObject(parent)
{
}

bool
PlayManager::songPlayingFlag() const
{
    PlayerController* pc=Context::instance()->getPlayerController();
    PlayerController::sb_player_state currentPlayState=pc?pc->playState():PlayerController::sb_player_state_stopped;
    return currentPlayState==PlayerController::sb_player_state_play?1:0;
}

///	Public slots:
void
PlayManager::playerPrevious()
{
    qDebug() << SB_DEBUG_INFO << "Calling playerNext()";
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
    qDebug() << SB_DEBUG_INFO;
    PlayerController* pc=Context::instance()->getPlayerController();
    SBModelQueuedSongs* mqs=Context::instance()->getSBModelQueuedSongs();
    int numSongs=mqs?mqs->numSongs():0;
    int numTries=numSongs;
    bool isPlayingFlag=0;
    bool exitLoopFlag=0;	//	meta indicator to avoid infinite loops
    bool lastSongPlayedFlag=0;

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
    lastSongPlayedFlag=(numSongs-currentPlayID()-1)==0;

    qDebug() << SB_DEBUG_INFO;
    pc->playerStop();
    while((numTries>0 && isPlayingFlag==0 && exitLoopFlag==0) || (lastSongPlayedFlag==1 && radioModeFlag()))
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
    qDebug() << SB_DEBUG_INFO;
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
    qDebug() << SB_DEBUG_INFO;
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
qDebug() << SB_DEBUG_INFO << "Calling playerNext()";
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
    bool isPlayingFlag=0;
    _setCurrentPlayID(playlistIndex);

    SBIDOnlinePerformancePtr performancePtr=_performanceAt(currentPlayID());

    if(!performancePtr)
    {
        qDebug() << SB_DEBUG_INFO << "returning 0";
        return 0;
    }
    else
    {
        //	Song is valid, go and play
        performancePtr->setPlayPosition(this->currentPlayID());
        isPlayingFlag=pc->playSong(performancePtr);
        if(isPlayingFlag==0)
        {
            qDebug() << SB_DEBUG_INFO << "returning 0";
            return 0;
        }
        else if(_radioModeFlag)
        {
            performancePtr->updateLastPlayDate();

        }
    }
    qDebug() << SB_DEBUG_INFO << isPlayingFlag;
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
    qDebug() << SB_DEBUG_INFO;
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
    const int numberSongsToDisplay=100;
    _radioModeFlag=1;

    QMap<int,SBIDOnlinePerformancePtr> playList;
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

    SBSqlQueryModel* qm=SBIDOnlinePerformance::retrieveAllOnlinePerformances(0);
    pd.setValue(++progressStep);
    QCoreApplication::processEvents();

    int numPerformances=qm->rowCount();
    if(numPerformances>numberSongsToDisplay)
    {
        //	DataEntityCurrentPlaylist::getAllOnlineSongs() may return more than 100,
        //	limit this to a 100 to make the view not too large.
        numPerformances=numberSongsToDisplay;
    }
    const int maxNumberAttempts=numberSongsToDisplay/2;
    int maxNumberToRandomize=qm->rowCount();
    //	If collection greater than 400, limit to 1st third of least recent played songs.
    maxNumberToRandomize=(maxNumberToRandomize>(4 * numberSongsToDisplay)?maxNumberToRandomize/3:maxNumberToRandomize);
    //	If greater than 5000, limit to 5000 to avoid long term starvation.
    maxNumberToRandomize=(maxNumberToRandomize>5000?5000:maxNumberToRandomize);

    int songInterval=numPerformances/10;

    bool found=1;
    int nextOpenSlotIndex=0;
    tqs->setViewLayout();
    int index=0;

    //	Find number of unplayed songs first.
    int nextPlayedSongID=0;
    while(qm->record(nextPlayedSongID++).value(1).toDate()==QDate(1900,1,1))
    {
    }
    qDebug() << SB_DEBUG_INFO << maxNumberToRandomize << nextPlayedSongID;

    while(index<numPerformances)
    {
        found=0;
        int idx=-1;

        if(index<nextPlayedSongID)
        {
            idx=index;
        }
        else
        {
            for(int j=maxNumberAttempts;j && !found;j--)
            {
                idx=Common::randomOldestFirst(maxNumberToRandomize);
                qDebug() << SB_DEBUG_INFO << idx;
                if(indexCovered.contains(idx)==0)
                {
                    found=1;
                    indexCovered.append(idx);
                }
            }
        }

        if(!found)
        {
            //	If we can't get a random index after n tries, get the first
            //	not-used index
            for(int i=0;i<numPerformances && found==0;i++)
            {
                if(indexCovered.contains(i)==0)
                {
                    idx=i;
                    found=1;
                    indexCovered.append(idx);
                }
            }
        }

        int onlinePerformanceID=qm->record(idx).value(0).toInt();
        SBIDOnlinePerformancePtr opPtr=SBIDOnlinePerformance::retrieveOnlinePerformance(onlinePerformanceID,1);

        playList[nextOpenSlotIndex++]=opPtr;

        if(index%songInterval==0 || index+1==numPerformances)
        {
            //	Update progress
            pd.setValue(++progressStep);
            QCoreApplication::processEvents();
        }

        //	Load the 1st n songs as soon as we get n songs or load the remainder after all songs are retrieved
        if(index+1==firstBatchNumber || index+1==numPerformances)
        {
            if(!firstBatchLoaded)
            {
                mqs->populate(playList);
                tqs->setViewLayout();

qDebug() << SB_DEBUG_INFO << "Calling playerNext()";
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

SBIDOnlinePerformancePtr
PlayManager::_performanceAt(int index) const
{
    SBModelQueuedSongs* mqs=Context::instance()->getSBModelQueuedSongs();
    return mqs?mqs->performanceAt(index):SBIDOnlinePerformancePtr();
}

void
PlayManager::_setCurrentPlayID(int currentPlayID)
{
    SBModelQueuedSongs* mqs=Context::instance()->getSBModelQueuedSongs();
    mqs->setCurrentPlayID(currentPlayID);
    _currentPlayID=currentPlayID;
    return;
}
