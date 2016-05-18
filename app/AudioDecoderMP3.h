#ifndef AUDIODECODERMP3_H
#define AUDIODECODERMP3_H

#include <mad.h>
#include "AudioDecoder.h"

class AudioDecoderMP3 : public AudioDecoder
{
protected:
    friend class AudioDecoderFactory;

    AudioDecoderMP3(const QString& fileName);
    virtual ~AudioDecoderMP3();

    static bool supportFileExtension(const QString& extension);

private:
    friend class AudioDecoderMP3Reader;
    qint64      _frameCount;
    mad_timer_t _fileLength;

    void init();
};

#endif // AUDIODECODERMP3_H
