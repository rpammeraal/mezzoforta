#ifndef STREAMCONTENT_H
#define STREAMCONTENT_H

#include <QtGlobal>

class StreamContent
{
public:
    StreamContent();
    StreamContent(void* data, qint64 length);
    StreamContent(const void* data, qint64 length);
    StreamContent(const StreamContent& f);
    StreamContent& operator= (const StreamContent& f);
    ~StreamContent();

    void* data() const;

private:
    class Data
    {
    public:
        Data()                        :_count(1),_ptr(NULL),_length(0) { };
        Data(void* ptr, qint64 length):_count(1),_ptr(ptr),_length(length) { };
        Data(const Data& d)           :_count(d._count),_ptr(d._ptr),_length(d._length) { };

        qint64 _count;
        void* _ptr;
        qint64 _length;
    };

    Data* _data;
};

#endif // STREAMCONTENT_H
