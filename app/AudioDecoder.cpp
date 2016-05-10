#include "AudioDecoder.h"

///	Public methods
qint64
AudioDecoder::getSamples(void* buffer, qint64 sampleCount)	//	CWIP: remove, make pure virtual method()
{
    Q_UNUSED(buffer);
    Q_UNUSED(sampleCount);
    return 0;
}

qint64
AudioDecoder::setPosition(qint64 position)
{
    return 0;
}

///	Protected methods
AudioDecoder::AudioDecoder()
{
    init();
}

AudioDecoder::~AudioDecoder()
{
}

bool
AudioDecoder::supportFileExtension(const QString& extension)
{
    Q_UNUSED(extension);
    return 0;
}

///	Private methods
void
AudioDecoder::init()
{
    _bitsPerSample=0;
    _error=QString();
    _file=NULL;
    _length=0;
    _numChannels=0;
    _sampleRate=0;
    _sampleFormat=0;
}
