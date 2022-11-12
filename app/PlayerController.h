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

#include "SBMediaPlayer.h"
#include "SBDuration.h"
#include "SBIDBase.h"

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

    explicit PlayerController(QObject *parent = 0);
    ~PlayerController();

    inline SBIDOnlinePerformancePtr currentPerformancePlaying() const { return _playerInstance[_currentPlayerID].getSBIDOnlinePerformancePtr(); }
    inline int getCurrentPlayer() const { return _currentPlayerID; }
    QMediaPlayer::State playState() const;
    void setPlayerFinished() { _playerPlayingID=-1; }
    inline int getPlayerPlayingID() const { return _playerPlayingID; }
    void handlePlayingSong(int playerID, SBIDOnlinePerformancePtr opPtr);

signals:
    void playNextSong();
    void setRowVisible(int playIndex);

public slots:
    void playerRewind();
    void playerForward();
    void startNextSong(int fromCurrentPlayerID);

private slots:
    void playerDataClicked(const QUrl& url);

protected:
    friend class PlayManager;
    friend class Context;
    friend class SBMediaPlayer;

    void doInit();	//	Init done by Context::
    bool playSong(SBIDOnlinePerformancePtr& performancePtr);
    void continueNextSong(int newCurrentPlayerID);
    void explicitSetPlayerVisible(int playerID);

protected slots:
    bool playerPlay();
    void playerSeek(int ms);
    void playerStop();

private:
    static const int                _maxPlayerID=2;
    int                             _currentPlayerID;
    SBDuration                      _durationTime[_maxPlayerID];
    SBMediaPlayer                   _playerInstance[_maxPlayerID];
    SBIDOnlinePerformancePtr        _nextPerformancePlayingPtr;
    int								_playerPlayingID;

    void _init();
    inline int _getNextPlayerID(int currentPlayerID) const { return currentPlayerID+1>=_maxPlayerID?0:currentPlayerID+1; }
    void _loadNextSong();
    void _makePlayerVisible(int playerID);
    void _refreshPlayingNowData() const;
    bool _setupPlayer(int playerID, SBIDOnlinePerformancePtr opPtr);
    void _startPlayer(int playerID);
};

#endif // PLAYERCONTROLLER_H
