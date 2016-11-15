#include "PlayerController.h"

#include <QPalette>

#include "Common.h"
#include "Context.h"
#include "Controller.h"
#include "DataAccessLayer.h"
#include "MainWindow.h"
#include "Navigator.h"
#include "Properties.h"
#include "SBMediaPlayer.h"
#include "SBModelQueuedSongs.h"
#include "SBSqlQueryModel.h"


PlayerController::PlayerController(QObject *parent) : QObject(parent)
{
}

///	Public slots
void
PlayerController::playerRewind()
{
    qDebug() << SB_DEBUG_INFO << "**************************************";
    qDebug() << SB_DEBUG_INFO << "_state_=" << _state
    ;
    qint64 position=_playerInstance[_currentPlayerID].position();
    position=position/1000-10;
    playerSeek(position);
}

void
PlayerController::playerForward()
{
    qDebug() << SB_DEBUG_INFO << "**************************************";
    qDebug() << SB_DEBUG_INFO << "_state_=" << _state
    ;
    qint64 position=_playerInstance[_currentPlayerID].position();
    position=(position/1000)+10;
    playerSeek(position);
}

void
PlayerController::playerDurationChanged(quint64 durationMS)
{

    const int durationSec=durationMS/1000;

    _playerProgressSlider[_currentPlayerID]->setValue(0);
    _playerProgressSlider[_currentPlayerID]->setMaximum(durationSec);

    _durationTime[_currentPlayerID]=_ms2Duration(durationMS);
}

void
PlayerController::playerPositionChanged(quint64 durationMS)
{
    QString tStr;
    const int durationSec=durationMS/1000;

    Duration currentTime=_ms2Duration(durationMS);
    QString format = "mm:ss";
    if(_durationTime[_currentPlayerID].hour()>=1)
    {
        format = "hh:mm:ss";
    }
    tStr = currentTime.toString(Duration::sb_hhmmss_format) + " / " + _durationTime[_currentPlayerID].toString(Duration::sb_hhmmss_format);

    _playerProgressSlider[_currentPlayerID]->setValue(durationSec);
    _playerDurationLabel[_currentPlayerID]->setText(tStr);
}

void
PlayerController::playerStateChanged(QMediaPlayer::State playerState)
{
#ifdef Q_OS_WIN
    //	DO NOT REMOVE THIS!
    //	QT 5.7 in Windows will complain about QMediaPlayer::State
    //	not being registered and won't emit playNextSong() below.
    //	Of course, this is not a problem with OS X.
    qRegisterMetaType<QMediaPlayer::State>("whatever");
#endif

    if((_state==PlayerController::sb_player_state_changing_media) ||
            playerState==QMediaPlayer::PausedState ||
            playerState==QMediaPlayer::PlayingState)
    {
        return;
    }
    if(_state==PlayerController::sb_player_state_play)
    {
        //	Continue with next song
        emit playNextSong();
    }
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
    SBIDPtr ptr=SBIDBase::createPtr(static_cast<SBIDBase::sb_type>(l[0].toInt()),l[1].toInt());
    qDebug() << SB_DEBUG_INFO;
    Context::instance()->getNavigator()->openScreen(ptr);
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
    qDebug() << SB_DEBUG_INFO << "**************************************";
    qDebug() << SB_DEBUG_INFO << "_state_=" << _state
    ;

    //	Handle UI stuff
    if(_playerInstance[_currentPlayerID].state()==QMediaPlayer::PlayingState)
    {
        //	If playing, pause
        _playerInstance[_currentPlayerID].pause();
        _updatePlayState(PlayerController::sb_player_state_pause);
    }
    else if(_playerInstance[_currentPlayerID].state()==QMediaPlayer::PausedState)
    {
        //	If paused, resume play
        _playerInstance[_currentPlayerID].play();
        _updatePlayState(PlayerController::sb_player_state_play);
    }
    else if(_playerInstance[_currentPlayerID].state()==QMediaPlayer::StoppedState && _currentPerformancePlayingPtr)
    {
        //	If stopped and there is a valid song, play.
        _playerInstance[_currentPlayerID].play();
        _updatePlayState(PlayerController::sb_player_state_play);
    }
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
    qDebug() << SB_DEBUG_INFO << "**************************************";
    qDebug() << SB_DEBUG_INFO << "_state_=" << _state;
    _playerProgressSlider[_currentPlayerID]->setValue(0);
    _updatePlayState(PlayerController::sb_player_state_stopped);
    _playerInstance[_currentPlayerID].stop();
    playerSeek(0);
}

///
/// \brief PlayerController::playSong
/// \param playID
/// \return
///
/// Returns 1 on success, 0 otherwise.
bool
PlayerController::playSong(SBIDPerformancePtr& performancePtr)
{
    Properties* p=Context::instance()->getProperties();
    Controller* c=Context::instance()->getController();

    qDebug() << SB_DEBUG_INFO << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>";

    QString path=QString("%1/%2")
                .arg(p->musicLibraryDirectorySchema())
                .arg(performancePtr->path())
    ;
    emit setRowVisible(performancePtr->playPosition());	//	changed to here, so we can continue in case of error of playing a song.

    if(_playerInstance[_currentPlayerID].setMedia(path)==0)
    {
        QString errorMsg=_playerInstance[_currentPlayerID].error();
        c->updateStatusBarText(errorMsg);
        qDebug() << SB_DEBUG_ERROR << errorMsg;
        _updatePlayState(PlayerController::sb_player_state_stopped);
        performancePtr->setErrorMessage(errorMsg);
        qDebug() << SB_DEBUG_INFO << "returning 0";
        SBIDPerformancePtr null;
        _currentPerformancePlayingPtr=null;
        return 0;
    }

    //	Instruct player to play
    _currentPerformancePlayingPtr=performancePtr;
    _playerInstance[_currentPlayerID].play();
    _updatePlayState(PlayerController::sb_player_state_play);

    qDebug() << SB_DEBUG_INFO << "returning success";
    return 1;
}

///	Private methods
void
PlayerController::_init()
{
    _currentPlayerID=0;
    SBIDPerformancePtr null;
    _currentPerformancePlayingPtr=null;
    _state=PlayerController::sb_player_state_stopped;

    const MainWindow* mw=Context::instance()->getMainWindow();

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

    //	Get QFrame pointers
    _playerFrame[0]=mw->ui.frMusicPlayerLeft;
    _playerFrame[1]=mw->ui.frMusicPlayerRight;

    //	Get QPushButton pointers
    _playerPlayButton[0]=mw->ui.pbMusicPlayerControlLeftPLAY;
    _playerPlayButton[1]=mw->ui.pbMusicPlayerControlRightPLAY;

    //	Progress sliders
    _playerProgressSlider[0]=mw->ui.hsMusicPlayerProgressLeft;
    _playerProgressSlider[1]=mw->ui.hsMusicPlayerProgressRight;

    //	Duration label
    _playerDurationLabel[0]=mw->ui.lMusicPlayerDurationLeft;
    _playerDurationLabel[1]=mw->ui.lMusicPlayerDurationRight;

    //	Info label
    _playerDataLabel[0]=mw->ui.lMusicPlayerDataLeft;
    _playerDataLabel[1]=mw->ui.lMusicPlayerDataRight;

    //	Set left player visible
    _makePlayerVisible(PlayerController::sb_player_left);

    //	Instantiate media players
    for(int i=0; i<_maxPlayerID;i++)
    {
        _playerInstance[i].assignID(i);

        //	durationChanged
        connect(&_playerInstance[i],SIGNAL(durationChanged(quint64)),
                this, SLOT(playerDurationChanged(quint64)));

        //	positionChanged
        connect(&_playerInstance[i],SIGNAL(positionChanged(quint64)),
                this, SLOT(playerPositionChanged(quint64)));

        //	stateChanged
        connect(&_playerInstance[i],SIGNAL(stateChanged(QMediaPlayer::State)),
                this, SLOT(playerStateChanged(QMediaPlayer::State)));
    }

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
}

void
PlayerController::_makePlayerVisible(PlayerController::sb_player player)
{
    _playerFrame[0]->setVisible(player & PlayerController::sb_player_left);
    _playerFrame[1]->setVisible(player & PlayerController::sb_player_right);
}

Duration
PlayerController::_ms2Duration(quint64 ms) const
{
    const int s=ms/1000;
    const int seconds = (s) % 60;
    const int m=(s/60);
    const int minutes = (m) % 60;
    const int h=(m/24);
    const int hours = (h) % 24;

    return Duration(hours,minutes,seconds);
}

void
PlayerController::_refreshPlayingNowData() const
{
    QString playState;
    switch(_state)
    {
    case PlayerController::sb_player_state_stopped:
        _playerPlayButton[_currentPlayerID]->setText(">");
        playState="Stopped: ";
        break;

    case PlayerController::sb_player_state_play:
        playState="Now Playing: ";
        _playerPlayButton[_currentPlayerID]->setText("||");
        break;

    case PlayerController::sb_player_state_pause:
        _playerPlayButton[_currentPlayerID]->setText(">");
        playState="Paused: ";
        break;

    case PlayerController::sb_player_state_changing_media:
        //	ignore
        break;
    }

    playState="<BODY BGCOLOR=\"#f0f0f0\"><CENTER>"+playState;
    if(_state==PlayerController::sb_player_state_pause ||
       _state==PlayerController::sb_player_state_play)
    {
        playState+=QString(
            "<A HREF=\"%1_%2\">%3</A> by "
            "<A HREF=\"%4_%5\">%6</A> from the "
            "<A HREF=\"%7_%8\">'%9'</A> album")
            .arg(SBIDBase::sb_type_song)
            .arg(_currentPerformancePlayingPtr->songID())
            .arg(_currentPerformancePlayingPtr->songTitle())

            .arg(SBIDBase::sb_type_performer)
            .arg(_currentPerformancePlayingPtr->songPerformerID())
            .arg(_currentPerformancePlayingPtr->songPerformerName())

            .arg(SBIDBase::sb_type_album)
            .arg(_currentPerformancePlayingPtr->albumID())
            .arg(_currentPerformancePlayingPtr->albumTitle())
        ;
    }
    playState+="</CENTER></BODY>";
    _playerDataLabel[_currentPlayerID]->setText(playState);
}

void
PlayerController::_updatePlayState(PlayerController::sb_player_state newState)
{
    _state=newState;

    return _refreshPlayingNowData();
}
