#ifndef SBMEDIAPLAYER_H
#define SBMEDIAPLAYER_H

#include <portaudio.h>

#include <QMediaPlayer>

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
///
class SBMediaPlayer : public QObject
{
    Q_OBJECT

public:
    SBMediaPlayer();
    ~SBMediaPlayer();

    void assignID(int playerID);
    int paCallback(const void *input, void *output, unsigned long frameCount, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags);
    quint64 position() const;
    bool setMedia(const QString& fileName);
    inline QString error() const { return _errMsg; }
    QMediaPlayer::State state() const;

signals:
    void durationChanged(quint64 duration);
    void positionChanged(quint64 position);
    void stateChanged(QMediaPlayer::State state);

public slots:
    void play();
    void pause();
    void setPosition(quint64 position);
    void stop();

private:
    int                 _playerID;
    AudioDecoder*       _ad;
    bool                _portAudioInitFlag;
    PaError             _paError;
    PaStream*           _stream;
    QString             _errMsg;
    bool                _hasErrorFlag;
    QMediaPlayer::State _state;
    bool                _threadPrioritySetFlag;
    quint64             _bufferIndex;
    void*               _buffer[BUFFERSIZE];
    quint64             _oldPositionInSec;

    void closeStream();
    void init();
    void portAudioInit();
    bool portAudioOpen(AudioDecoder* ad);
    void setErrorMsg(const QString& errMsg);
    void setState(QMediaPlayer::State state);
};

#endif // SBMEDIAPLAYER_H
