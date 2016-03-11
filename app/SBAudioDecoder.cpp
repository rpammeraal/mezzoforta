#include "SBAudioDecoder.h"

SBAudioDecoder::SBAudioDecoder()
{
}

SBAudioDecoder::~SBAudioDecoder()
{
}

bool
SBAudioDecoder::supportFileExtension(const QString& extension)
{
    return 0;
}

QIODevice*
SBAudioDecoder::stream(const QString& fileName)
{
    return NULL;
}
