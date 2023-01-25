#ifndef SBMEDIAPLAYER_H
#define SBMEDIAPLAYER_H

#include <portaudio.h>

class QFrame;
class QPushButton;
class QSlider;
class QLabel;
class QHBoxLayout;

#include <QMediaPlayer>
#include <QTextBrowser>
#include "SBIDBase.h"
#include "SBDuration.h"

#define CHECK(x) { if(!(x)) { \
fprintf(stderr, "%s:%i: failure at: %s\n", __FILE__, __LINE__, #x); \
return(0); } }

#define BUFFERSIZE 2048

class AudioDecoder;
///
/// \brief The SBMediaPlayer class
///
/// This class is our own implementation of QMediaPlayer, as the latter will not
/// play anything else than RAW files (on Mac) or RAW/MP3 files (on Windows).
/// Since we need a bit more than these, the SBMediaPlayer class was created that
/// used PortAudio.
/// Logic to play songs is in PlayManager
///
/// Sequence to run SBMediaPlayer instance:
/// 1.	setMedia
/// 2.	startPlay()/playOnCue()
/// 3.	pause()/play()/stop()/setPosition()
///
class SBMediaPlayer : public QObject
{
    Q_OBJECT

public:
    SBMediaPlayer();
    ~SBMediaPlayer();

    inline QString error() const { return _errMsg; }
    int paCallback(const void *input, void *output, unsigned long frameCount, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags);
    QString path() const;
    quint64 position() const;
    void releaseMedia();
    bool setMedia(SBIDOnlinePerformancePtr opPtr);
    inline void setPlayerID(int id) { _playerID=id; }
    inline QMediaPlayer::State state() const { return _state; }

signals:
    void stateChanged(QMediaPlayer::State state);
    void prepareToStartNextSong(int currentPlayerID);
    void needMoreSongs();	//	thrown when no more songs in current playlist exists.
    void weArePlayingTime2LoadNextSong(int currentPlayerID, SBIDOnlinePerformancePtr opPtr);	//	This will be emitted *once* only.

public slots:
    //	Player Initialization slots
    void playOnCue();		//	Start player from beginning when other player finished. This function
                            //	should only be called *once* for the duration of each song.
    int startPlay();		//	Start player from beginning. This function should only be called *once*
                            //	for the duration of each song.

    //	Player manipulation slots
    void play();			//	Play() and pause() are for pausing and continuing playing.
                            //	The song should be started by calling startPlay() first.
    void pause();
    void setPosition(qint64 position);
    void stop();

protected:
    friend class PlayerController;
    inline SBIDOnlinePerformancePtr getSBIDOnlinePerformancePtr() const { return _opPtr; }
    void resetPlayer();
    inline void setPlayerDataLabel(QTextBrowser* tb) { _playerDataLabel=tb; }
    inline void setPlayerDurationLabel(QLabel* l) { _playerDurationLabel=l; }
    inline void setPlayerPlayButton(QPushButton* pb) { _playerPlayButton=pb; }
    inline void setPlayerFrame(QFrame* f) { _playerFrame=f; }
    inline void setPlayerVisible(bool visible) { _playerFrame->setVisible(visible); }
    inline void setProgressSlider(QSlider* s) { _playerProgressSlider=s; }

    void refreshPlayingNowData() const;

private:
    AudioDecoder*       		_ad;
    bool                		_portAudioInitFlag;
    PaError             		_paError;
    PaStream*           		_stream;
    QString             		_errMsg;
    bool                		_hasErrorFlag;
    QMediaPlayer::State 		_state;
    bool                		_threadPrioritySetFlag;
    quint64             		_oldPositionInSec;
    bool						_isActivePlayer;
    int                 		_playerID;
    bool                		_startNextSongSignalGiven;
    bool						_fiveSecondsLeftToPlay;
    SBDuration					_durationTime;
    SBIDOnlinePerformancePtr	_opPtr;	//	online performance to be played
    QFrame*						_playerFrame;
    QPushButton*        		_playerPlayButton;
    QSlider*            		_playerProgressSlider;
    QLabel*             		_playerDurationLabel;
    QTextBrowser*     			_playerDataLabel;
    bool                        _mediaLoaded;	//	player is ready to play.
    bool						_isReadyTogo;	//	use this to signify that we have thrown weArePlayingTime2Load...

    int _checkReadyTogoStatus() const;
    void _closeStream();
    QString _constructPath(SBIDOnlinePerformancePtr opPtr) const;
    QString _getErrorMsg() const;
    void _init();
    void _portAudioInit();
    bool _portAudioOpen();
    void _portAudioTerminate();
    int _progressSliderMovable() const;
    void _resetPlayer();
    void _setDuration(int durationMs);
    void _setErrorMsg(const QString& errMsg);
    void _setState(QMediaPlayer::State state);
    void _updatePosition();

private slots:
    void _setPlayerDataLabel(QString text) const;
};

#endif // SBMEDIAPLAYER_H
