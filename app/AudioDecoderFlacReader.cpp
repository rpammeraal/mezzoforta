#include <QDebug>

#include "Common.h"
#include "AudioDecoderReader.h"
#include "AudioDecoderFlacReader.h"
#include "AudioDecoderFlac.h"

#ifdef Q_OS_UNIX

AudioDecoderFlacReader::AudioDecoderFlacReader(AudioDecoderFlac* adf):AudioDecoderReader(adf)
{
}

AudioDecoderFlacReader::~AudioDecoderFlacReader()
{
}

void
AudioDecoderFlacReader::backFill()
{
    SB_DEBUG_IF_NULL(_ad->_stream);
    qDebug() << SB_DEBUG_INFO << "start" << &(_ad->_stream);
    unsigned int i=0;
    quint64 sampleIndex=0;	//	compatible with numSamples
    qint16* samplePtr=(qint16 *)(_ad->_stream);
    while(sampleIndex<dynamic_cast<AudioDecoderFlac *>(_ad)->_numSamples)
    {
        if (dynamic_cast<AudioDecoderFlac *>(_ad)->_flacBufferLength == 0)
        {
            i=0;
            if (!FLAC__stream_decoder_process_single(dynamic_cast<AudioDecoderFlac *>(_ad)->_flacDecoder))
            {
                qDebug() << SB_DEBUG_ERROR << "SSFLAC: decoder_process_single returned false";
                break;
            }
            else if (dynamic_cast<AudioDecoderFlac *>(_ad)->_flacBufferLength == 0)
            {
                qDebug() << SB_DEBUG_INFO << "EOF";
                // EOF
                break;
            }
        }

        *(samplePtr++)=dynamic_cast<AudioDecoderFlac *>(_ad)->_flacBuffer[i++]; sampleIndex++;
        --(dynamic_cast<AudioDecoderFlac *>(_ad)->_flacBufferLength);

        quint64 bytesReadSoFar=(char *)samplePtr-(char *)(_ad->_stream);
        if(bytesReadSoFar%1000==0)
        {
            _ad->_maxScrollableIndex=bytesReadSoFar;
        }
    }

    //	Now prepare for exit.
    _ad->exit();
    qDebug() << SB_DEBUG_INFO << "end";
    emit QThread::currentThread()->exit();
}

#endif
