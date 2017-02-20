#ifndef AUDIODECODERWAVEREADER_H
#define AUDIODECODERWAVEREADER_H

#include <QObject>

#include "AudioDecoderReader.h"

class AudioDecoderWave;

class AudioDecoderWaveReader : public AudioDecoderReader
{
    Q_OBJECT

public:
    AudioDecoderWaveReader(AudioDecoderWave* adw);
    virtual ~AudioDecoderWaveReader();

public slots:
    virtual void backFill();
};

#endif // AUDIODECODERWAVEREADER_H
