#include <QDebug>

#include "Common.h"
#include "AudioDecoderWave.h"
#include "AudioDecoderWaveReader.h"

AudioDecoderWaveReader::AudioDecoderWaveReader(AudioDecoderWave* adw):AudioDecoderReader(adw)
{
}

AudioDecoderWaveReader::~AudioDecoderWaveReader()
{
}

void
AudioDecoderWaveReader::backFill()
{
    qDebug() << SB_DEBUG_INFO << "start";
    SB_DEBUG_IF_NULL(_ad);
    SB_DEBUG_IF_NULL(_ad->_file);
    SB_DEBUG_IF_NULL(_ad->_stream);
    quint64 index=0;
    const int bufferSize=8192;
    while(index<_ad->lengthInBytes())
    {
        quint64 bytesRead=_ad->_file->read(((char *)_ad->_stream)+index,bufferSize);
        if(bytesRead<=0)
        {
            qDebug() << SB_DEBUG_NPTR;
            return;
        }
        index+=bytesRead;
        _ad->_maxScrollableIndex=index;
    }
    qDebug() << SB_DEBUG_INFO << "end";
    emit QThread::currentThread()->exit();
}
