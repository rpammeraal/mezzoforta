#include "PlayerController.h"

#include <QPalette>

#include "Common.h"
#include "Context.h"
#include "MainWindow.h"
#include "Navigator.h"
#include "SBMediaPlayer.h"
#include "SBMessageBox.h"
#include "SBModelCurrentPlaylist.h"
#include "SBSqlQueryModel.h"


PlayerController::PlayerController(QObject *parent) : QObject(parent), _maxPlayerID(2)
{
    init();
}

void
PlayerController::initialize()
{
    qDebug() << SB_DEBUG_INFO;
    if(_initDoneFlag==0)
    {
        qDebug() << SB_DEBUG_INFO;

        const MainWindow* mw=Context::instance()->getMainWindow();
        _initDoneFlag=1;

        //	Left player
        connect(mw->ui.pbMusicPlayerControlLeftPREV, SIGNAL(clicked(bool)),
                this, SLOT(playerPrevious()));
        connect(mw->ui.pbMusicPlayerControlLeftREW, SIGNAL(clicked(bool)),
                this, SLOT(playerRewind()));
        connect(mw->ui.pbMusicPlayerControlLeftSTOP, SIGNAL(clicked(bool)),
                this, SLOT(playerStop()));
        connect(mw->ui.pbMusicPlayerControlLeftPLAY, SIGNAL(clicked(bool)),
                this, SLOT(playerPlay()));
        connect(mw->ui.pbMusicPlayerControlLeftFWD, SIGNAL(clicked(bool)),
                this, SLOT(playerForward()));
        connect(mw->ui.pbMusicPlayerControlLeftNEXT, SIGNAL(clicked(bool)),
                this, SLOT(playerNext()));
        connect(mw->ui.lMusicPlayerDataLeft, SIGNAL(anchorClicked(QUrl)),
                this, SLOT(playerDataClicked(QUrl)));

        //	Right player
        connect(mw->ui.pbMusicPlayerControlRightPREV, SIGNAL(clicked(bool)),
                this, SLOT(playerPrevious()));
        connect(mw->ui.pbMusicPlayerControlRightREW, SIGNAL(clicked(bool)),
                this, SLOT(playerRewind()));
        connect(mw->ui.pbMusicPlayerControlRightSTOP, SIGNAL(clicked(bool)),
                this, SLOT(playerStop()));
        connect(mw->ui.pbMusicPlayerControlRightPLAY, SIGNAL(clicked(bool)),
                this, SLOT(playerPlay()));
        connect(mw->ui.pbMusicPlayerControlRightFWD, SIGNAL(clicked(bool)),
                this, SLOT(playerForward()));
        connect(mw->ui.pbMusicPlayerControlRightNEXT, SIGNAL(clicked(bool)),
                this, SLOT(playerNext()));
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
        makePlayerVisible(PlayerController::sb_player_left);

        //	Instantiate media players
        for(int i=0; i<_maxPlayerID;i++)
        {
            _playerInstance[i].assignID(i);

            //	durationChanged
            connect(&_playerInstance[i],SIGNAL(durationChanged(qint64)),
                    this, SLOT(playerDurationChanged(qint64)));

            //	positionChanged
            connect(&_playerInstance[i],SIGNAL(positionChanged(qint64)),
                    this, SLOT(playerPositionChanged(qint64)));

            //	stateChanged
            connect(&_playerInstance[i],SIGNAL(stateChanged(QMediaPlayer::State)),
                    this, SLOT(playerStateChanged(QMediaPlayer::State)));
        }

        //	slider
        connect(mw->ui.hsMusicPlayerProgressLeft,SIGNAL(sliderMoved(int)),
                this, SLOT(playerSeek(int)));
        connect(mw->ui.hsMusicPlayerProgressRight,SIGNAL(sliderMoved(int)),
                this, SLOT(playerSeek(int)));
    }
}

///	Public slots
void
PlayerController::loadPlaylist(QMap<int,int> fromTo)
{
    const MainWindow* mw=Context::instance()->getMainWindow();
    SBTabCurrentPlaylist* cp=mw->ui.tabCurrentPlaylist;
    qDebug() << SB_DEBUG_INFO << "current before=" << _playList[_currentPlayID];
    if(fromTo.contains(_currentPlayID+1))
    {
        qDebug() << SB_DEBUG_INFO << "old _currentPlayID" << _currentPlayID;
        //_currentPlayID=fromTo[_currentPlayID];
        _updateCurrentPlayerID(fromTo[_currentPlayID+1]-1);

        qDebug() << SB_DEBUG_INFO << "new _currentPlayID" << _currentPlayID;
    }
    _playList=cp->playlist();
    qDebug() << SB_DEBUG_INFO << "current after=" << _playList[_currentPlayID];
}

void
PlayerController::playerPrevious()
{
    qDebug() << SB_DEBUG_INFO << "**************************************" << _state << _currentPlayID;

    _state = PlayerController::sb_player_state_changing_media;
    _seekPreviousSongFlag=1;
    _playerStop();
    calculateNextSongID();
    playerPlay();
    _state = PlayerController::sb_player_state_play;
    _seekPreviousSongFlag=0;
    qDebug() << SB_DEBUG_INFO;
}

void
PlayerController::playerRewind()
{
    qDebug() << SB_DEBUG_INFO << "**************************************" << _state << _currentPlayID;
    qint64 position=_playerInstance[_currentPlayerID].position();
    position=position/1000-10;
    playerSeek(position);
}

void
PlayerController::playerStop()
{
    qDebug() << SB_DEBUG_INFO << "**************************************" << _state << _currentPlayID;
    _state=PlayerController::sb_player_state_stopped;
    _playerStop();
    _playerPlayButton[_currentPlayerID]->setText(">");
    _updatePlayerInfo();
}

bool
PlayerController::playerPlay(int playID)
{
    qDebug() << SB_DEBUG_INFO << "**************************************" << _state << _currentPlayID << playID;
    if(_playerInstance[_currentPlayerID].state()==QMediaPlayer::PlayingState && playID==-1)
    {
        qDebug() << SB_DEBUG_INFO;
        //	If playing, pause
        _playerInstance[_currentPlayerID].pause();
        _playerPlayButton[_currentPlayerID]->setText(">");
        _state=PlayerController::sb_player_state_pause;
        _updatePlayerInfo();
    }
    else if(_playerInstance[_currentPlayerID].state()==QMediaPlayer::PausedState)
    {
        qDebug() << SB_DEBUG_INFO;
        //	If paused, resume play
        _playerInstance[_currentPlayerID].play();
        _playerPlayButton[_currentPlayerID]->setText("||");
        _state=PlayerController::sb_player_state_play;
        _updatePlayerInfo();
    }
    else if(_playerInstance[_currentPlayerID].state()==QMediaPlayer::StoppedState)
    {
        qDebug() << SB_DEBUG_INFO;
        if(_currentPlayID==-1 || _currentPlayID>=_playList.count())
        {
            qDebug() << SB_DEBUG_INFO << _playList.count();
            if(_playList.count()==0)
            {
                qDebug() << SB_DEBUG_INFO << "Attempt to load playlist";
                loadPlaylist();
            }
            if(_playList.count())
            {
                qDebug() << SB_DEBUG_INFO << "Reset playlist";
                //_currentPlayID=0;
                _updateCurrentPlayerID(0);
            }
            else
            {
                qDebug() << SB_DEBUG_INFO << "No playlist found";
                _state=PlayerController::sb_player_state_stopped;
                _playerPlayButton[_currentPlayerID]->setText(">");
                //_currentPlayID=-1;
                _updateCurrentPlayerID(-1);
                return 0;
            }
        }
        qDebug() << SB_DEBUG_INFO << _currentPlayID;

        if(_currentPlayID>=0)
        {
            if(playID!=-1)
            {
                //_currentPlayID=playID;
                _updateCurrentPlayerID(playID);
            }
            qDebug() << SB_DEBUG_INFO << _currentPlayID;

            while(_currentPlayID<_playList.count())
            {
                qDebug() << SB_DEBUG_INFO << _currentPlayID << _seekPreviousSongFlag;

                QString path="/Volumes/bigtmp/Users/roy/songbase/music/files/rock/"+_playList[_currentPlayID].path;
                if(_playerInstance[_currentPlayerID].setMedia(path)==0)
                {
                    qDebug() << SB_DEBUG_INFO << "Missing file";
                    _state=PlayerController::sb_player_state_stopped;
                    calculateNextSongID();
                }
                else
                {
                    qDebug() << SB_DEBUG_INFO;
                    //	Instruct player to play
                    _state=PlayerController::sb_player_state_play;
                    qDebug() << SB_DEBUG_INFO << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>";
                    _playerInstance[_currentPlayerID].play();
                    _playerPlayButton[_currentPlayerID]->setText("||");
                    _updatePlayerInfo();
                    return 1;
                }
            }
        }
    }
    return 0;
}

void
PlayerController::playerPlayNow(int playID)
{
    qDebug() << SB_DEBUG_INFO << "**************************************" << _state << _currentPlayID << playID;
    if(playID==_currentPlayID)
    {
        return;
    }
    if(_state!=PlayerController::sb_player_state_stopped)
    {
        int result=SBMessageBox::createSBMessageBox("Stop currently played song?",
                                                    "",
                                                    QMessageBox::Question,
                                                    QMessageBox::Yes | QMessageBox::No,
                                                    QMessageBox::No,
                                                    QMessageBox::No);
        if(result==QMessageBox::No)
        {
            return;
        }
    }
    _state=PlayerController::sb_player_state_changing_media;
    _playerStop();
    //_currentPlayID=playID;
    _updateCurrentPlayerID(playID);
    playerPlay(playID);
    _state = PlayerController::sb_player_state_play;
}

void
PlayerController::playerForward()
{
    qDebug() << SB_DEBUG_INFO << "**************************************" << _state << _currentPlayID;
    if(_currentPlayID==-1)
    {
    qDebug() << SB_DEBUG_INFO << "**************************************" << _state << _currentPlayID;
        return;
    }
    qDebug() << SB_DEBUG_INFO << "**************************************" << _state << _currentPlayID;
    qint64 position=_playerInstance[_currentPlayerID].position();
    position=position/1000+10;
    playerSeek(position);
}

void
PlayerController::playerNext()
{
    qDebug() << SB_DEBUG_INFO << "**************************************" << _state << _currentPlayID;
    _state = PlayerController::sb_player_state_changing_media;
    _playerStop();
    //_currentPlayID++;
    _updateCurrentPlayerID(_currentPlayID+1);
    playerPlay();
    _state = PlayerController::sb_player_state_play;
}

void
PlayerController::playerDurationChanged(qint64 durationMS)
{

    const int durationSec=durationMS/1000;
    qDebug() << SB_DEBUG_INFO << "ms:" << durationMS << "s:" << durationSec;

    _playerProgressSlider[_currentPlayerID]->setValue(0);
    _playerProgressSlider[_currentPlayerID]->setMaximum(durationSec);

    _durationTime[_currentPlayerID]=calculateTime(durationMS);

    qDebug() << SB_DEBUG_INFO << _durationTime[_currentPlayerID];
}

void
PlayerController::playerPositionChanged(qint64 durationMS)
{
    QString tStr;
    const int durationSec=durationMS/1000;

    QTime currentTime=calculateTime(durationMS);
    QString format = "mm:ss";
    if(_durationTime[_currentPlayerID].hour()>=1)
    {
        format = "hh:mm:ss";
    }
    tStr = currentTime.toString(format) + " / " + _durationTime[_currentPlayerID].toString(format);

    _playerProgressSlider[_currentPlayerID]->setValue(durationSec);
    _playerDurationLabel[_currentPlayerID]->setText(tStr);
}

void
PlayerController::playerSeek(int ms)
{
    qDebug() << SB_DEBUG_INFO << ms;
    _playerInstance[_currentPlayerID].setPosition(ms * 1000);
}

void
PlayerController::playerStateChanged(QMediaPlayer::State playerState)
{
    if((_state==PlayerController::sb_player_state_changing_media) ||
            playerState==QMediaPlayer::PausedState ||
            playerState==QMediaPlayer::PlayingState)
    {
        return;
    }
    qDebug() << SB_DEBUG_INFO << "**************************************" << _state << _currentPlayID << playerState;
    if(_state==PlayerController::sb_player_state_play)
    {
        //	Continue with next
        calculateNextSongID();
        playerPlay();
    }
}

///	Private slots
void
PlayerController::playerDataClicked(const QUrl &url)
{
    qDebug() << SB_DEBUG_INFO << url;
    QStringList l=url.toString().split('_');
    SBID item(static_cast<SBID::sb_type>(l[0].toInt()),l[1].toInt());
    Context::instance()->getNavigator()->openScreenByID(item);
    _updatePlayerInfo();
}

///	Private methods
QTime
PlayerController::calculateTime(qint64 ms) const
{
    const int s=ms/1000;
    const int seconds = (s) % 60;
    const int m=(s/60);
    const int minutes = (m) % 60;
    const int h=(m/24);
    const int hours = (h) % 24;

    return QTime(hours,minutes,seconds);
}

int
PlayerController::calculateNextSongID()
{
    int newCurrentPlayID=_currentPlayID;
    newCurrentPlayID+=(_seekPreviousSongFlag==1)?-1:1;
    if(newCurrentPlayID<0 || newCurrentPlayID==_playList.count())
    {
        newCurrentPlayID=0;
    }
    _updateCurrentPlayerID(newCurrentPlayID);
    return _currentPlayID;
}

void
PlayerController::init()
{
    _initDoneFlag=0;
    _currentPlayerID=0;
    _state=PlayerController::sb_player_state_stopped;
    _currentPlayID=-1;
    _playList.clear();
    _seekPreviousSongFlag=0;
}

void
PlayerController::makePlayerVisible(PlayerController::sb_player player)
{
    _playerFrame[0]->setVisible(player & PlayerController::sb_player_left);
    _playerFrame[1]->setVisible(player & PlayerController::sb_player_right);
}

///
/// \brief PlayerController::_playerStop
///
/// Stops the current physical player without changing the UI.
void
PlayerController::_playerStop()
{
    _playerInstance[_currentPlayerID].stop();
    playerSeek(0);
}

void
PlayerController::_updateCurrentPlayerID(int newPlayID)
{
    _currentPlayID=newPlayID;
    qDebug() << SB_DEBUG_INFO << "newPlayID" << newPlayID;
    emit songChanged(_currentPlayID);
}

void
PlayerController::_updatePlayerInfo()
{
    QString playState;
    switch(_state)
    {
    case PlayerController::sb_player_state_stopped:
        playState="Stopped: ";
        break;

    case PlayerController::sb_player_state_play:
        playState="Now Playing: ";
        break;

    case PlayerController::sb_player_state_pause:
        playState="Paused: ";
        break;

    case PlayerController::sb_player_state_changing_media:
        //	ignore
        break;
    }

    playState="<BODY BGCOLOR=\"#f0f0f0\"><CENTER>"+playState;
    if(_currentPlayID>=0)
    {
        playState+=QString(
            "<A HREF=\"%1_%2\">%3</A> by "
            "<A HREF=\"%4_%5\">%6</A> from the "
            "<A HREF=\"%7_%8\">'%9'</A> album")
            .arg(SBID::sb_type_song)
            .arg(_playList[_currentPlayID].sb_song_id)
            .arg(_playList[_currentPlayID].songTitle)

            .arg(SBID::sb_type_performer)
            .arg(_playList[_currentPlayID].sb_performer_id)
            .arg(_playList[_currentPlayID].performerName)

            .arg(SBID::sb_type_album)
            .arg(_playList[_currentPlayID].sb_album_id)
            .arg(_playList[_currentPlayID].albumTitle)
        ;
    }
    playState+="</CENTER></BODY>";
    _playerDataLabel[_currentPlayerID]->setText(playState);
}
