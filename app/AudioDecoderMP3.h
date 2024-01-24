#ifndef AUDIODECODERMP3_H
#define AUDIODECODERMP3_H

#include <mad.h>
#include "AudioDecoder.h"

class AudioDecoderMP3 : public AudioDecoder
{
    Q_OBJECT

public:
    virtual ~AudioDecoderMP3();
    virtual QString getType() const { return QString("MP3"); }

protected:
    friend class AudioDecoderFactory;

    AudioDecoderMP3(const QString& fileName);

    static bool supportFileExtension(const QString& extension);

private:
    friend class AudioDecoderMP3Reader;
    quint64     _frameCount;
    mad_timer_t _fileLength;

    void init();
};

#endif // AUDIODECODERMP3_H
