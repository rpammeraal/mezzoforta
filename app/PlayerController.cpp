#include "PlayerController.h"

#include <QPalette>

#include "Common.h"
#include "Context.h"
#include "Controller.h"
#include "MainWindow.h"
#include "Navigator.h"
#include "PlayManager.h"
#include "SBMediaPlayer.h"
#include "SBIDOnlinePerformance.h"


PlayerController::PlayerController(QObject *parent) : QObject(parent)
{
}

PlayerController::~PlayerController()
{
}

QMediaPlayer::PlaybackState
PlayerController::playState() const
{
    return _playerInstance[getCurrentPlayerID()].state();
}

void
PlayerController::setPlayerFinished(int currentPlayerID)
{
    if(currentPlayerID==getPlayerPlayingID())
    {
        _setPlayerPlayingID(-1);
    }
    else
    {
        qDebug() << SB_DEBUG_ERROR << getCurrentPlayerID() << "attempt from player " << currentPlayerID << "to reset status";
    }
}

///	Public slots
void
PlayerController::playerRewind()
{
    qint64 position=_playerInstance[getCurrentPlayerID()].position();
    position=position/1000-10;
    playerSeek(position);
}

void
PlayerController::playerForward()
{
    qint64 position=_playerInstance[getCurrentPlayerID()].position();
    position=(position/1000)+10;
    playerSeek(position);
}

void
PlayerController::startNextPlayerOnCue(int currentPlayerID)
{
    //	Called then `currentPlayer` is about a second away from playing.
    //	Instruct the nextPlayer to play until currentPlayer is truly done.
    qDebug() << SB_DEBUG_INFO << currentPlayerID;
    int nextPlayerID=_getNextPlayerID(currentPlayerID);
    _playerInstance[nextPlayerID].playOnCue();
}

void
PlayerController::handleNeedMoreSongs()
{
    qDebug() << SB_DEBUG_INFO;
    _resetPlayers();	//	reset anyway.
    emit needMoreSongs();
}

void
PlayerController::handleReorderedPlaylist()
{
    qDebug() << SB_DEBUG_INFO;
    this->loadNextSong();
}

//	CWIP:
//	Finishing playlist is handled gracefully.
//	If playing radio, we'll need to continue
//	Next:
//	Start one song playing, queue next song.
//	When song is queued, PC needs to manage this next song to be loaded in 2nd player

void
PlayerController::loadNextSong()
{
    PlayManager* pm=Context::instance()->playManager();
    SBIDOnlinePerformancePtr nextOpPtr=pm->getNextPlayItem();

    int nextPlayerID=this->_getNextPlayerID(this->getCurrentPlayerID());
    if(nextOpPtr!=nullptr)
    {
        qDebug() << SB_DEBUG_INFO << getCurrentPlayerID() << "loading player" << nextPlayerID << "with" << nextOpPtr->path();
        this->_setupPlayer(nextPlayerID,nextOpPtr);
        nextOpPtr->resetPlayPosition();
    }
    else
    {
        qDebug() << SB_DEBUG_INFO << "releasing media on player" << nextPlayerID;
        _playerInstance[nextPlayerID].releaseMedia();
        qDebug() << SB_DEBUG_WARNING << getCurrentPlayerID() << "no next song";
    }
    qDebug() << SB_DEBUG_INFO << getCurrentPlayerID() << "ok";
}

void
PlayerController::processPlayerStarted(int playerID, SBIDOnlinePerformancePtr opPtr)
{
    qDebug() << SB_DEBUG_INFO << getCurrentPlayerID() << "playerID=" << playerID;
    _setCurrentPlayerID(playerID);
    _setPlayerPlayingID(playerID);

    PlayManager* pm=Context::instance()->playManager();
    pm->handlePlayingSong(opPtr);

    emit setRowVisible(opPtr->playPosition());

    loadNextSong();
    this->_makePlayerVisible(getCurrentPlayerID());
    qDebug() << SB_DEBUG_INFO << playerID << "ok";
}


///	Private slots

///
/// \brief PlayerController::playerDataClicked
/// \param url
///
/// Is called when an item in player is clicked.
void
PlayerController::playerDataClicked(const QUrl &url)
{
    QStringList l=url.toString().split('_');
    SBKey key=SBKey(static_cast<SBKey::ItemType>(l[0].toInt()),l[1].toInt());
    Context::instance()->navigator()->openScreen(key);
    //	For whatever reason, data is hidden after link is clicked.
    _playerInstance[getCurrentPlayerID()].refreshPlayingNowData();
}

///	Protected methods
void
PlayerController::doInit()
{
    _init();
}

///	Protected slots

///
/// \brief PlayerController::playerPlay
/// \param playID
/// \return
///
/// returns 1 on success, 0 on failure.
bool
PlayerController::playerPlay()
{
    //	Handle UI stuff
    if(_playerInstance[getCurrentPlayerID()].state()==QMediaPlayer::PlayingState)
    {
        //	If playing, pause
        _playerInstance[getCurrentPlayerID()].pause();
    }
    else if(_playerInstance[getCurrentPlayerID()].state()==QMediaPlayer::PausedState)
    {
        //	If paused, resume play
        _playerInstance[getCurrentPlayerID()].play();
    }
    //else if(_playerInstance[getCurrentPlayerID()].state()==QMediaPlayer::StoppedState && _currentPerformancePlayingPtr)
    else if(_playerInstance[getCurrentPlayerID()].state()==QMediaPlayer::StoppedState && _playerInstance[getCurrentPlayerID()].getSBIDOnlinePerformancePtr())
    {
        //	If stopped and there is a valid song, play.
        _playerInstance[getCurrentPlayerID()].play();
    }
    return 0;
}

void
PlayerController::playerSeek(int s)
{
    _playerInstance[getCurrentPlayerID()].setPosition(s * 1000 );
}

void
PlayerController::playerStop()
{
    _playerInstance[getCurrentPlayerID()].stop();
    playerSeek(0);
}

///
/// \brief PlayerController::playSong
/// \param playID
/// \return
///
/// Returns 1 on success, 0 otherwise.
/// TAG: path constructed
bool
PlayerController::playSong(SBIDOnlinePerformancePtr& opPtr)
{
    //	Called from PlayManager to start specific song.
    SB_RETURN_IF_NULL(opPtr,0);
    qDebug() << SB_DEBUG_INFO << getCurrentPlayerID() << "path=" << opPtr->path();
    int result=this->_setupPlayer(getCurrentPlayerID(), opPtr);
    if(result)
    {

        //	Instruct player to play
        this->_startPlayer(getCurrentPlayerID());

        qDebug() << SB_DEBUG_INFO << getCurrentPlayerID() << "ok";
        return 1;
    }
    qDebug() << SB_DEBUG_INFO << getCurrentPlayerID() << "finish:error";
    return 0;
}

void
PlayerController::explicitSetPlayerVisible(int playerID)
{
    this->_makePlayerVisible(playerID);
}

///	Private methods
void
PlayerController::_init()
{
    _currentPlayerID=0;

    const MainWindow* mw=Context::instance()->mainWindow();

    //	Left player
    connect(mw->ui.pbMusicPlayerControlLeftREW, SIGNAL(clicked(bool)),
            this, SLOT(playerRewind()));
    connect(mw->ui.pbMusicPlayerControlLeftSTOP, SIGNAL(clicked(bool)),
            this, SLOT(playerStop()));
    connect(mw->ui.pbMusicPlayerControlLeftFWD, SIGNAL(clicked(bool)),
            this, SLOT(playerForward()));
    connect(mw->ui.lMusicPlayerDataLeft, SIGNAL(anchorClicked(QUrl)),
            this, SLOT(playerDataClicked(QUrl)));

    connect(mw->ui.pbMusicPlayerControlRightREW, SIGNAL(clicked(bool)),
            this, SLOT(playerRewind()));
    connect(mw->ui.pbMusicPlayerControlRightSTOP, SIGNAL(clicked(bool)),
            this, SLOT(playerStop()));
    connect(mw->ui.pbMusicPlayerControlRightFWD, SIGNAL(clicked(bool)),
            this, SLOT(playerForward()));
    connect(mw->ui.lMusicPlayerDataRight, SIGNAL(anchorClicked(QUrl)),
            this, SLOT(playerDataClicked(QUrl)));

    for(int i=0;i<_maxPlayerID;i++)
    {
        connect(&(_playerInstance[i]), SIGNAL(prepareToStartNextSong(int)),
            this, SLOT(startNextPlayerOnCue(int)));
        connect(&(_playerInstance[i]), SIGNAL(needMoreSongs()),
                this, SLOT(handleNeedMoreSongs()));
        connect(&(_playerInstance[i]), SIGNAL(weArePlayingTime2LoadNextSong(int,SBIDOnlinePerformancePtr)),
            this, SLOT(processPlayerStarted(int,SBIDOnlinePerformancePtr)));
    }

    //	Get QFrame pointers
    _playerInstance[0].setPlayerFrame(mw->ui.frMusicPlayerLeft);
    _playerInstance[1].setPlayerFrame(mw->ui.frMusicPlayerRight);

    //	Get QPushButton pointers
    _playerInstance[0].setPlayerPlayButton(mw->ui.pbMusicPlayerControlLeftPLAY);
    _playerInstance[1].setPlayerPlayButton(mw->ui.pbMusicPlayerControlRightPLAY);

    //	Progress sliders
    _playerInstance[0].setProgressSlider(mw->ui.hsMusicPlayerProgressLeft);
    _playerInstance[1].setProgressSlider(mw->ui.hsMusicPlayerProgressRight);

    //	Duration label
    _playerInstance[0].setPlayerDurationLabel(mw->ui.lMusicPlayerDurationLeft);
    _playerInstance[1].setPlayerDurationLabel(mw->ui.lMusicPlayerDurationRight);

    //	Info label
    _playerInstance[0].setPlayerDataLabel(mw->ui.lMusicPlayerDataLeft);
    _playerInstance[1].setPlayerDataLabel(mw->ui.lMusicPlayerDataRight);

    //	Set left player visible
    _makePlayerVisible(getCurrentPlayerID());

    //	Instantiate media players
//    for(int i=0; i<_maxPlayerID;i++)
//    {
//        //	stateChanged
//        connect(&_playerInstance[i],SIGNAL(stateChanged(QMediaPlayer::State)),
//                this, SLOT(playerStateChanged(QMediaPlayer::State)));
//    }

    //	slider
    connect(mw->ui.hsMusicPlayerProgressLeft,SIGNAL(sliderMoved(int)),
            this, SLOT(playerSeek(int)));
    connect(mw->ui.hsMusicPlayerProgressRight,SIGNAL(sliderMoved(int)),
            this, SLOT(playerSeek(int)));
    connect(mw->ui.hsMusicPlayerProgressLeft,SIGNAL(valueChanged(int)),
            this, SLOT(playerSeek(int)));
    connect(mw->ui.hsMusicPlayerProgressRight,SIGNAL(valueChanged(int)),
            this, SLOT(playerSeek(int)));

    mw->ui.hsMusicPlayerProgressLeft->setTracking(1);
    mw->ui.hsMusicPlayerProgressRight->setTracking(1);

    for(int i=0;i<_maxPlayerID;i++)
    {
        _playerInstance[i].setPlayerID(i);
    }
    SBModelQueuedSongs* qs=Context::instance()->sbModelQueuedSongs();
    connect(qs, SIGNAL(listReordered()),
            this, SLOT(handleReorderedPlaylist()));
    _setPlayerPlayingID(-1);
}

void
PlayerController::_makePlayerVisible(int playerID)
{
    _playerInstance[0].setPlayerVisible(playerID==0);
    _playerInstance[1].setPlayerVisible(playerID==1);
}

void
PlayerController::_resetPlayers()
{
    qDebug() << SB_DEBUG_INFO;
    _setPlayerPlayingID(-1);

    const int defaultPlayerID=0;
    _makePlayerVisible(defaultPlayerID);
    _setCurrentPlayerID(defaultPlayerID);
    for(int i=0;i<_maxPlayerID;i++)
    {
        qDebug() << SB_DEBUG_INFO << i;
        _playerInstance[i].resetPlayer();
    }

    qDebug() << SB_DEBUG_INFO;
    _playerInstance[0].refreshPlayingNowData();
    qDebug() << SB_DEBUG_INFO;
}

void
PlayerController::_setCurrentPlayerID(int newPlayerID)
{
    if(newPlayerID<0 || newPlayerID>=_maxPlayerID)
    {
        qDebug() << SB_DEBUG_ERROR << "newPlayerID" << newPlayerID << "out of bounds";
        return;
    }
    _currentPlayerID=newPlayerID;
}

void
PlayerController::_setPlayerPlayingID(int newPlayerID)
{
    if(newPlayerID<-1 || newPlayerID>=_maxPlayerID)
    {
        qDebug() << SB_DEBUG_ERROR << "newPlayerID" << newPlayerID << "out of bounds";
        return;
    }
    _playerPlayingID=newPlayerID;
}

bool
PlayerController::_setupPlayer(int playerID, SBIDOnlinePerformancePtr opPtr)
{
    if(_playerInstance[playerID].setMedia(opPtr)==0)
    {
        QString errorMsg=_playerInstance[playerID].error();
        qDebug() << SB_DEBUG_ERROR << errorMsg;
        Controller* c=Context::instance()->controller();
        c->updateStatusBarText(errorMsg);
        opPtr->setErrorMessage(errorMsg);
        qDebug() << SB_DEBUG_INFO << playerID << "error";
        return 0;
    }
    qDebug() << SB_DEBUG_INFO << playerID << "ok";
    return 1;
}

void
PlayerController::_startPlayer(int playerID)
{
    qDebug() << SB_DEBUG_INFO << "playerID=" << playerID;
    qDebug() << SB_DEBUG_INFO << "_currentPlayerID=" << getCurrentPlayerID();
    _playerInstance[playerID].startPlay();
}
