#ifndef SBMEDIAPLAYER_H
#define SBMEDIAPLAYER_H

//#include <vorbis/vorbisfile.h>

#include <QMediaPlayer>

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

private:
    int _playerID;

    void init();
    void clear();

    //	Added to ogg/vorbis
//    OggVorbis_File ovf;
//    unsigned long fileLength;
//    int numChannels;
//    int sampleRate;
};

#endif // SBMEDIAPLAYER_H
