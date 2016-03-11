#ifndef SBMEDIAPLAYER_H
#define SBMEDIAPLAYER_H

#include <vorbis/vorbisfile.h>

#include <QMediaPlayer>

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
    OggVorbis_File ovf;
    unsigned long fileLength;
    int numChannels;
    int sampleRate;
};

#endif // SBMEDIAPLAYER_H
