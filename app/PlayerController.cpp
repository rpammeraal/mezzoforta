#include "PlayerController.h"

#include <QPalette>

#include "Common.h"
#include "Context.h"
#include "Controller.h"
#include "MainWindow.h"
#include "Navigator.h"
#include "PlayManager.h"
#include "Properties.h"
#include "SBMediaPlayer.h"
#include "SBIDOnlinePerformance.h"


PlayerController::PlayerController(QObject *parent) : QObject(parent)
{
}

PlayerController::~PlayerController()
{
}

void
PlayerController::handlePlayingSong(int playerID, SBIDOnlinePerformancePtr opPtr)
{
    _playerPlayingID=playerID;

    PlayManager* pm=Context::instance()->playManager();
    qDebug() << SB_DEBUG_INFO << playerID << "About to call pm->handlePlayingSong()";
    pm->handlePlayingSong(opPtr);

    qDebug() << SB_DEBUG_INFO<< _currentPlayerID << opPtr->playPosition();
    emit setRowVisible(opPtr->playPosition());
}

QMediaPlayer::State
PlayerController::playState() const
{
    return _playerInstance[_currentPlayerID].state();
}

///	Public slots
void
PlayerController::playerRewind()
{
    qint64 position=_playerInstance[_currentPlayerID].position();
    position=position/1000-10;
    playerSeek(position);
}

void
PlayerController::playerForward()
{
    qint64 position=_playerInstance[_currentPlayerID].position();
    position=(position/1000)+10;
    playerSeek(position);
}

void
PlayerController::startNextSong(int currentPlayerID)
{
    //	Called then `currentPlayer` is about a second away from playing.
    //	Instruct the nextPlayer to play until currentPlayer is truly done.
    qDebug() << SB_DEBUG_INFO << currentPlayerID << "now playing" << _playerPlayingID;
    int nextPlayerID=_getNextPlayerID(currentPlayerID);
    qDebug() << SB_DEBUG_INFO << currentPlayerID << "about to play" << nextPlayerID;
    _playerInstance[nextPlayerID].playOnCue();
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
    _refreshPlayingNowData();	//	For whatever reason, data is hidden after link is clicked.
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
    if(_playerInstance[_currentPlayerID].state()==QMediaPlayer::PlayingState)
    {
        //	If playing, pause
        _playerInstance[_currentPlayerID].pause();
    }
    else if(_playerInstance[_currentPlayerID].state()==QMediaPlayer::PausedState)
    {
        //	If paused, resume play
        _playerInstance[_currentPlayerID].play();
    }
    //else if(_playerInstance[_currentPlayerID].state()==QMediaPlayer::StoppedState && _currentPerformancePlayingPtr)
    else if(_playerInstance[_currentPlayerID].state()==QMediaPlayer::StoppedState && _playerInstance[_currentPlayerID].getSBIDOnlinePerformancePtr())
    {
        //	If stopped and there is a valid song, play.
        _playerInstance[_currentPlayerID].play();
    }
    _refreshPlayingNowData();
    return 0;
}

void
PlayerController::playerSeek(int s)
{
    _playerInstance[_currentPlayerID].setPosition(s * 1000 );
}

void
PlayerController::playerStop()
{
    _refreshPlayingNowData();
    _playerInstance[_currentPlayerID].stop();
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
    qDebug() << SB_DEBUG_INFO << _currentPlayerID << "start";
    SB_RETURN_IF_NULL(opPtr,0);
    qDebug() << SB_DEBUG_INFO << _currentPlayerID << "path=" << opPtr->path();
    int result=this->_setupPlayer(_currentPlayerID, opPtr);
    qDebug() << SB_DEBUG_INFO << _currentPlayerID << "result=" << result;
    if(result)
    {

        //	Instruct player to play
        this->_startPlayer(_currentPlayerID);

        //	Load next song in alternate player
        this->_loadNextSong();

        qDebug() << SB_DEBUG_INFO << _currentPlayerID << "finish:ok";
        return 1;
    }
    qDebug() << SB_DEBUG_INFO << _currentPlayerID << "finish:error";
    return 0;
}

void
PlayerController::continueNextSong(int newCurrentPlayerID)
{
    qDebug() << SB_DEBUG_INFO << "_currentPlayerID=" << _currentPlayerID;
    qDebug() << SB_DEBUG_INFO << "newCurrentPlayerID=" << newCurrentPlayerID;

    _currentPlayerID=newCurrentPlayerID;

    SBIDOnlinePerformancePtr opPtr=_playerInstance[_currentPlayerID].getSBIDOnlinePerformancePtr();
    if(opPtr!=SBIDOnlinePerformancePtr())
    {
        qDebug() << SB_DEBUG_INFO << opPtr->playPosition();
        emit setRowVisible(opPtr->playPosition());	//	CWIP:	Handle from SBMediaPlayer
    }

    qDebug() << SB_DEBUG_INFO << _currentPlayerID << _playerInstance[_currentPlayerID].path();
    this->_makePlayerVisible(_currentPlayerID);
    this->_refreshPlayingNowData();

    this->_loadNextSong();
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
        connect(&(_playerInstance[i]), SIGNAL(prepareNextSong(int)),
            this, SLOT(startNextSong(int)));
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
    _makePlayerVisible(_currentPlayerID);

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
    _playerPlayingID=-1;
}

void
PlayerController::_loadNextSong()
{
    qDebug() << SB_DEBUG_INFO << _currentPlayerID << "start";

    PlayManager* pm=Context::instance()->playManager();
    SBIDOnlinePerformancePtr nextOpPtr=pm->getNextPlayItem();
    if(nextOpPtr!=nullptr)
    {
        qDebug() << SB_DEBUG_INFO << _currentPlayerID << "loading next:" << nextOpPtr->path();
        this->_setupPlayer(_getNextPlayerID(this->_currentPlayerID),nextOpPtr);
        nextOpPtr->resetPlayPosition();
    }
    qDebug() << SB_DEBUG_INFO << _currentPlayerID << "finish";
}

void
PlayerController::_makePlayerVisible(int playerID)
{
    _playerInstance[0].setPlayerVisible(playerID==0);
    _playerInstance[1].setPlayerVisible(playerID==1);
}

void
PlayerController::_refreshPlayingNowData() const
{
    qDebug() << SB_DEBUG_INFO;
    _playerInstance[_currentPlayerID].refreshPlayingNowData();
}

bool
PlayerController::_setupPlayer(int playerID, SBIDOnlinePerformancePtr opPtr)
{
    qDebug() << SB_DEBUG_INFO << playerID << "start";
    if(_playerInstance[playerID].setMedia(opPtr)==0)
    {
        QString errorMsg=_playerInstance[playerID].error();
        qDebug() << SB_DEBUG_ERROR << errorMsg;
        Controller* c=Context::instance()->controller();
        c->updateStatusBarText(errorMsg);
        _refreshPlayingNowData();
        opPtr->setErrorMessage(errorMsg);
        qDebug() << SB_DEBUG_INFO << playerID << "finish:error";
        return 0;
    }
    qDebug() << SB_DEBUG_INFO << playerID << "finish";
    return 1;
}

void
PlayerController::_startPlayer(int playerID)
{
    qDebug() << SB_DEBUG_INFO << "playerID=" << playerID;
    _playerInstance[playerID].play();
    _refreshPlayingNowData();
}
