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
    void prepareNextSong(int currentPlayerID);

public slots:
    void play();
    void playOnCue();
    void pause();
    void setPosition(qint64 position);
    void stop();

protected:
    friend class PlayerController;
    inline SBIDOnlinePerformancePtr getSBIDOnlinePerformancePtr() const { return _opPtr; }
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
    bool						_fiveSecondMarkPassed;
    SBDuration					_durationTime;
    SBIDOnlinePerformancePtr	_opPtr;	//	online performance to be played
    QFrame*						_playerFrame;
    QPushButton*        		_playerPlayButton;
    QSlider*            		_playerProgressSlider;
    QLabel*             		_playerDurationLabel;
    QTextBrowser*     			_playerDataLabel;

    void _closeStream();
    QString _constructPath(SBIDOnlinePerformancePtr opPtr) const;
    QString _getErrorMsg() const;
    void _init();
    void _portAudioInit();
    bool _portAudioOpen();
    void _portAudioTerminate();
    void _setDuration(int durationMs);
    void _setErrorMsg(const QString& errMsg);
    void _setState(QMediaPlayer::State state);
    void _updatePosition();
};

#endif // SBMEDIAPLAYER_H
