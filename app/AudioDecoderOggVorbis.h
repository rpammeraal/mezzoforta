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

public:
    virtual ~AudioDecoderOggVorbis();
    virtual QString getType() const { return QString("OGG"); }

protected:
    friend class AudioDecoderFactory;

    AudioDecoderOggVorbis(const QString& fileName);

    static bool supportFileExtension(const QString& extension);

private:
    friend class AudioDecoderOggVorbisReader;

    OggVorbis_File _ovf;
    int            _endianity;
    int            _ovInitialized;

    void _init();
};

#endif // AUDIODECODEROGGVORBIS_H
