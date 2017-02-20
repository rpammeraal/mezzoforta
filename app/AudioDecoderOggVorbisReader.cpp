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
    quint64 index=0;
    const int bufferSize=8192;
    int currentSection=0;
    while(index<_ad->lengthInBytes())
    {
        quint64 bytesRead=ov_read(
                    &dynamic_cast<AudioDecoderOggVorbis *>(_ad)->_ovf,
                    ((char *)_ad->_stream)+index,bufferSize,
                    dynamic_cast<AudioDecoderOggVorbis *>(_ad)->_endianity,
                    (_ad->bitsPerSample()/8),
                    1,
                    &currentSection);

        if(bytesRead<=0)
        {
            _ad->_error=QString("Unable to read '%s'").arg(_fileName);
            return;
        }
        index+=bytesRead;
        _ad->_maxScrollableIndex=index;
    }
    emit QThread::currentThread()->exit();
}
