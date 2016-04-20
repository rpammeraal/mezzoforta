#ifndef AUDIOdECODER_H
#define AUDIOdECODER_H

#include <QString>
#include <QByteArray>

#include "StreamContent.h"

///
/// \brief The AudioDecoder class
///
/// This class is a parent class for all codecs. Given a fileName, stream()
/// will open this file, read, decode and return a bytestream accessible in
/// class streamContent.
///
class AudioDecoder
{
protected:
    friend class AudioDecoderFactory;

    AudioDecoder();
    virtual ~AudioDecoder();

    static bool supportFileExtension(const QString& extension) ;
    virtual StreamContent stream(const QString& fileName)=0;

    virtual QString error() const { return errStr; }
    QString errStr;
};

#endif // AUDIOdECODER_H
