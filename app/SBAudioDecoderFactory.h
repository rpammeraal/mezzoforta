#ifndef SBAUDIODECODERFACTORY_H
#define SBAUDIODECODERFACTORY_H

#include <QIODevice>

class SBAudioDecoderFactory
{
public:
    SBAudioDecoderFactory();
    ~SBAudioDecoderFactory();

    QIODevice* stream(const QString& fileName);
    QString error() const { return errStr; }

private:
    QString errStr;

    void init();
};

#endif // SBAUDIODECODERFACTORY_H
