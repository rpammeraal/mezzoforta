#include <QDebug>

#include "Common.h"
#include "AudioDecoder.h"
#include "AudioDecoderReader.h"

///	Public methods
qint64
AudioDecoder::getSamples(void* buffer, qint64 sampleCount)
{
    SB_DEBUG_IF_NULL(_file);
    qint64 bytesToRead=samplesToBytes(sampleCount);
    if(_index+bytesToRead>lengthInBytes())
    {
        bytesToRead=lengthInBytes()-_index;
    }

    if(bytesToRead)
    {
        memcpy(buffer,(char *)&_stream[_index],bytesToRead);
        _index+=bytesToRead;
    }
    return bytesToSamples(bytesToRead);
}

qint64
AudioDecoder::setPosition(qint64 position)
{
    qDebug() << SB_DEBUG_INFO << position/1000 <<  index2MS(_index)/1000;
    if(std::abs((position/1000)-(index2MS(_index)/1000))>=2)
    {
        //	Don't reposition unless there's more than 1 sec of difference.
        _index=ms2Index(position);
        if(_index%bytesPerStereoSample()!=0)
        {
            //	Align with stero sample
            _index=(_index/4)*4;
        }
        if(_index>this->_maxScrollableIndex)
        {
            _index-=bytesPerStereoSample();
        }
    }
    return _index;
}

///	Protected methods
AudioDecoder::AudioDecoder()
{
    init();
}

AudioDecoder::~AudioDecoder()
{
    AudioDecoder::exit();
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
    _adr=NULL;
    _bitsPerSample=0;
    _error=QString();
    _file=NULL;
    _index=0;
    _length=0;
    _numChannels=0;
    _sampleRate=0;
    _sampleFormat=0;
    _stream=NULL;
}

void
AudioDecoder::exit()
{
    if(_adr)
    {
        delete _adr;_adr=NULL;
    }
    if(_stream!=NULL)
    {
        free(_stream); _stream=NULL;
    }
}
