#include "StreamContent.h"

#include "Common.h"

StreamContent::StreamContent():_data(new Data())
{
}

StreamContent::StreamContent(const void* ptr, qint64 length, qint16 numChannels, qint32 sampleRate, PaSampleFormat sampleFormat, qint16 bitsPerSample)
{

    qDebug() << SB_DEBUG_INFO << "const ctor";
    void* _ptr=malloc(length);
    memcpy(_ptr,ptr,length);
    _data=new Data(_ptr,length,numChannels,sampleRate,sampleFormat,bitsPerSample);
}

StreamContent::StreamContent(void* ptr, qint64 length, qint16 numChannels, qint32 sampleRate, PaSampleFormat sampleFormat, qint16 bitsPerSample):
    _data(new Data(ptr,length,numChannels,sampleRate,sampleFormat,bitsPerSample))
{
    qDebug() << SB_DEBUG_INFO << "non-const ctor";
}

StreamContent::~StreamContent()
{
    if (--_data->_count==0)
    {
        delete _data;
    }
}

QDebug operator<<(QDebug dbg, const StreamContent& sc)
{
    dbg.nospace() << "bps=" ; dbg.nospace() << sc.bitsPerSample();
    dbg.nospace() << ":#channels=" ; dbg.nospace() << sc.numChannels();
    dbg.nospace() << ":#sampleRate=" ; dbg.nospace() << sc.sampleRate();
    dbg.nospace() << ":#sampleFormat=" ; dbg.nospace() << sc.sampleFormat();
    dbg.nospace() << ":#length=" ; dbg.nospace() << sc.length();
    dbg.space();
    return dbg;
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
