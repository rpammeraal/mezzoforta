#ifndef PLAYERCONTROLLER_H
#define PLAYERCONTROLLER_H

#include <QMap>
#include <QObject>
#include <QTime>
#include <qsystemdetection.h>

class QFrame;
class QLabel;
class QPushButton;
class QSlider;
class QTextBrowser;
class SBModelCurrentPlaylist;

#include "SBMediaPlayer.h"
#include "SBID.h"

///
/// \brief The PlayerController class
///
/// PlayerController holds the key to playing songs.
/// It controls two instances of SBMediaPlayer and utilizes these.
///	The master of to be played list should reside with PlayerController.
///
class PlayerController : public QObject
{
    Q_OBJECT

    friend class SBTabCUrrentPlaylist;

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

    void initialize();
    void setModelCurrentPlaylist(SBModelCurrentPlaylist* mcp);

signals:
    void songChanged(const SBID& song);

public slots:
    void playerPrevious();
    void playerRewind();
    void playerStop();
    bool playerPlay();
    void playerForward();
    void playerNext();
    void playerDurationChanged(quint64 duration);
    void playerPositionChanged(quint64 duration);
    void playerSeek(int ms);
    void playerStateChanged(QMediaPlayer::State playerState);

private slots:
    void playerDataClicked(const QUrl& url);

private:
    bool _initDoneFlag;
    PlayerController::sb_player_state _state;
    SBID _currentSong;
    SBModelCurrentPlaylist* _modelCurrentPlaylist;

    int _currentPlayerID;
    static const int _maxPlayerID=2;
    QFrame* _playerFrame[_maxPlayerID];
    QPushButton* _playerPlayButton[_maxPlayerID];
    QSlider* _playerProgressSlider[_maxPlayerID];
    QLabel* _playerDurationLabel[_maxPlayerID];
    QTextBrowser* _playerDataLabel[_maxPlayerID];
    SBMediaPlayer _playerInstance[_maxPlayerID];
    QTime _durationTime[_maxPlayerID];

    SBID calculateNextSongID(bool previousFlag=0) const;
    QTime calculateTime(quint64 ms) const;
    void init();
    void makePlayerVisible(PlayerController::sb_player player);
    bool _playSong(const SBID& song);
    void _playerStop();
    void _refreshPlayingNowData() const;
    void _updatePlayState(PlayerController::sb_player_state newState);
};

#endif // PLAYERCONTROLLER_H
