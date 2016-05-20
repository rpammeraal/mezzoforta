#ifndef AUDIODECODERREADER_H
#define AUDIODECODERREADER_H

#include <QObject>

class AudioDecoder;

class AudioDecoderReader : public QObject
{
    Q_OBJECT

public:
    AudioDecoderReader(AudioDecoder* ad);
    virtual ~AudioDecoderReader();

    QString _fileName;

public slots:
    virtual void backFill()=0;

protected:
    AudioDecoder* _ad;
};

#endif // AUDIODECODERREADER_H
