#ifndef SBMEDIAPLAYER_H
#define SBMEDIAPLAYER_H

#include <portaudio.h>

#include <QMediaPlayer>

#include "StreamContent.h"

#define CHECK(x) { if(!(x)) { \
fprintf(stderr, "%s:%i: failure at: %s\n", __FILE__, __LINE__, #x); \
return(0); } }

class SBMediaPlayer : public QMediaPlayer
{

    Q_OBJECT

public:
    SBMediaPlayer();
    ~SBMediaPlayer();

    void assignID(int playerID);
    int paCallback(const void *input, void *output, unsigned long frameCount, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags);
    bool play();
    bool setMedia(const QString& fileName);


private:
    int _playerID;
    StreamContent _sc;
    qint64 _index;
    bool _portAudioInitFlag;
    PaError _paError;
    PaStream* _stream;
    QString _errorStr;

    void init();
    void clear();
    void portAudioInit();
    bool portAudioOpen(const StreamContent& sc);

    //	REMOVÉ
};

#endif // SBMEDIAPLAYER_H
