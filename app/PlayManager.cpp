#include "PlayManager.h"

#include "CacheManager.h"
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
    PlayerController* pc=Context::instance()->playerController();
    PlayerController::sb_player_state currentPlayState=pc?pc->playState():PlayerController::sb_player_state_stopped;
    return currentPlayState==PlayerController::sb_player_state_play?1:0;
}

///	Public slots:
void
PlayManager::playerPrevious()
{
    playerNext(PlayMode::Previous);
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
    PlayerController* pc=Context::instance()->playerController();
    PlayerController::sb_player_state currentPlayState=pc?pc->playState():PlayerController::sb_player_state_stopped;

    switch(currentPlayState)
    {
    case PlayerController::sb_player_state_stopped:
        {
            SBModelQueuedSongs* mqs=Context::instance()->sbModelQueuedSongs();
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
PlayManager::playerNext(PlayMode playMode)
{
    PlayerController* pc=Context::instance()->playerController();
    SBModelQueuedSongs* mqs=Context::instance()->sbModelQueuedSongs();
    int numSongs=mqs?mqs->numSongs():0;
    int numTries=numSongs;
    bool isPlayingFlag=0;
    bool exitLoopFlag=0;	//	meta indicator to avoid infinite loops
    bool lastSongPlayedFlag=0;

    //	Log if endOfSong
    if(playMode==PlayMode::Previous && currentPlayID()==0)
    {
        //	Skip to start of song if first song is active
        pc->playerSeek(0);
        return 0;
    }

    if(playMode==PlayMode::Previous)
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

    pc->playerStop();
    while((numTries>0 && isPlayingFlag==0 && exitLoopFlag==0) || (lastSongPlayedFlag==1 && radioModeFlag()))
    {
        int nextCurrentPlayID=(playMode==PlayMode::Previous)?currentPlayID()-1:currentPlayID()+1;

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
        isPlayingFlag=playItem(nextCurrentPlayID,playMode);

        //	If previous and first song is not playing reverse directions
        if(isPlayingFlag==0)
        {
            if(playMode==PlayMode::Previous && nextCurrentPlayID==0)
            {
                playMode=PlayMode::Default;
                numTries=numSongs;
            }
            if(playMode==PlayMode::Default && nextCurrentPlayID==numSongs-1)
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
    Context::instance()->playerController()->playerStop();
}

void
PlayManager::changeCurrentDatabaseSchema()
{
    this->clearPlaylist();
}

void
PlayManager::clearPlaylist()
{
    SBModelQueuedSongs* mqs=Context::instance()->sbModelQueuedSongs();
    mqs->clear();
    _resetCurrentPlayID();

    emit playlistChanged(-1);
}

bool
PlayManager::playItemNow(SBKey key, const bool enqueueFlag)
{
    bool isPlayingFlag=0;
    PlayerController* pc=Context::instance()->playerController();

    if(enqueueFlag==0)
    {
        this->clearPlaylist();
        pc->playerStop();
    }

    SBIDPtr ptr=CacheManager::get(key);
    SB_RETURN_IF_NULL(ptr,0);

    ptr->sendToPlayQueue(enqueueFlag);
    _radioModeFlag=0;
    if(ptr && ptr->itemType()==SBKey::Playlist && enqueueFlag==0)
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
    else
    {
        if(!this->songPlayingFlag())
        {
            this->playerNext(PlayMode::SetReady);
        }
    }
    return isPlayingFlag;
}

void
PlayManager::shufflePlaylist()
{
    SBModelQueuedSongs* mqs=Context::instance()->sbModelQueuedSongs();
    const int newPlayID=mqs->shuffle(1);	//	always leave played songs untouched.
    _setCurrentPlayID(newPlayID);
    emit setRowVisible(newPlayID+1);
}

void
PlayManager::startRadio()
{
    _resetCurrentPlayID();
    _radioModeFlag=1;

    qDebug() << SB_DEBUG_INFO << _radioModeFlag;

    PlayerController* pc=Context::instance()->playerController();

    //	stop player
    pc->playerStop();

    //	load queue
    _loadRadio();
    emit playlistChanged(-1);

    //	show Songs in Queue tab
    Context::instance()->navigator()->showCurrentPlaylist();
}

void
PlayManager::dummyPlayAllSongs()
{
    _radioModeFlag=1;

    PlayerController* pc=Context::instance()->playerController();

    int progressStep=0;
    ProgressDialog::instance()->startDialog(__SB_PRETTY_FUNCTION__,"Test All Song Paths",1);

    SBSqlQueryModel* qm=SBIDOnlinePerformance::retrieveAllOnlinePerformances(0,1);
    int numPerformances=qm->rowCount();
    ProgressDialog::instance()->update(__SB_PRETTY_FUNCTION__,"_dummyPlayAllSongs",0,numPerformances);

    int index=0;
    while(index<numPerformances)
    {
        int onlinePerformanceID=qm->record(index).value(0).toInt();
        SBIDOnlinePerformancePtr opPtr=SBIDOnlinePerformance::retrieveOnlinePerformance(onlinePerformanceID);

        if(pc->testSongFilepath(opPtr)==0)
        {
            qDebug() << SB_DEBUG_INFO << opPtr->path();
        }

        index++;
        if(index % 1000==0)
        {
            qDebug() << SB_DEBUG_INFO << "Processed " << index << " songs.";
            ProgressDialog::instance()->update(__SB_PRETTY_FUNCTION__,"_dummyPlayAllSongs",index,numPerformances);
        }
    }
    qDebug() << SB_DEBUG_INFO << "Processed " << index << " songs";

    ProgressDialog::instance()->finishStep(__SB_PRETTY_FUNCTION__,"_dummyPlayAllSongs");
    ProgressDialog::instance()->finishDialog(__SB_PRETTY_FUNCTION__);
}


///	Protected methods
void
PlayManager::doInit()
{
    _init();
}

///
/// \brief PlayManager::playItemNow
/// \param playlistIndex
/// \return
///
/// ::playItemNow(unsigned int) is the lowest level function that will call PlayerController
/// to play a song.
bool
PlayManager::playItem(unsigned int playlistIndex,PlayMode playMode)
{
    //	Check if music library directory is set up prior to playing.
    Context::instance()->properties()->musicLibraryDirectory();

    PlayerController* pc=Context::instance()->playerController();
    bool isPlayingFlag=0;
    _setCurrentPlayID(playlistIndex);

    SBIDOnlinePerformancePtr opPtr=_performanceAt(currentPlayID());

    SB_RETURN_IF_NULL(opPtr,0);

    //	Song is valid, go and play
    opPtr->setPlayPosition(this->currentPlayID());
    isPlayingFlag=pc->playSong(opPtr,playMode==PlayMode::SetReady);
    if(_radioModeFlag)
    {
        opPtr->updateLastPlayDate();
    }
    Controller* c=Context::instance()->controller();
    SB_RETURN_IF_NULL(c,0);

    c->logSongPlayedHistory(_radioModeFlag,opPtr->key());

    return isPlayingFlag;
}

///	Private methods
void
PlayManager::_init()
{
    _currentPlayID=-1;
    _radioModeFlag=0;

    const MainWindow* mw=Context::instance()->mainWindow();

    PlayerController* pc=Context::instance()->playerController();
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
    connect(Context::instance()->controller(), SIGNAL(databaseSchemaChanged()),
            this, SLOT(changeCurrentDatabaseSchema()));
}

void
PlayManager::_loadRadio()
{
    SBModelQueuedSongs* mqs=Context::instance()->sbModelQueuedSongs();
    SBTabQueuedSongs* tqs=Context::instance()->tabQueuedSongs();
    const int firstBatchNumber=5;
    bool firstBatchLoaded=false;
    const int numberSongsToDisplay=100;
    _radioModeFlag=1;

    QMap<int,SBIDOnlinePerformancePtr> playList;

    int progressStep=0;
    ProgressDialog::instance()->startDialog(__SB_PRETTY_FUNCTION__,"Starting Auto DJ",1);
    ProgressDialog::instance()->update(__SB_PRETTY_FUNCTION__,"_loadRadio",progressStep,11);

    SBSqlQueryModel* qm=SBIDOnlinePerformance::retrieveAllOnlinePerformances(0);
    ProgressDialog::instance()->update(__SB_PRETTY_FUNCTION__,"_loadRadio",++progressStep,11);

    int numPerformances=qm->rowCount();
    if(numPerformances>numberSongsToDisplay)
    {
        //	DataEntityCurrentPlaylist::getAllOnlineSongs() may return more than 100,
        //	limit this to a 100 to make the view not too large.
        numPerformances=numberSongsToDisplay;
    }
    //	const int maxNumberAttempts=numberSongsToDisplay/2;
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

    QString indexCovered=QString(".").repeated(maxNumberToRandomize+1);
    indexCovered+=QString("");
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
            int rnd=Common::randomOldestFirst(maxNumberToRandomize);

            //	Find first untaken spot, counting untaken spots.
            idx=0;
            for(int i=0;i<maxNumberToRandomize && !found;i++)
            {
                //	qDebug() << SB_DEBUG_INFO << index << indexCovered.left(100) << i << idx << rnd;
                //	QString ptr=QString("%1%2").arg(QString(".").repeated(i)).arg("^");
                //	qDebug() << SB_DEBUG_INFO << index << ptr.left(100);

                if(indexCovered.at(i)=='.')
                {
                    if(idx==rnd)
                    {
                        indexCovered.replace(i,1,'X');
                        found=1;
                    }
                    idx++;
                }
            }
        }


        int onlinePerformanceID=qm->record(idx).value(0).toInt();
        SBIDOnlinePerformancePtr opPtr=SBIDOnlinePerformance::retrieveOnlinePerformance(onlinePerformanceID);

        playList[nextOpenSlotIndex++]=opPtr;

        if(index%songInterval==0 || index+1==numPerformances)
        {
            //	Update progress
            ProgressDialog::instance()->update(__SB_PRETTY_FUNCTION__,"_loadRadio",++progressStep,11);
        }

        //	Load the 1st n songs as soon as we get n songs or load the remainder after all songs are retrieved
        if(index+1==firstBatchNumber || index+1==numPerformances)
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
    ProgressDialog::instance()->finishStep(__SB_PRETTY_FUNCTION__,"_loadRadio");
    ProgressDialog::instance()->finishDialog(__SB_PRETTY_FUNCTION__);
}

void
PlayManager::_resetCurrentPlayID()
{
    _setCurrentPlayID(-1);
}

SBIDOnlinePerformancePtr
PlayManager::_performanceAt(int index) const
{
    SBModelQueuedSongs* mqs=Context::instance()->sbModelQueuedSongs();
    return mqs?mqs->performanceAt(index):SBIDOnlinePerformancePtr();
}

void
PlayManager::_setCurrentPlayID(int currentPlayID)
{
    SBModelQueuedSongs* mqs=Context::instance()->sbModelQueuedSongs();
    mqs->setCurrentPlayID(currentPlayID);
    _currentPlayID=currentPlayID;
    return;
}
