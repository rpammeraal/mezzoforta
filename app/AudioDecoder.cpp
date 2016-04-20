#include "AudioDecoder.h"

AudioDecoder::AudioDecoder()
{
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

