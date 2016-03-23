#ifndef SBMEDIAPLAYER_H
#define SBMEDIAPLAYER_H

//#include <vorbis/vorbisfile.h>

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

    void assignID(int playerID);
    virtual bool setMedia(const QString& fileName);
    int paCallback(const void *input, void *output, unsigned long frameCount, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags);

private:
    QByteArray _stream;
    int _playerID;
    StreamContent _sc;

    void init();
    void clear();

    qint64 _index;
    qint64 _length;
    void* _data;

    //	Added to ogg/vorbis
//    OggVorbis_File ovf;
//    unsigned long fileLength;
//    int numChannels;
//    int sampleRate;
};

#endif // SBMEDIAPLAYER_H
