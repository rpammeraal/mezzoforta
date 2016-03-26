#ifndef SBAUDIODECODEROGGVORBIS_H
#define SBAUDIODECODEROGGVORBIS_H

#include "SBAudioDecoder.h"
#include "StreamContent.h"

class SBAudioDecoderOggVorbis : public SBAudioDecoder
{
public:
    SBAudioDecoderOggVorbis();
    virtual ~SBAudioDecoderOggVorbis();

    static bool supportFileExtension(const QString& extension);
    virtual StreamContent stream(const QString& fileName);
};

#endif // SBAUDIODECODEROGGVORBIS_H
