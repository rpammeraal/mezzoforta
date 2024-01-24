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
    inline int getCurrentPlayerID() const { return _currentPlayerID; }
    inline int getPlayerPlayingID() const { return _playerPlayingID; }
    void handlePlayingSong(int playerID, SBIDOnlinePerformancePtr opPtr);
    QMediaPlayer::PlaybackState playState() const;
    void setPlayerFinished(int currentPlayerID);

signals:
    void playNextSong();
    void needMoreSongs(); 	//	Thrown when no more songs are available.
    void setRowVisible(int playIndex);

public slots:
    void playerRewind();
    void playerForward();
    void startNextPlayerOnCue(int fromCurrentPlayerID);
    void handleNeedMoreSongs();
    void handleReorderedPlaylist();
    void loadNextSong();
    void processPlayerStarted(int playerID, SBIDOnlinePerformancePtr opPtr);

private slots:
    void playerDataClicked(const QUrl& url);

protected:
    friend class PlayManager;
    friend class Context;
    friend class SBMediaPlayer;

    void doInit();	//	Init done by Context::
    bool playSong(SBIDOnlinePerformancePtr& performancePtr);
    void explicitSetPlayerVisible(int playerID);

protected slots:
    bool playerPlay();
    void playerSeek(int ms);
    void playerStop();

private:
    static const int                _maxPlayerID=2;
    int                             _currentPlayerID;	//	This value can not be larger than _maxPlayerID-1
                                                        //	(In practice: 0 or 1).
    SBDuration                      _durationTime[_maxPlayerID];
    SBMediaPlayer                   _playerInstance[_maxPlayerID];
    SBIDOnlinePerformancePtr        _nextPerformancePlayingPtr;
    int								_playerPlayingID;	//	used to find out which player is actually playing.
                                                        //	Used to start player 'on cue' when other player finishes.
                                                        //	Can have values -1,0,1, where -1 means that no player
                                                        //	is playing.

    void _init();
    inline int _getNextPlayerID(int currentPlayerID) const { return currentPlayerID+1>=_maxPlayerID?0:currentPlayerID+1; }
    void _makePlayerVisible(int playerID);
    void _resetPlayers();
    void _setCurrentPlayerID(int playerID);
    void _setPlayerPlayingID(int playerID);
    bool _setupPlayer(int playerID, SBIDOnlinePerformancePtr opPtr);
    void _startPlayer(int playerID);
};

#endif // PLAYERCONTROLLER_H
