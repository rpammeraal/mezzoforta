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
    SB_DEBUG_IF_NULL(_ad);
    SB_DEBUG_IF_NULL(_ad->_file);
    SB_DEBUG_IF_NULL(_ad->_stream);
    qDebug() << SB_DEBUG_INFO << _ad->lengthInBytes();
    qint64 index=0;
    const int bufferSize=8192;
    while(index<_ad->lengthInBytes())
    {
        qint64 bytesRead=_ad->_file->read(((char *)_ad->_stream)+index,bufferSize);
        if(index==0)
        {
            qDebug() << SB_DEBUG_INFO << "Reading data:size=" << _ad->lengthInBytes();
        }
        if(bytesRead<=0)
        {
            qDebug() << SB_DEBUG_NPTR;
            return;
        }
        qDebug() << SB_DEBUG_INFO << index << _ad->index2MS(index)/1000;
        index+=bytesRead;
        _ad->_maxScrollableIndex=index;
    }
    emit QThread::currentThread()->exit();
}
