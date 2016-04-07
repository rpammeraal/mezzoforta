#ifndef SBAUDIODECODERMP3_H
#define SBAUDIODECODERMP3_H

#include <mad.h>
#include "SBAudioDecoder.h"

class SBAudioDecoderMP3 : public SBAudioDecoder
{
public:
    SBAudioDecoderMP3();
    virtual ~SBAudioDecoderMP3();

    static bool supportFileExtension(const QString& extension);
    virtual StreamContent stream(const QString& fileName);

private:
    inline signed int madScale(mad_fixed_t sample) const
    {
        sample += (1L << (MAD_F_FRACBITS - 16));

        if (sample >= MAD_F_ONE)
        {
            sample = MAD_F_ONE - 1;
        }
        else if (sample < -MAD_F_ONE)
        {
            sample = -MAD_F_ONE;
        }

        return sample >> (MAD_F_FRACBITS + 1 - 16);
    }


};

#endif // SBAUDIODECODERMP3_H
