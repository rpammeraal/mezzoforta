#include "PlayerController.h"

#include <QPalette>

#include "Common.h"
#include "Context.h"
#include "Controller.h"
#include "MainWindow.h"
#include "Navigator.h"
#include "SBMediaPlayer.h"
#include "SBMessageBox.h"
#include "DataEntityCurrentPlaylist.h"
#include "DataEntitySong.h"
#include "SBSqlQueryModel.h"


PlayerController::PlayerController(QObject *parent) : QObject(parent)
{
    init();
}

void
PlayerController::clearPlaylist()
{
    qDebug() << SB_DEBUG_INFO;
    _playList.clear();
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
        connect(mw->ui.hsMusicPlayerProgressLeft,SIGNAL(valueChanged(int)),
                this, SLOT(playerSeek(int)));
        connect(mw->ui.hsMusicPlayerProgressRight,SIGNAL(valueChanged(int)),
                this, SLOT(playerSeek(int)));
    }
}

void
PlayerController::loadPlaylist(const QMap<int, SBID> &playList,bool firstBatchLoaded)
{
    qDebug() << SB_DEBUG_INFO;
    if(firstBatchLoaded==0)
    {
        qDebug() << SB_DEBUG_INFO << "FIRST BATCH";
        _playList=playList;
        _currentPlayID=-1;
    }
    else
    {
        qDebug() << SB_DEBUG_INFO << "SECOND BATCH";
        for(int i=_playList.count();i<playList.count();i++)
        {
            _playList[i]=playList[i];
        }
    }
}

void
PlayerController::reorderPlaylist(QMap<int, int> fromTo)
{
    QMap<int,SBID> newPlayList;
    for(int i=0;i<_playList.count();i++)
    {
        newPlayList[i]=_playList[fromTo[i]];
    }
    //	This is the only other method that is allowed to change _currentPlayID
    _currentPlayID=fromTo[_currentPlayID];
}

///	Public slots
void
PlayerController::playerPrevious()
{
    qDebug() << SB_DEBUG_INFO << "**************************************";
    qDebug() << SB_DEBUG_INFO
             << "_state_=" << _state
             << "_currentPlayID=" << _currentPlayID
             << "_playList.count()=" << _playList.count()
    ;

    //	If 1st song of playlist is playing, simply do a seek to start.
    if(_currentPlayID==0)
    {
        qDebug() << SB_DEBUG_INFO;
        playerSeek(0);
    }
    else
    {
        qDebug() << SB_DEBUG_INFO;
        _state=PlayerController::sb_player_state_changing_media;
        _playerStop();
        bool isPlayingFlag=0;
        int previousPlayID=-2;
        int newPlayID=_currentPlayID;

        //	Get out of loop if we're stuck. Either:
        //	-	get a song playing, or
        //	-	the very first song does not play for whatever reason
        while(isPlayingFlag==0 && (previousPlayID!=newPlayID))
        {
            previousPlayID=newPlayID;
            newPlayID=calculateNextSongID(previousPlayID,1);
            isPlayingFlag=_playSong(newPlayID);
        }
        //	CWIP:PLAY
        //	If isPlaying==0 show error
        qDebug() << SB_DEBUG_INFO;
        if(!isPlayingFlag)
        {
            playerStop();
        }
    }
}

void
PlayerController::playerRewind()
{
    qDebug() << SB_DEBUG_INFO << "**************************************";
    qDebug() << SB_DEBUG_INFO
             << "_state_=" << _state
             << "_currentPlayID=" << _currentPlayID
             << "_playList.count()=" << _playList.count()
    ;
    qint64 position=_playerInstance[_currentPlayerID].position();
    position=position/1000-10;
    playerSeek(position);
}

void
PlayerController::playerStop()
{
    qDebug() << SB_DEBUG_INFO << "**************************************";
    qDebug() << SB_DEBUG_INFO
             << "_state_=" << _state
             << "_currentPlayID=" << _currentPlayID
             << "_playList.count()=" << _playList.count()
    ;
    _updatePlayState(PlayerController::sb_player_state_stopped);
    _playerStop();
}

///
/// \brief PlayerController::playerPlay
/// \param playID
/// \return
///
/// Plays the song in _playList at position playID.
/// PlayID must be vetted by calculateNextSong.
/// returns 1 on success, 0 on failure.
bool
PlayerController::playerPlay(int playID)
{
    qDebug() << SB_DEBUG_INFO << "**************************************";
    qDebug() << SB_DEBUG_INFO
             << "_state_=" << _state
             << "_currentPlayID=" << _currentPlayID
             << "_playList.count()=" << _playList.count()
    ;

    //	Handle UI stuff
    if(_playerInstance[_currentPlayerID].state()==QMediaPlayer::PlayingState && playID==-1)
    {
        qDebug() << SB_DEBUG_INFO;
        //	If playing, pause
        _playerInstance[_currentPlayerID].pause();
        _updatePlayState(PlayerController::sb_player_state_pause);
    }
    else if(_playerInstance[_currentPlayerID].state()==QMediaPlayer::PausedState)
    {
        qDebug() << SB_DEBUG_INFO;
        //	If paused, resume play
        _playerInstance[_currentPlayerID].play();
        _updatePlayState(PlayerController::sb_player_state_play);
    }
    else if(_playerInstance[_currentPlayerID].state()==QMediaPlayer::StoppedState)
    {
        qDebug() << SB_DEBUG_INFO
                 << "_currentPlayID=" << _currentPlayID
                 << "_playList.count()=" << _playList.count()
        ;
        if(playID==-1 || playID>=_playList.count())
        {
            if(_playList.count()==0)
            {
                qDebug() << SB_DEBUG_INFO << "No playlist";
                return 0;
            }
            if(_playList.count())
            {
                qDebug() << SB_DEBUG_INFO << "Reset playlist";
                playID=0;
            }
        }
        qDebug() << SB_DEBUG_INFO << playID;

        return _playSong(playID);
    }
    qDebug() << SB_DEBUG_INFO;
    return 0;
}

void
PlayerController::playerPlayNow_(int playID)
{
    //	Untested! Possibly deprecated. Possibly in wrong class
    qDebug() << SB_DEBUG_INFO << "**************************************";
    qDebug() << SB_DEBUG_INFO
             << "_state_=" << _state
             << "_currentPlayID=" << _currentPlayID
             << "_playList.count()=" << _playList.count()
    ;
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
    _playerStop();
    playerPlay(playID);
}

void
PlayerController::playerForward()
{
    qDebug() << SB_DEBUG_INFO << "**************************************";
    qDebug() << SB_DEBUG_INFO
             << "_state_=" << _state
             << "_currentPlayID=" << _currentPlayID
             << "_playList.count()=" << _playList.count()
    ;
    if(_currentPlayID==-1)
    {
        return;
    }
    qint64 position=_playerInstance[_currentPlayerID].position();
    position=position/1000+10;
    playerSeek(position);
}

void
PlayerController::playerNext()
{
    qDebug() << SB_DEBUG_INFO << "**************************************";
    qDebug() << SB_DEBUG_INFO
             << "_state_=" << _state
             << ":_currentPlayID=" << _currentPlayID
             << ":_playList.count()=" << _playList.count()
    ;

    _state=PlayerController::sb_player_state_changing_media;
    _playerStop();
    bool isPlayingFlag=0;
    int previousPlayID=-2;
    int newPlayID=_currentPlayID;

    qDebug() << SB_DEBUG_INFO;
    //	Get out of loop if we're stuck. Either:
    //	-	get a song playing, or
    //	-	the very first song does not play for whatever reason
    while(isPlayingFlag==0 && (previousPlayID!=newPlayID))
    {
        previousPlayID=newPlayID;
        newPlayID=calculateNextSongID(previousPlayID);
        isPlayingFlag=_playSong(newPlayID);
        qDebug() << SB_DEBUG_INFO
                 << "isPlayingFlag=" << isPlayingFlag
                 << ":previousPlayID=" << previousPlayID
                 << ":newPlayID=" << newPlayID
        ;
    }
    qDebug() << SB_DEBUG_INFO << isPlayingFlag;
    //	CWIP:PLAY
    //	If isPlaying==0 show error
    if(!isPlayingFlag)
    {
        playerStop();
    }
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
    qDebug() << SB_DEBUG_INFO << "**************************************";
    qDebug() << SB_DEBUG_INFO
             << "_state=" << _state
             << ":_currentPlayID=" << _currentPlayID
             << ":playerState=" << playerState
    ;
    if((_state==PlayerController::sb_player_state_changing_media) ||
            playerState==QMediaPlayer::PausedState ||
            playerState==QMediaPlayer::PlayingState)
    {
        return;
    }
    if(_state==PlayerController::sb_player_state_play)
    {
        //	Continue with next song
        playerNext();
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
    qDebug() << SB_DEBUG_INFO << url;
    QStringList l=url.toString().split('_');
    SBID item(static_cast<SBID::sb_type>(l[0].toInt()),l[1].toInt());
    Context::instance()->getNavigator()->openScreenByID(item);
    _refreshPlayingNowData();	//	For whatever reason, data is hidden after link is clicked.
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
PlayerController::calculateNextSongID(int currentPlayID,bool previousFlag) const
{
    int newPlayID=currentPlayID;
    newPlayID+=(previousFlag==1)?-1:1;
    qDebug() << SB_DEBUG_INFO
             << ":newPlayID=" << newPlayID
             << ":_playList.count()=" << _playList.count()
    ;
    if(newPlayID<0 || newPlayID>=_playList.count())
    {
        newPlayID=0;
    }
    return newPlayID;
}

void
PlayerController::init()
{
    _initDoneFlag=0;
    _currentPlayerID=0;
    _state=PlayerController::sb_player_state_stopped;
    _currentPlayID=-1;
    _playList.clear();
}

SBID
PlayerController::_getPlaylistEntry(int playlistID) const
{
    if(_playList.contains(playlistID))
    {
        for(int i=0;i<_playList.count();i++)
        {
            qDebug() << SB_DEBUG_INFO << i << _playList[i];
        }
        qDebug() << SB_DEBUG_INFO
                 << "playlistID=" << playlistID
                 << _playList[playlistID]
        ;
        return _playList[playlistID];
    }
    return SBID();
}

void
PlayerController::makePlayerVisible(PlayerController::sb_player player)
{
    _playerFrame[0]->setVisible(player & PlayerController::sb_player_left);
    _playerFrame[1]->setVisible(player & PlayerController::sb_player_right);
}

///
/// \brief PlayerController::_playSong
/// \param playID
/// \return
///
/// plays song in _playList with index playID.
/// Returns 1 on success, 0 otherwise.
bool
PlayerController::_playSong(int playID)
{
    Controller* c=Context::instance()->getController();

    qDebug() << SB_DEBUG_INFO << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>";
    qDebug() << SB_DEBUG_INFO
             << "_currentPlayID=" << _currentPlayID
             << "_playList.count()=" << _playList.count()
             << "playID=" << playID
    ;

    if(!_playList.contains(playID))
    {
        qDebug() << SB_DEBUG_ERROR << "playID not in playlist";
        //	CWIP: error
        return 0;
    }

    SBID id=_getPlaylistEntry(playID);
    QString path=
#ifdef Q_OS_UNIX
            "/Volumes/bigtmp/Users/roy/songbase/music/files/rock/"+id.path;
#endif
#ifdef Q_OS_WIN
            "D:/songbase/files/rock/"+id.path;
#endif
    //	CWIP: check if path is empty

    qDebug() << SB_DEBUG_INFO << "path=" << path;

    if(_playerInstance[_currentPlayerID].setMedia(path)==0)
    {
        c->updateStatusBarText(_playerInstance[_currentPlayerID].error());
        qDebug() << SB_DEBUG_INFO << "Missing file";
        _updatePlayState(PlayerController::sb_player_state_stopped);
        return 0;
    }

    //	Instruct player to play
    _currentPlayID=playID;	//	This is the only time that _currentPlayID may be assigned to something else.
    _playerInstance[_currentPlayerID].play();

    _updatePlayState(PlayerController::sb_player_state_play);

    emit songChanged(_currentPlayID);

    //	Update timestamp in online_performance
    //	Do this in both radio and non-radio mode.
    qDebug() << SB_DEBUG_INFO << "Updating timestamp for " << _playList[playID];
    DataEntitySong::updateLastPlayDate(_playList[playID]);

    qDebug() << SB_DEBUG_INFO
             << "_currentPlayID=" << _currentPlayID
    ;

    return 1;
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
    qDebug() << SB_DEBUG_INFO << playState << _state;

    playState="<BODY BGCOLOR=\"#f0f0f0\"><CENTER>"+playState;
    if(_state==PlayerController::sb_player_state_pause ||
       _state==PlayerController::sb_player_state_play)
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
    qDebug() << SB_DEBUG_INFO << playState;
    _playerDataLabel[_currentPlayerID]->setText(playState);
}

void
PlayerController::_updatePlayState(PlayerController::sb_player_state newState)
{
    qDebug() << SB_DEBUG_INFO
             << "_currentPlayID=" << _currentPlayID
             << ":_state=" << _state
             << ":newState=" << newState
    ;
    if(newState==_state)
    {
        return;
    }
    _state=newState;

    return _refreshPlayingNowData();
}
