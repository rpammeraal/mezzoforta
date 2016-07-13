#ifndef PLAYERCONTROLLER_H
#define PLAYERCONTROLLER_H

#include <QMap>
#include <QObject>
#include <qsystemdetection.h>

class QFrame;
class QLabel;
class QPushButton;
class QSlider;
class QTextBrowser;
class SBModelQueuedSongs;

#include "SBMediaPlayer.h"
#include "SBIDSong.h"
#include "Duration.h"

///
/// \brief The PlayerController class
///
/// PlayerController holds the key to playing songs.
/// It controls two instances of SBMediaPlayer and utilizes these.
///
class PlayerController : public QObject
{
    Q_OBJECT

public:
    enum sb_player
    {
        sb_player_none=0,
        sb_player_left=1,
        sb_player_right=2,
        sb_player_both=3
    };

    enum sb_player_state
    {
        sb_player_state_stopped=0,
        sb_player_state_play=1,
        sb_player_state_pause=2,
        sb_player_state_changing_media=4
    };

        explicit PlayerController(QObject *parent = 0);

        inline SBIDSong currentSongPlaying() const { return _currentSongPlaying; }
        inline PlayerController::sb_player_state playState() const { return _state; }
        void setModelCurrentPlaylist_depreciated(SBModelQueuedSongs* mcp);

signals:
        void songChanged(const SBID& song);
    void playlistChanged_depreciated(const SBID& playlist);
        void playNextSong();

public slots:
    void playerPrevious_depreciated(); //
        void playerRewind();
        void playerForward();
    void playerNext_depreciated(); //
        void playerDurationChanged(quint64 duration);
        void playerPositionChanged(quint64 duration);
        void playerStateChanged(QMediaPlayer::State playerState);

    //	The following methods are used for the first song and
    //	sets whether playlist or radio. If radio, update last played date
    //	is set at the start of song.
    bool playerPlayNonRadio_depreciated(const SBID& id);
    bool playerPlayInRadio_depreciated();

    inline bool radioPlayingFlag_depreciated() const { return _radioPlayingFlag_depreciated; }

private slots:
        void playerDataClicked(const QUrl& url);

protected:
    friend class PlayManager;
    friend class Context;
        void doInit();	//	Init done by Context::
        bool playSong(SBID& song);

protected slots:
    friend class PlayManager;
        bool playerPlay();
        void playerSeek(int ms);
        void playerStop();

private:
    static const int                  _maxPlayerID=2;
    int                               _currentPlayerID;
    SBIDSong                          _currentSongPlaying;
    Duration                          _durationTime[_maxPlayerID];
    SBModelQueuedSongs*               _modelCurrentPlaylist;
    QFrame*                           _playerFrame[_maxPlayerID];
    QPushButton*                      _playerPlayButton[_maxPlayerID];
    QSlider*                          _playerProgressSlider[_maxPlayerID];
    QLabel*                           _playerDurationLabel[_maxPlayerID];
    QTextBrowser*                     _playerDataLabel[_maxPlayerID];
    SBMediaPlayer                     _playerInstance[_maxPlayerID];
    bool                              _radioPlayingFlag_depreciated;
    PlayerController::sb_player_state _state;

    SBIDSong _calculateNextSongID_depreciated(bool previousFlag=0) const;
        void _init();
        void _makePlayerVisible(PlayerController::sb_player player);
        Duration _ms2Duration(quint64 ms) const;
    void _playerStop_depreciated();

        void _refreshPlayingNowData() const;
        void _updatePlayState(PlayerController::sb_player_state newState);
};

#endif // PLAYERCONTROLLER_H
