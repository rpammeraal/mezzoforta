//#ifndef AUDIODECODERMP3_H
//#define AUDIODECODERMP3_H

//#include <mad.h>
//#include "AudioDecoder.h"

//class AudioDecoderMP3 : public AudioDecoder
//{
//public:
//    AudioDecoderMP3();
//    virtual ~AudioDecoderMP3();

//    static bool supportFileExtension(const QString& extension);
//    virtual StreamContent stream(const QString& fileName);

//private:
//    inline signed int madScale(mad_fixed_t sample) const
//    {
//        sample += (1L << (MAD_F_FRACBITS - 16));

//        if (sample >= MAD_F_ONE)
//        {
//            sample = MAD_F_ONE - 1;
//        }
//        else if (sample < -MAD_F_ONE)
//        {
//            sample = -MAD_F_ONE;
//        }

//        return sample >> (MAD_F_FRACBITS + 1 - 16);
//    }


//};

//#endif // AUDIODECODERMP3_H
