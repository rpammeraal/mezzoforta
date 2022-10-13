#include <qsystemdetection.h>
#include <QFileInfo>
#include <QString>

#include "Common.h"
#include "AudioDecoder.h"
#include "AudioDecoderFactory.h"
#include "AudioDecoderFlac.h"
#include "AudioDecoderMP3.h"
#include "AudioDecoderOggVorbis.h"
#include "AudioDecoderWave.h"

AudioDecoderFactory::AudioDecoderFactory()
{

}

AudioDecoderFactory::~AudioDecoderFactory()
{

}

bool
AudioDecoderFactory::fileSupportedFlag(const QFileInfo& fileInfo, bool testFilePathOnly, AudioDecoder **newAudioDecoder)
{
    QString extension=fileInfo.suffix();
    if(AudioDecoderWave::supportFileExtension(extension)==1)
    {
        if(newAudioDecoder)
        {
            *newAudioDecoder=new AudioDecoderWave(fileInfo.absoluteFilePath(),testFilePathOnly);
        }
        return true;
    }
    else if(AudioDecoderOggVorbis::supportFileExtension(extension)==1)
    {
        if(newAudioDecoder)
        {
            *newAudioDecoder=new AudioDecoderOggVorbis(fileInfo.absoluteFilePath(),testFilePathOnly);
        }
        return true;
    }
    else if(AudioDecoderMP3::supportFileExtension(extension)==1)
    {
        if(newAudioDecoder)
        {
            *newAudioDecoder=new AudioDecoderMP3(fileInfo.absoluteFilePath(),testFilePathOnly);
        }
        return true;
    }
#ifdef Q_OS_UNIX
    else if(AudioDecoderFlac::supportFileExtension(extension)==1)
    {
        if(newAudioDecoder)
        {
            *newAudioDecoder=new AudioDecoderFlac(fileInfo.absoluteFilePath(),testFilePathOnly);
        }
        return true;
    }
#endif

    return false;
}

AudioDecoder*
AudioDecoderFactory::openFile(const QString &fileName,bool testFilePathOnly)
{
    //	Flensburg - Neumuenster
    AudioDecoder* ad=NULL;

    QFileInfo fi(fileName);
    qDebug() << SB_DEBUG_INFO;
    if(fileSupportedFlag(fi,testFilePathOnly,&ad))
    {
    qDebug() << SB_DEBUG_INFO;
        if(ad->error().length()!=0)
        {
    qDebug() << SB_DEBUG_INFO;
            _error=ad->error();
            delete ad; ad=NULL;
        }
    }
    qDebug() << SB_DEBUG_INFO;
    return ad;
}

void
AudioDecoderFactory::init()
{
    _error.clear();
}
