#ifndef AUDIODECODERFACTORY_H
#define AUDIODECODERFACTORY_H

#include <QIODevice>

class AudioDecoder;
///
/// \brief The AudioDecoderFactory class
///
///
///	AudioDecoderFactory will return an encoder appropriate for the file type.
///
class AudioDecoderFactory
{
public:
    AudioDecoderFactory();
    ~AudioDecoderFactory();

    static bool fileSupportedFlag(const QFileInfo& fileInfo,bool testFilePathOnly,AudioDecoder** newAudioDecoder=NULL);
    AudioDecoder* openFile(const QString& fileName, bool testFilePathOnly=0);

    QString error() const { return _error; }

private:
    QString _error;

    void init();
};

#endif // AUDIODECODERFACTORY_H
