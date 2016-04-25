#ifndef SBMEDIAPLAYER_H
#define SBMEDIAPLAYER_H

#include <portaudio.h>

#include <QMediaPlayer>

#include "StreamContent.h"

#define CHECK(x) { if(!(x)) { \
fprintf(stderr, "%s:%i: failure at: %s\n", __FILE__, __LINE__, #x); \
return(0); } }

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
    qint64 position() const;
    bool setMedia(const QString& fileName);
    inline QString error() const { return _errMsg; }
    QMediaPlayer::State state() const;

signals:
    void durationChanged(qint64 duration);
    void positionChanged(qint64 position);
    void stateChanged(QMediaPlayer::State state);

public slots:
    void play();
    void pause();
    void setPosition(qint64 position);
    void stop();

private:
    int _playerID;
    StreamContent _sc;
    qint64 _index;
    bool _portAudioInitFlag;
    PaError _paError;
    PaStream* _stream;
    QString _errMsg;
    bool _hasErrorFlag;
    QMediaPlayer::State _state;
    bool _threadPrioritySetFlag;

    qint64 index2PositionInMS(qint64 index) const;
    void closeStream();
    void init();
    void portAudioInit();
    bool portAudioOpen(const StreamContent& sc);
    void setErrorMsg(const QString& errMsg);
    void setState(QMediaPlayer::State state);
};

#endif // SBMEDIAPLAYER_H
