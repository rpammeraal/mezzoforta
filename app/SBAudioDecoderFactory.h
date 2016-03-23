#ifndef SBAUDIODECODERFACTORY_H
#define SBAUDIODECODERFACTORY_H

#include <QIODevice>

#include "StreamContent.h"

class SBAudioDecoderFactory
{
public:
    SBAudioDecoderFactory();
    ~SBAudioDecoderFactory();

    StreamContent stream(const QString& fileName);
    QString error() const { return errStr; }

private:
    QString errStr;

    void init();
};

#endif // SBAUDIODECODERFACTORY_H
