#ifndef AUDIODECODERFACTORY_H
#define AUDIODECODERFACTORY_H

#include <QIODevice>

//#include "StreamContent.h"

class AudioDecoder;
///
/// \brief The AudioDecoderFactory class
///
///
///	AudioDecoderFactory will open a file, read it and return a bytestream 2 samples per channel,
///	each sample being 16 bits, embedded and accessible in StreamContent, which holds all parameters
///	on this bytestream.
///
///	In the future we may go back to classical architecture of portAudio calling back for more data,
///	and have this callback function functioning at the actual decoder level. For now, everything is
///	streamed from memory.
///
class AudioDecoderFactory
{
public:
    AudioDecoderFactory();
    ~AudioDecoderFactory();

    //StreamContent stream(const QString& fileName);
    AudioDecoder* openFile(const QString& fileName);

    QString error() const { return errStr; }

private:
    QString errStr;

    void init();
};

#endif // AUDIODECODERFACTORY_H
