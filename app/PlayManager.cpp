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
    QMediaPlayer::State currentPlayState=pc?pc->playState():QMediaPlayer::State::StoppedState;
    return currentPlayState==QMediaPlayer::State::PlayingState?1:0;
}

int
PlayManager::numSongs() const
{
    SBModelQueuedSongs* mqs=Context::instance()->sbModelQueuedSongs();
    return mqs?mqs->numSongs():0;
}

///	Public slots:
void
PlayManager::playerPrevious()
{
    qDebug() << SB_DEBUG_INFO << "start:Calling playerNext";
    playerNext(PlayMode::Previous);
    qDebug() << SB_DEBUG_INFO << "Finish";
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
    QMediaPlayer::State currentPlayState=pc?pc->playState():QMediaPlayer::State::StoppedState;

    switch(currentPlayState)
    {
    case QMediaPlayer::State::StoppedState:
        {
            SBModelQueuedSongs* mqs=Context::instance()->sbModelQueuedSongs();
            int numSongs=mqs?mqs->numSongs():0;
            if(numSongs==0)
            {
                startRadio();	//	No need to do anything else.
                return 1;
            }
            else
            {
                (pc?pc->playerPlay():0);
            }
        }
        break;

    case QMediaPlayer::State::PlayingState:
    case QMediaPlayer::State::PausedState:
        return (pc?pc->playerPlay():0);
        break;
    }
    return 0;
}

bool
PlayManager::playerNext(PlayManager::PlayMode playMode)
{
    qDebug() << SB_DEBUG_INFO << "Start:playMode=" << playMode;
    PlayerController* pc=Context::instance()->playerController();
    int numSongs=this->numSongs();
    int numTries=0;
    bool isPlayingFlag=0;
    bool exitLoopFlag=0;	//	meta indicator to avoid infinite loops
    bool lastSongPlayedFlag=0;

    //	Log if endOfSong
    qDebug() << SB_DEBUG_INFO;
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
    qDebug() << SB_DEBUG_INFO << numTries;
    if(numTries>5)
    {
        numTries=5;
    }
    lastSongPlayedFlag=(numSongs-currentPlayID()-1)==0;

    qDebug() << SB_DEBUG_INFO << this->currentPlayID();
    pc->playerStop();
    while((numTries>0 && isPlayingFlag==0 && exitLoopFlag==0) || (lastSongPlayedFlag==1 && radioModeFlag()))
    {
        int nextCurrentPlayID=(playMode==PlayMode::Previous)?currentPlayID()-1:currentPlayID()+1;
        qDebug() << SB_DEBUG_INFO << nextCurrentPlayID;

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
        qDebug() << SB_DEBUG_INFO << "Calling playItem2";
        isPlayingFlag=playItem(nextCurrentPlayID);
        qDebug() << SB_DEBUG_INFO << "isPlayingFlag" << isPlayingFlag;

        //	If previous and first song is not playing reverse directions
        if(isPlayingFlag==0)
        {
            if(playMode==PlayMode::Previous && nextCurrentPlayID==0)
            {
                qDebug() << SB_DEBUG_INFO;
                playMode=PlayMode::Default;
                numTries=numSongs;
            }
            if(playMode==PlayMode::Default && nextCurrentPlayID==numSongs-1)
            {
                qDebug() << SB_DEBUG_INFO;
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
    qDebug() << SB_DEBUG_INFO << "finish:isPlayingFlag" << isPlayingFlag;
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
        qDebug() << SB_DEBUG_INFO << "Calling playerNext";
        isPlayingFlag=this->playerNext();
    }
    else
    {
        if(!this->songPlayingFlag())
        {
            qDebug() << SB_DEBUG_INFO << "Calling playerNext";
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
    qDebug() << SB_DEBUG_INFO << "About to call _setCurrentPlayID";
    _setCurrentPlayID(newPlayID);
}

void
PlayManager::startRadio()
{
    _resetCurrentPlayID();
    _radioModeFlag=1;

    PlayerController* pc=Context::instance()->playerController();

    //	stop player
    pc->playerStop();

    //	load queue
    _loadRadio();
    emit playlistChanged(-1);

    //	show Songs in Queue tab
    Context::instance()->navigator()->showCurrentPlaylist();
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
/// ::playItem(unsigned int) is the lowest level function that will call PlayerController
/// to play a song.
bool
PlayManager::playItem(unsigned int playlistIndex)
{
    qDebug() << SB_DEBUG_INFO << "start:playlistIndex=" << playlistIndex;
    //	Check if music library directory is set up prior to playing.
    Context::instance()->properties()->musicLibraryDirectory();
    //qDebug() << SB_DEBUG_INFO;

    PlayerController* pc=Context::instance()->playerController();
    //qDebug() << SB_DEBUG_INFO;
    bool isPlayingFlag=0;

    int newPlayID=playlistIndex;	//	this->currentPlayID();
    //qDebug() << SB_DEBUG_INFO << newPlayID;
    if(newPlayID<0)
    {
        newPlayID=0;
    }
    qDebug() << SB_DEBUG_INFO << newPlayID;
    SBIDOnlinePerformancePtr opPtr=_performanceAt(newPlayID);
    SB_DEBUG_IF_NULL(opPtr);
    SB_RETURN_IF_NULL(opPtr,0);
    opPtr->setPlayPosition(playlistIndex);
    qDebug() << SB_DEBUG_INFO << opPtr->path();


    //	Song is valid, go and play
    qDebug() << SB_DEBUG_INFO << "Calling playSong";
    isPlayingFlag=pc->playSong(opPtr);
    qDebug() << SB_DEBUG_INFO << isPlayingFlag;


    return isPlayingFlag;
}

SBIDOnlinePerformancePtr
PlayManager::getNextPlayItem() const
{
    if(currentPlayID()+1>=this->numSongs())
    {
        return nullptr;
    }
    return _performanceAt(currentPlayID()+1);
}

//	Tell PlayManager to handle admin tasks
void
PlayManager::handlePlayingSong(SBIDOnlinePerformancePtr opPtr)
{
    qDebug() << SB_DEBUG_INFO << "start";
    qDebug() << SB_DEBUG_INFO << "currentPlayID=" << this->currentPlayID();
    qDebug() << SB_DEBUG_INFO << "_radioModeFlag=" << _radioModeFlag;

    //	Figure out playPosition.
    int newPlayID=-1;
    qDebug() << SB_DEBUG_INFO << opPtr->path();
    qDebug() << SB_DEBUG_INFO << opPtr->playPosition();
    if(opPtr->playPosition()!=-1)
    {
        newPlayID=opPtr->playPosition();
        qDebug() << SB_DEBUG_INFO << "opPtr:path=" << opPtr->path();
        qDebug() << SB_DEBUG_INFO << "using pre-assigned play position";
    }
    else
    {
        int startPlayID=(currentPlayID()-1<0?0:currentPlayID()-1);
        for(int i=startPlayID;newPlayID==-1 && i<=currentPlayID()+1;i++)
        {
            SBIDOnlinePerformancePtr currentOpPtr=_performanceAt(i);
            if(currentOpPtr!=SBIDOnlinePerformancePtr())
            {
                qDebug() << SB_DEBUG_INFO << currentOpPtr->path();

                if(currentOpPtr->key()==opPtr->key())
                {
                    newPlayID=i;
                    qDebug() << SB_DEBUG_INFO << "found:newPlayID=" << newPlayID;
                }
            }
        }
    }

    if(newPlayID==-1)
    {
        qDebug() << SB_DEBUG_ERROR << "Unable to determine newPlayID";
        return;
    }

    qDebug() << SB_DEBUG_INFO << "path=" << opPtr->path();
    qDebug() << SB_DEBUG_INFO << "newPlayID=" << newPlayID;
    emit setRowVisible(newPlayID);
    qDebug() << SB_DEBUG_INFO << "About to call _setCurrentPlayID";
    _setCurrentPlayID(newPlayID);
    opPtr->setPlayPosition(newPlayID);	//	this is the only place where playPosition should be set.

    qDebug() << SB_DEBUG_INFO << "opPtr=" << &(*opPtr);
    qDebug() << SB_DEBUG_INFO << "playPosition=" << opPtr->playPosition();
    if(_radioModeFlag)
    {
        opPtr->updateLastPlayDate();
    }

    Controller* c=Context::instance()->controller();
    SB_RETURN_VOID_IF_NULL(c);
    c->logSongPlayedHistory(_radioModeFlag,opPtr->key());
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

    //	Avoid having the same performer show up more than once.
    bool avoidDuplicatePerformer=1;			//	When we're loading unplayed songs
    bool selectingUnplayedSongsFirst=0;		//	this flag supersedes the selectingUnplayedSongsFirst
    int numberOfRejects=0;
    QSet<QString> performerInList;

    QString indexCovered=QString(".").repeated(maxNumberToRandomize+1);
    indexCovered+=QString("");
    while(index<numPerformances)
    {
        found=0;
        int idx=-1;

        if(index<nextPlayedSongID-1)
        {
            selectingUnplayedSongsFirst=1;
            idx=index;
        }
        else
        {
            selectingUnplayedSongsFirst=0;
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
        bool addToPlaylist=0;
        QString performerName=opPtr->songPerformerName();

        if (selectingUnplayedSongsFirst==1)
        {
            //	Adding unplayed songs or logic to avoid duplicate performers is turned off
            addToPlaylist=1;
        }
        else
        {
            if(avoidDuplicatePerformer==1)
            {
                //	Find out if we have performer already in list

                if(performerInList.contains(performerName))
                {
                    qDebug() << SB_DEBUG_WARNING << "Performer " << performerName << " already in playlist";
                    addToPlaylist=0;
                    numberOfRejects++;
                    if(numberOfRejects>numPerformances)
                    {
                        //	Turn off logic after n (numPerformances) tries.
                        avoidDuplicatePerformer=0;
                        qDebug() << SB_DEBUG_WARNING << "avoidDuplicatePerformer in playlist turned off";
                    }
                }
                else
                {
                    addToPlaylist=1;
                }
            }
            else
            {
                //	Logic is (already) turned off.
                addToPlaylist=1;
            }
        }

        if (addToPlaylist)
        {
            performerInList.insert(performerName);

            playList[nextOpenSlotIndex++]=opPtr;

            if(index==0 || songInterval==0 || index%songInterval==0 || index+1==numPerformances)
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

                    qDebug() << SB_DEBUG_INFO << "Calling playerNext";
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
    qDebug() << SB_DEBUG_INFO << "About to call _setCurrentPlayID";
    _setCurrentPlayID(-1);
}

SBIDOnlinePerformancePtr
PlayManager::_performanceAt(int index) const
{
    SBModelQueuedSongs* mqs=Context::instance()->sbModelQueuedSongs();
    return mqs?mqs->performanceAt(index):SBIDOnlinePerformancePtr();
}

void
PlayManager::_setCurrentPlayID(int newPlayID)
{
    qDebug() << SB_DEBUG_INFO << "*****************************************************************";
    qDebug() << SB_DEBUG_INFO << "old" << this->currentPlayID();
    qDebug() << SB_DEBUG_INFO << "new" << newPlayID;

    emit setRowVisible(newPlayID);

    if(newPlayID>=0)
    {
        SBModelQueuedSongs* mqs=Context::instance()->sbModelQueuedSongs();
        mqs->setCurrentPlayID(newPlayID);
        _currentPlayID=newPlayID;
    }
    return;
}
