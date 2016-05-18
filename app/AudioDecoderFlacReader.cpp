#include <QDebug>

#include "Common.h"
#include "AudioDecoderFlacReader.h"
#include "AudioDecoderFlac.h"

AudioDecoderFlacReader::AudioDecoderFlacReader(AudioDecoderFlac* adf):AudioDecoderReader(adf)
{
}

AudioDecoderFlacReader::~AudioDecoderFlacReader()
{
}

void
AudioDecoderFlacReader::backFill()
{
    unsigned int i=0;
    qint64 sampleIndex=0;	//	compatible with numSamples
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
    }

    //	Now prepare for exit.
    _ad->exit();
    emit QThread::currentThread()->exit();
}
