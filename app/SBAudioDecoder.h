#ifndef SBAUDIODECODER_H
#define SBAUDIODECODER_H

#include <QString>
#include <QByteArray>

#include "StreamContent.h"

///
/// \brief The SBAudioDecoder class
///
/// This class is a parent class for all codecs. Given a fileName, stream()
/// will open this file, read, decode and return a bytestream accessible in
/// class streamContent.
///
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
