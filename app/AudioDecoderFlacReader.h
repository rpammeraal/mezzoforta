#ifndef AUDIODECODERFLACREADER_H
#define AUDIODECODERFLACREADER_H

#include <QObject>

#include "AudioDecoderReader.h"

class AudioDecoderFlac;

class AudioDecoderFlacReader : public AudioDecoderReader
{
    Q_OBJECT

public:
    AudioDecoderFlacReader(AudioDecoderFlac* adf);
    virtual ~AudioDecoderFlacReader();

public slots:
    virtual void backFill();
};

#endif // AUDIODECODERFLACREADER_H
