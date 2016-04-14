#ifndef PLAYERCONTROLLER_H
#define PLAYERCONTROLLER_H

#include <QMap>
#include <QObject>
#include <QTime>

class QFrame;
class QLabel;
class QPushButton;
class QSlider;
class QTextBrowser;

#include "SBMediaPlayer.h"
#include "SBID.h"

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
    void initialize();

signals:
    void songChanged(int playID);

public slots:
    void loadPlaylist(QMap<int,int> fromTo=QMap<int,int>());
    void playerPrevious();
    void playerRewind();
    void playerStop();
    bool playerPlay(int playID=-1);
    void playerPlayNow(int playID);
    void playerForward();
    void playerNext();
    void playerDurationChanged(qint64 duration);
    void playerPositionChanged(qint64 duration);
    void playerSeek(int ms);
    void playerStateChanged(QMediaPlayer::State playerState);

private slots:
    void playerDataClicked(const QUrl& url);

private:
    bool _initDoneFlag;
    PlayerController::sb_player_state _state;
    int _currentPlayID;
    QMap<int,SBID> _playList;	//	<_currentPlayID:0,SBID>
    bool _seekPreviousSongFlag; //	used if prevSong is clicked

    int _currentPlayerID;
    const int _maxPlayerID;
    QFrame* _playerFrame[2];
    QPushButton* _playerPlayButton[2];
    QSlider* _playerProgressSlider[2];
    QLabel* _playerDurationLabel[2];
    QTextBrowser* _playerDataLabel[2];
    SBMediaPlayer _playerInstance[2];
    QTime _durationTime[2];

    int calculateNextSongID();
    QTime calculateTime(qint64 ms) const;
    void init();
    SBID _getPlaylistEntry(int playlistID) const;
    void makePlayerVisible(PlayerController::sb_player player);
    void _playerStop();
    void _updateCurrentPlayerID(int newPlayID);
    void _updatePlayerInfo();
};

#endif // PLAYERCONTROLLER_H
