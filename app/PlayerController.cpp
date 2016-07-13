#include "PlayerController.h"

#include <QPalette>

#include "Common.h"
#include "Context.h"
#include "Controller.h"
#include "DataAccessLayer.h"
#include "DataEntityCurrentPlaylist.h"
#include "DataEntitySong.h"
#include "MainWindow.h"
#include "Navigator.h"
#include "Properties.h"
#include "SBMediaPlayer.h"
#include "SBModelQueuedSongs.h"
#include "SBSqlQueryModel.h"


PlayerController::PlayerController(QObject *parent) : QObject(parent)
{
}

void
PlayerController::setModelCurrentPlaylist_depreciated(SBModelQueuedSongs *mcp)
{
    _modelCurrentPlaylist=mcp;
}

///	Public slots
void
PlayerController::playerPrevious_depreciated()
{
    qDebug() << SB_DEBUG_INFO << "**************************************";
    qDebug() << SB_DEBUG_INFO
             << "_state_=" << _state
    ;

    //	If 1st song of playlist is playing, simply do a seek to start.
    if(_modelCurrentPlaylist->currentPlayID()==0)
    {
        qDebug() << SB_DEBUG_INFO;
        playerSeek(0);
    }
    else
    {
        qDebug() << SB_DEBUG_INFO;
        _state=PlayerController::sb_player_state_changing_media;
        _playerStop_depreciated();
        bool isPlayingFlag=0;
        SBIDSong previousSong;
        SBIDSong newSong=_currentSongPlaying;

        //	Get out of loop if we're stuck. Either:
        //	-	get a song playing, or
        //	-	the very first song does not play for whatever reason
        while(isPlayingFlag==0 && (previousSong!=newSong))
        {
            previousSong=newSong;
            newSong=_calculateNextSongID_depreciated(1);
            qDebug() << SB_DEBUG_INFO << "Calling this->playSong()";
            isPlayingFlag=playSong(newSong);
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
PlayerController::playerForward()
{
    qDebug() << SB_DEBUG_INFO << "**************************************";
    qDebug() << SB_DEBUG_INFO
             << "_state_=" << _state
    ;
    if(_modelCurrentPlaylist->currentPlayID()==-1)
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
PlayerController::playerNext_depreciated()
{
    qDebug() << SB_DEBUG_INFO << ">|>|>|>|>|>|>|>|>|>|>|>|>|>|>|>|>|>|>|>|>|>|>|>|>|>|>|>|>|>|>|>|>|>|>|>|>|>|>|>|>|>|";
    qDebug() << SB_DEBUG_INFO << "_state_=" << _state
    ;

    _state=PlayerController::sb_player_state_changing_media;
    _playerStop_depreciated();
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
        newSong=_calculateNextSongID_depreciated();
        qDebug() << SB_DEBUG_INFO << "newSong=" << newSong ;
        if(newSong==_currentSongPlaying)
        {
            //	End of the list
            if(cpl->playingRadioFlag_depreciated())
            {
                qDebug() << SB_DEBUG_INFO;
                this->playerStop();
                Context::instance()->getPlayManager()->startRadio();
                return;
            }
            else
            {
                this->playerStop();
                _modelCurrentPlaylist->resetCurrentPlayID_depreciated();
            }
        }
        else
        {
            qDebug() << SB_DEBUG_INFO << "Calling this->playSong()";
            isPlayingFlag=playSong(newSong);
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

bool
PlayerController::playerPlayNonRadio_depreciated(const SBID& id)
{
    _radioPlayingFlag_depreciated=0;
    if(id.sb_item_type()==SBID::sb_type_playlist)
    {
        emit playlistChanged_depreciated(id);
    }
    else
    {
        emit playlistChanged_depreciated(SBID());
    }
    return this->playerPlay();
}

bool
PlayerController::playerPlayInRadio_depreciated()
{
    _radioPlayingFlag_depreciated=1;
    qDebug() << SB_DEBUG_INFO;
    emit playlistChanged_depreciated(SBID());
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
    qDebug() << SB_DEBUG_INFO;
    qDebug() << SB_DEBUG_INFO << "**************************************";
    qDebug() << SB_DEBUG_INFO
             << "_state_=" << _state
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
    qDebug() << SB_DEBUG_INFO
             << "_state_=" << _state
    ;
    _playerProgressSlider[_currentPlayerID]->setValue(0);
    _updatePlayState(PlayerController::sb_player_state_stopped);
    _playerInstance[_currentPlayerID].stop();
    qDebug() << SB_DEBUG_INFO;
    playerSeek(0);
}

///
/// \brief PlayerController::playSong
/// \param playID
/// \return
///
/// Returns 1 on success, 0 otherwise.
bool
PlayerController::playSong(SBID& song)
{
    DataAccessLayer* dal=Context::instance()->getDataAccessLayer();
    Controller* c=Context::instance()->getController();

    qDebug() << SB_DEBUG_INFO << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>";

    qDebug() << SB_DEBUG_INFO << dal->getSchemaName();

    QString path=QString("%1/%2%3%4")
                .arg(Context::instance()->getProperties()->musicLibraryDirectory())
                .arg(dal->getSchemaName())
                .arg(dal->getSchemaName().length()?"/":"")
                .arg(song.path)
    ;
    qDebug() << SB_DEBUG_INFO << path;
/*
#ifdef Q_OS_UNIX
            "/Volumes/bigtmp/Users/roy/songbase/music/files/rock/"+song.path;
#endif
#ifdef Q_OS_WIN
            "D:/songbase/files/rock/"+song.path;
#endif
*/
    emit songChanged(song);	//	changed to here, so we can continue in case of error of playing a song.
    //	This used to be here after instructing player to play

    qDebug() << SB_DEBUG_INFO << path;

    if(_playerInstance[_currentPlayerID].setMedia(path)==0)
    {
        QString errorMsg=_playerInstance[_currentPlayerID].error();
        c->updateStatusBarText(errorMsg);
        qDebug() << SB_DEBUG_INFO << errorMsg;
        _updatePlayState(PlayerController::sb_player_state_stopped);
        song.errorMsg=errorMsg;
        return 0;
    }

    //	Instruct player to play
    _currentSongPlaying=song;
    _playerInstance[_currentPlayerID].play();
    _updatePlayState(PlayerController::sb_player_state_play);


    //	Update timestamp in online_performance
    //	Do this in radio mode only.
    if(_radioPlayingFlag_depreciated)
    {
        DataEntitySong::updateLastPlayDate(_currentSongPlaying);
    }

    return 1;
}

///	Private methods
SBIDSong
PlayerController::_calculateNextSongID_depreciated(bool previousFlag) const
{
    SB_DEBUG_IF_NULL(_modelCurrentPlaylist);
    return SBIDSong(_modelCurrentPlaylist->getNextSong_depreciated(previousFlag));
}

void
PlayerController::_init()
{
    _currentPlayerID=0;
    _currentSongPlaying=SBID();
    _modelCurrentPlaylist=NULL;
    _radioPlayingFlag_depreciated=0;
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

///
/// \brief PlayerController::_playerStop
///
/// Stops the current physical player without changing the UI.
void
PlayerController::_playerStop_depreciated()
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
//    if(newState==_state)
//    {
//        return;
//    }
    _state=newState;

    return _refreshPlayingNowData();
}
