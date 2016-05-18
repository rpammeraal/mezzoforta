#ifndef AUDIODECODEROGGVORBIS_H
#define AUDIODECODEROGGVORBIS_H

#include <ogg/ogg.h>
#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>

#include "AudioDecoder.h"

#define SB_VORBIS_BUFFER_SIZE 4096

class AudioDecoderOggVorbis : public AudioDecoder
{
    Q_OBJECT

protected:
    friend class AudioDecoderFactory;

    AudioDecoderOggVorbis(const QString& fileName);
    virtual ~AudioDecoderOggVorbis();

    static bool supportFileExtension(const QString& extension);

private:
    friend class AudioDecoderOggVorbisReader;

    OggVorbis_File ovf;
    int endianity;

    void init();
};

#endif // AUDIODECODEROGGVORBIS_H
