#include "AudioDecoderReader.h"

#include <QDebug>

#include "Common.h"

AudioDecoderReader::AudioDecoderReader(AudioDecoder* ad) : _ad(ad)
{
}

AudioDecoderReader::~AudioDecoderReader()
{
    qDebug() << SB_DEBUG_INFO << _fileName;
}
