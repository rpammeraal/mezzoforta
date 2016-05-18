#ifndef AUDIODECODERMP3READER_H
#define AUDIODECODERMP3READER_H

#include <mad.h>

#include <QObject>

#include "AudioDecoderReader.h"

class AudioDecoderMP3;

class AudioDecoderMP3Reader : public AudioDecoderReader
{
    Q_OBJECT

public:
    AudioDecoderMP3Reader(AudioDecoderMP3* admp3);
    virtual ~AudioDecoderMP3Reader();

public slots:
    virtual void backFill();

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

#endif // AUDIODECODERMP3READER_H
