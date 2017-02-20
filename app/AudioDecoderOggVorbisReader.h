#ifndef AUDIODECODEROGGVORBISREADER_H
#define AUDIODECODEROGGVORBISREADER_H

#include <QObject>

#include "AudioDecoderReader.h"

class AudioDecoderOggVorbis;

class AudioDecoderOggVorbisReader : public AudioDecoderReader
{
    Q_OBJECT

public:
    AudioDecoderOggVorbisReader(AudioDecoderOggVorbis* adov);
    virtual ~AudioDecoderOggVorbisReader();

public slots:
    virtual void backFill();
};

#endif // AUDIODECODEROGGVORBISREADER_H
