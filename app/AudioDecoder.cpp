#include <cstdlib>

#include <QDebug>
#include <QCoreApplication>

#include "Common.h"
#include "AudioDecoder.h"
#include "AudioDecoderReader.h"

///	Public methods
quint64
AudioDecoder::getSamples(void* buffer, quint64 sampleCount)
{
    SB_DEBUG_IF_NULL(_stream);
    if(_index==0)
    {
        QCoreApplication::processEvents();
    }
    quint64 bytesToRead=samplesToBytes(sampleCount);
    while(_index>=_maxScrollableIndex && _index<_length)
    {
        qDebug() << SB_DEBUG_ERROR << "WARNING! Reading unfilled area" << _index << _maxScrollableIndex << _length;
    }
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

quint64
AudioDecoder::setPosition(qint64 position)
{
    if(position<0)
    {
        _index=0;
    }

    qint64 newPosInSec=position/1000;
    qint64 currPosInSec=index2MS(_index)/1000;
    if(std::abs(newPosInSec-currPosInSec)>=2)
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
            _index=_maxScrollableIndex-bytesPerStereoSample();
        }
    }
    return _index;
}

///	Protected methods
AudioDecoder::AudioDecoder()
{
    _init();
}

AudioDecoder::~AudioDecoder()
{

    AudioDecoder::_exit();
}

bool
AudioDecoder::supportFileExtension(const QString& extension)
{
    Q_UNUSED(extension);
    return 0;
}

///	Private methods
void
AudioDecoder::_init()
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
AudioDecoder::_exit()
{
    if(_stream!=NULL)
    {
        free(_stream); _stream=NULL;
    }
}
