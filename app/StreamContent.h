#ifndef STREAMCONTENT_H
#define STREAMCONTENT_H

#include <portaudio.h>

#include <QtGlobal>
#include <QString>

#include "Common.h"

class StreamContent
{
public:
    StreamContent();
    StreamContent(void* data, qint64 length, qint16 numChannels,qint32 sampleRate, PaSampleFormat sampleFormat, qint16 bitsPerSample);
    StreamContent(const void* data, qint64 length, qint16 numChannels, qint32 sampleRate, PaSampleFormat sampleFormat, qint16 bitsPerSample);
    StreamContent(const StreamContent& f);
    StreamContent& operator= (const StreamContent& f);
    ~StreamContent();

    inline qint16         bitsPerSample() const { return _data->_bitsPerSample; }
    inline void*          data() const { return _data->_ptr; }
    inline QString        errorMsg() const { return _data->_errMsg; }
    inline bool           hasErrorFlag() const { return _data->_hasErrorFlag; }
    inline qint64         length() const { return _data->_length; }
    inline qint16         numChannels() const { return _data->_numChannels; }
    inline PaSampleFormat sampleFormat() const { return _data->_sampleFormat; }
    inline qint32         sampleRate() const { return _data->_sampleRate; }
    inline void           setErrorMsg(const QString& errorMsg) { _data->setErrorMsg(errorMsg); }

private:
    class Data
    {
    public:
        Data()                        :
            _count(1),
            _ptr(NULL),
            _length(0),
            _numChannels(0),
            _sampleRate(0),
            _sampleFormat(0),
            _bitsPerSample(0),
            _errMsg(QString()),
            _hasErrorFlag(0)
        {
            qDebug() << SB_DEBUG_INFO;
        };
        Data(void* ptr, qint64 length,qint16 numChannels,qint32 sampleRate, PaSampleFormat sampleFormat, qint16 bitsPerSample):
            _count(1),
            _ptr(ptr),
            _length(length),
            _numChannels(numChannels),
            _sampleRate(sampleRate),
            _sampleFormat(sampleFormat),
            _bitsPerSample(bitsPerSample),
            _errMsg(QString()),
            _hasErrorFlag(0)
        {
            qDebug() << SB_DEBUG_INFO;
        };
        Data(const Data& d):
            _count(d._count),
            _ptr(d._ptr),
            _length(d._length),
            _numChannels(d._numChannels),
            _sampleRate(d._sampleRate),
            _sampleFormat(d._sampleFormat),
            _bitsPerSample(d._bitsPerSample),
            _errMsg(d._errMsg),
            _hasErrorFlag(d._hasErrorFlag)
        {
            qDebug() << SB_DEBUG_INFO;
        };
        ~Data()
        {
            if(_ptr)
            {
                free(_ptr);
            }
        }

        inline void           setErrorMsg(const QString& errorMsg)
            {
                qDebug() << SB_DEBUG_INFO << "set error to:" << errorMsg;
                _errMsg=errorMsg;
                _hasErrorFlag=1;
            }


        qint64         _count;
        void*          _ptr;
        qint64         _length;
        qint16         _numChannels;
        qint32         _sampleRate;
        PaSampleFormat _sampleFormat;
        qint16         _bitsPerSample;
        QString        _errMsg;
        bool           _hasErrorFlag;
    };

    Data* _data;
};

#endif // STREAMCONTENT_H
