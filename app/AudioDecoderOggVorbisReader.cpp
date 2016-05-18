#include <QDebug>

#include "Common.h"
#include "AudioDecoderOggVorbisReader.h"
#include "AudioDecoderOggVorbis.h"

AudioDecoderOggVorbisReader::AudioDecoderOggVorbisReader(AudioDecoderOggVorbis* adov):AudioDecoderReader(adov)
{
}

AudioDecoderOggVorbisReader::~AudioDecoderOggVorbisReader()
{
}

void
AudioDecoderOggVorbisReader::backFill()
{
    qDebug() << SB_DEBUG_INFO;
    qint64 index=0;
    const int bufferSize=8192;
    int currentSection=0;
    while(index<_ad->lengthInBytes())
    {
        qint64 bytesRead=ov_read(
                    &dynamic_cast<AudioDecoderOggVorbis *>(_ad)->ovf,
                    ((char *)_ad->_stream)+index,bufferSize,
                    dynamic_cast<AudioDecoderOggVorbis *>(_ad)->endianity,
                    (_ad->bitsPerSample()/8),
                    1,
                    &currentSection);

        if(index==0)
        {
            qDebug() << SB_DEBUG_INFO << "Reading data:size=" << _ad->lengthInBytes();
        }
        if(bytesRead<=0)
        {
            //_ad->_error=QString("Unable to read '%s'").arg(fileName);
            qDebug() << SB_DEBUG_NPTR;
            return;
        }
        index+=bytesRead;
        _ad->_maxScrollableIndex=index;
    }
    emit QThread::currentThread()->exit();
}
