#include "PlayerController.h"

#include <QPalette>

#include "Common.h"
#include "Context.h"
#include "Controller.h"
#include "MainWindow.h"
#include "Navigator.h"
#include "SBMediaPlayer.h"
#include "SBMessageBox.h"
#include "SBModelQueuedSongs.h"
#include "DataEntityCurrentPlaylist.h"
#include "DataEntitySong.h"
#include "SBSqlQueryModel.h"


PlayerController::PlayerController(QObject *parent) : QObject(parent)
{
    _init();
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
}

void
PlayerController::setModelCurrentPlaylist(SBModelQueuedSongs *mcp)
{
    _modelCurrentPlaylist=mcp;
}

///	Public slots
void
PlayerController::playerPrevious()
{
    qDebug() << SB_DEBUG_INFO << "**************************************";
    qDebug() << SB_DEBUG_INFO
             << "_state_=" << _state
    ;

    //	If 1st song of playlist is playing, simply do a seek to start.
    if(_modelCurrentPlaylist->currentPlaylistIndex()==0)
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
        SBIDSong previousSong;
        SBIDSong newSong=_currentSongPlaying;

        //	Get out of loop if we're stuck. Either:
        //	-	get a song playing, or
        //	-	the very first song does not play for whatever reason
        while(isPlayingFlag==0 && (previousSong!=newSong))
        {
            previousSong=newSong;
            newSong=calculateNextSongID(1);
            isPlayingFlag=_playSong(newSong);
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
    ;
    qint64 position=_playerInstance[_currentPlayerID].position();
    qDebug() << SB_DEBUG_INFO << position;
    position=position/1000-10;
    qDebug() << SB_DEBUG_INFO << position;
    playerSeek(position);
}

void
PlayerController::playerStop()
{
    qDebug() << SB_DEBUG_INFO << "**************************************";
    qDebug() << SB_DEBUG_INFO
             << "_state_=" << _state
    ;
    _updatePlayState(PlayerController::sb_player_state_stopped);
    _playerStop();
}


///
/// \brief PlayerController::playerPlay
/// \param playID
/// \return
///
/// PlayID must be vetted by calculateNextSong.
/// returns 1 on success, 0 on failure.
bool
PlayerController::playerPlay()
{
    qDebug() << SB_DEBUG_INFO;
    qDebug() << SB_DEBUG_INFO << "**************************************";
    qDebug() << SB_DEBUG_INFO
             << "_state_=" << _state
    ;

    if(_modelCurrentPlaylist==NULL)
    {
        //	No playlist loaded - switch to radio.
        qDebug() << SB_DEBUG_INFO;

        //	Open CurrentPlaylistTab
        Context::instance()->getNavigator()->showCurrentPlaylist();

        qDebug() << SB_DEBUG_INFO;
        MainWindow* mw=Context::instance()->getMainWindow();

        qDebug() << SB_DEBUG_INFO;
        mw->ui.tabCurrentPlaylist->startRadio();

        //	Return now, as startRadio() will call playerPlay() on its own.
        return 1;
    }

    int currentIndex=_modelCurrentPlaylist->currentPlaylistIndex();
    qDebug() << SB_DEBUG_INFO << currentIndex;
    if(currentIndex==-1)
    {
        if(_modelCurrentPlaylist->rowCount())
        {
            currentIndex++;
            qDebug() << SB_DEBUG_INFO << currentIndex;
        }
        else
        {
            qDebug() << SB_DEBUG_INFO << currentIndex;
            return 0;
        }
    }

    SBID song=_modelCurrentPlaylist->getSongFromPlaylist(_modelCurrentPlaylist->currentPlaylistIndex());
    qDebug() << SB_DEBUG_INFO
             << song
             << "song.playPosition=" << song.playPosition
    ;

    //	Handle UI stuff
    if(_playerInstance[_currentPlayerID].state()==QMediaPlayer::PlayingState)
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
        qDebug() << SB_DEBUG_INFO;

        return _playSong(song);
    }
    qDebug() << SB_DEBUG_INFO;
    return 0;
}

void
PlayerController::playerForward()
{
    qDebug() << SB_DEBUG_INFO << "**************************************";
    qDebug() << SB_DEBUG_INFO
             << "_state_=" << _state
    ;
    if(_modelCurrentPlaylist->currentPlaylistIndex()==-1)
    {
        return;
    }
    qint64 position=_playerInstance[_currentPlayerID].position();
    position=(position/1000)+10;
    qDebug() << SB_DEBUG_INFO;
    playerSeek(position);
}

//	CWIP: merge with playerPrevious
void
PlayerController::playerNext()
{
    qDebug() << SB_DEBUG_INFO << ">|>|>|>|>|>|>|>|>|>|>|>|>|>|>|>|>|>|>|>|>|>|>|>|>|>|>|>|>|>|>|>|>|>|>|>|>|>|>|>|>|>|";
    qDebug() << SB_DEBUG_INFO
             << "_state_=" << _state
    ;

    _state=PlayerController::sb_player_state_changing_media;
    _playerStop();
    bool isPlayingFlag=0;
    //SBID previousSong=_currentSongPlaying;
    SBIDSong newSong;
    int maxTries=_modelCurrentPlaylist->rowCount();
    int currentTry=0;
    const MainWindow* mw=Context::instance()->getMainWindow();
    SBTabQueuedSongs* cpl=mw->ui.tabCurrentPlaylist;
    SB_DEBUG_IF_NULL(cpl);

    //	Get out of loop if we're stuck. Either:
    //	-	get a song playing, or
    //	-	the very first song does not play for whatever reason
    qDebug() << SB_DEBUG_INFO;
    while(
        currentTry++<maxTries && //	only try as many times as there are songs, and
        isPlayingFlag==0         //	we still don't have anything playing as of now
    )
    {
        newSong=calculateNextSongID();
        qDebug() << SB_DEBUG_INFO << "newSong=" << newSong ;
        if(newSong==_currentSongPlaying)
        {
            //	End of the list
            if(cpl->playingRadioFlag())
            {
                qDebug() << SB_DEBUG_INFO;
                this->playerStop();
                cpl->startRadio();	//	restart radio
                return;
            }
            else
            {
                qDebug() << SB_DEBUG_INFO << _modelCurrentPlaylist->currentPlaylistIndex();
                this->playerStop();
                qDebug() << SB_DEBUG_INFO << _modelCurrentPlaylist->currentPlaylistIndex();
                _modelCurrentPlaylist->resetCurrentPlayID();
            }
        }
        else
        {
            isPlayingFlag=_playSong(newSong);
        }
    }
    qDebug() << SB_DEBUG_INFO << isPlayingFlag;

    //	CWIP:PLAY
    //	If isPlaying==0 show error
    qDebug() << SB_DEBUG_INFO << isPlayingFlag;
    if(!isPlayingFlag)
    {
        playerStop();
    }
}

void
PlayerController::playerDurationChanged(quint64 durationMS)
{

    const int durationSec=durationMS/1000;

    _playerProgressSlider[_currentPlayerID]->setValue(0);
    _playerProgressSlider[_currentPlayerID]->setMaximum(durationSec);

    _durationTime[_currentPlayerID]=calculateTime(durationMS);
}

void
PlayerController::playerPositionChanged(quint64 durationMS)
{
    QString tStr;
    const int durationSec=durationMS/1000;

    SBTime currentTime=calculateTime(durationMS);
    QString format = "mm:ss";
    if(_durationTime[_currentPlayerID].hour()>=1)
    {
        format = "hh:mm:ss";
    }
    tStr = currentTime.toString(SBTime::sb_hhmmss_format) + " / " + _durationTime[_currentPlayerID].toString(SBTime::sb_hhmmss_format);

    _playerProgressSlider[_currentPlayerID]->setValue(durationSec);
    _playerDurationLabel[_currentPlayerID]->setText(tStr);
}

void
PlayerController::playerSeek(int s)
{
    _playerInstance[_currentPlayerID].setPosition(s * 1000 );
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
    if(_state==PlayerController::sb_player_state_play)
    {
        //	Continue with next song
        playerNext();
    }
}

bool
PlayerController::playerPlayNonRadio(const SBID& id)
{
    _radioPlayingFlag=0;
    if(id.sb_item_type()==SBID::sb_type_playlist)
    {
        emit playlistChanged(id);
    }
    else
    {
        emit playlistChanged(SBID());
    }
    return this->playerPlay();
}

bool
PlayerController::playerPlayInRadio()
{
    _radioPlayingFlag=1;
    qDebug() << SB_DEBUG_INFO;
    emit playlistChanged(SBID());
    return this->playerPlay();
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
SBTime
PlayerController::calculateTime(quint64 ms) const
{
    const int s=ms/1000;
    const int seconds = (s) % 60;
    const int m=(s/60);
    const int minutes = (m) % 60;
    const int h=(m/24);
    const int hours = (h) % 24;

    return SBTime(hours,minutes,seconds);
}

SBIDSong
PlayerController::calculateNextSongID(bool previousFlag) const
{
    SB_DEBUG_IF_NULL(_modelCurrentPlaylist);
    return SBIDSong(_modelCurrentPlaylist->getNextSong(previousFlag));
}

void
PlayerController::_init()
{
    _currentPlayerID=0;
    _currentSongPlaying=SBID();
    _initDoneFlag=0;
    _modelCurrentPlaylist=NULL;
    _radioPlayingFlag=0;
    _state=PlayerController::sb_player_state_stopped;
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
/// Returns 1 on success, 0 otherwise.
bool
PlayerController::_playSong(const SBID& song)
{
    Controller* c=Context::instance()->getController();

    qDebug() << SB_DEBUG_INFO << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>";

    QString path=
#ifdef Q_OS_UNIX
            "/Volumes/bigtmp/Users/roy/songbase/music/files/rock/"+song.path;
#endif
#ifdef Q_OS_WIN
            "D:/songbase/files/rock/"+song.path;
#endif
    emit songChanged(song);	//	changed to here, so we can continue in case of error of playing a song.
    //	This used to be here after instructing player to play

    qDebug() << SB_DEBUG_INFO << path;

    if(_playerInstance[_currentPlayerID].setMedia(path)==0)
    {
        c->updateStatusBarText(_playerInstance[_currentPlayerID].error());
        qDebug() << SB_DEBUG_INFO << "Missing file";
        _updatePlayState(PlayerController::sb_player_state_stopped);
        return 0;
    }

    //	Instruct player to play
    _currentSongPlaying=song;
    _playerInstance[_currentPlayerID].play();
    _updatePlayState(PlayerController::sb_player_state_play);


    //	Update timestamp in online_performance
    //	Do this in radio mode only.
    if(_radioPlayingFlag)
    {
        DataEntitySong::updateLastPlayDate(_currentSongPlaying);
    }

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
    qDebug() << SB_DEBUG_INFO;
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

    playState="<BODY BGCOLOR=\"#f0f0f0\"><CENTER>"+playState;
    if(_state==PlayerController::sb_player_state_pause ||
       _state==PlayerController::sb_player_state_play)
    {
        playState+=QString(
            "<A HREF=\"%1_%2\">%3</A> by "
            "<A HREF=\"%4_%5\">%6</A> from the "
            "<A HREF=\"%7_%8\">'%9'</A> album")
            .arg(SBID::sb_type_song)
            .arg(_currentSongPlaying.sb_song_id)
            .arg(_currentSongPlaying.songTitle)

            .arg(SBID::sb_type_performer)
            .arg(_currentSongPlaying.sb_performer_id)
            .arg(_currentSongPlaying.performerName)

            .arg(SBID::sb_type_album)
            .arg(_currentSongPlaying.sb_album_id)
            .arg(_currentSongPlaying.albumTitle)
        ;
    }
    playState+="</CENTER></BODY>";
    _playerDataLabel[_currentPlayerID]->setText(playState);
}

void
PlayerController::_updatePlayState(PlayerController::sb_player_state newState)
{
    if(newState==_state)
    {
        return;
    }
    _state=newState;

    return _refreshPlayingNowData();
}
