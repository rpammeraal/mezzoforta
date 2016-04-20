#ifndef AUDIODECODEROGGVORBIS_H
#define AUDIODECODEROGGVORBIS_H

#include "AudioDecoder.h"
#include "StreamContent.h"

class AudioDecoderOggVorbis : public AudioDecoder
{
public:
    AudioDecoderOggVorbis();
    virtual ~AudioDecoderOggVorbis();

    static bool supportFileExtension(const QString& extension);
    virtual StreamContent stream(const QString& fileName);
};

#endif // AUDIODECODEROGGVORBIS_H
