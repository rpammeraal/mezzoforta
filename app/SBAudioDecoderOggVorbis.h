#ifndef SBAUDIODECODEROGGVORBIS_H
#define SBAUDIODECODEROGGVORBIS_H

//#include <vorbis/codec.h>
//#include <vorbis/vorbisfile.h>

#include "SBAudioDecoder.h"

class SBAudioDecoderOggVorbis : public SBAudioDecoder
{
protected:
    friend class SBAudioDecoderFactory;

    SBAudioDecoderOggVorbis();
    ~SBAudioDecoderOggVorbis();

    static bool supportFileExtension(const QString& extension);
    virtual QIODevice* stream(const QString& fileName);

private:
//    OggVorbis_File vf;
};

#endif // SBAUDIODECODEROGGVORBIS_H
