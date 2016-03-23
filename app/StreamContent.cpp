#include "StreamContent.h"

#include "Common.h"

StreamContent::StreamContent():_data(new Data())
{
}

StreamContent::StreamContent(const void* ptr, qint64 length)
{
    qDebug() << SB_DEBUG_INFO << "const ctor";
    void* _ptr=malloc(length);
    memcpy(_ptr,ptr,length);
    for(int i=0;i<10;i++)
    {
        qDebug() << SB_DEBUG_INFO << i << (int)((char *)_ptr)[i];
    }
    _data=new Data(_ptr,length);
}

StreamContent::StreamContent(void* ptr, qint64 length):_data(new Data(ptr,length))
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

void*
StreamContent::data() const
{
    return _data->_ptr;
}
