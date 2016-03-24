#include "StreamContent.h"

#include "Common.h"

StreamContent::StreamContent():_data(new Data())
{
    init();
}

StreamContent::StreamContent(const void* ptr, qint64 length, qint16 numChannels, qint32 sampleRate, PaSampleFormat sampleFormat, qint16 bitsPerSample)
{
    init();

    qDebug() << SB_DEBUG_INFO << "const ctor";
    void* _ptr=malloc(length);
    memcpy(_ptr,ptr,length);
    _data=new Data(_ptr,length,numChannels,sampleRate,sampleFormat,bitsPerSample);
}

StreamContent::StreamContent(void* ptr, qint64 length, qint16 numChannels, qint32 sampleRate, PaSampleFormat sampleFormat, qint16 bitsPerSample):
    _data(new Data(ptr,length,numChannels,sampleRate,sampleFormat,bitsPerSample))
{
    init();
    qDebug() << SB_DEBUG_INFO << "non-const ctor";
}

StreamContent::~StreamContent()
{
    if (--_data->_count==0)
    {
        delete _data;
    }
}

StreamContent::StreamContent(const StreamContent &f):_data(f._data)
{
    ++_data->_count;
}

StreamContent&
StreamContent::operator= (const StreamContent& f)
{
    Data* const old = _data;
    _data = f._data;
    ++_data->_count;
    if (--old->_count == 0)
    {
        delete old;
    }
    return *this;
}

void
StreamContent::init()
{
    _errMsg=QString();
    _hasErrorFlag=0;
    qDebug() << SB_DEBUG_INFO;
}
