#include <qsystemdetection.h>
#include <QFileInfo>
#include <QString>

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
AudioDecoderFactory::fileSupportedFlag(const QFileInfo& fileInfo, AudioDecoder **newAudioDecoder)
{
    QString extension=fileInfo.suffix();
    if(AudioDecoderWave::supportFileExtension(extension)==1)
    {
        if(newAudioDecoder)
        {
            *newAudioDecoder=new AudioDecoderWave(fileInfo.absoluteFilePath());
        }
        return true;
    }
    else if(AudioDecoderOggVorbis::supportFileExtension(extension)==1)
    {
        if(newAudioDecoder)
        {
            *newAudioDecoder=new AudioDecoderOggVorbis(fileInfo.absoluteFilePath());
        }
        return true;
    }
    else if(AudioDecoderMP3::supportFileExtension(extension)==1)
    {
        if(newAudioDecoder)
        {
            *newAudioDecoder=new AudioDecoderMP3(fileInfo.absoluteFilePath());
        }
        return true;
    }
#ifdef Q_OS_UNIX
    else if(AudioDecoderFlac::supportFileExtension(extension)==1)
    {
        if(newAudioDecoder)
        {
            *newAudioDecoder=new AudioDecoderFlac(fileInfo.absoluteFilePath());
        }
        return true;
    }
#endif

    return false;
}

AudioDecoder*
AudioDecoderFactory::openFile(const QString &fileName)
{
    //	Flensburg - Neumuenster
    AudioDecoder* ad=NULL;

    QFileInfo fi(fileName);
    if(fileSupportedFlag(fi,&ad))
    {
        if(ad->error().length()!=0)
        {
            _error=ad->error();
            delete ad; ad=NULL;
        }
    }
    return ad;
}

void
AudioDecoderFactory::init()
{
    _error.clear();
}
