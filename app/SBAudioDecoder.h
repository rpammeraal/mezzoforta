#ifndef SBAUDIODECODER_H
#define SBAUDIODECODER_H

#include <QString>
#include <QByteArray>

#include "StreamContent.h"

class SBAudioDecoder
{
protected:
    friend class SBAudioDecoderFactory;

    SBAudioDecoder();
    virtual ~SBAudioDecoder();

    static bool supportFileExtension(const QString& extension) ;
    virtual StreamContent stream(const QString& fileName)=0;

    virtual QString error() const { return errStr; }
    QString errStr;
};

#endif // SBAUDIODECODER_H
