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

AudioDecoder*
AudioDecoderFactory::openFile(const QString &fileName)
{
    //	Flensburg - Neumuenster
    QFileInfo fi(fileName);
    QString extension=fi.suffix();
    AudioDecoder* ad=NULL;

    if(AudioDecoderWave::supportFileExtension(extension)==1)
    {
        ad=new AudioDecoderWave(fileName);
    }
    else if(AudioDecoderOggVorbis::supportFileExtension(extension)==1)
    {
        ad=new AudioDecoderOggVorbis(fileName);
    }
    else if(AudioDecoderMP3::supportFileExtension(extension)==1)
    {
        ad=new AudioDecoderMP3(fileName);
    }
#ifdef Q_OS_UNIX
    else if(AudioDecoderFlac::supportFileExtension(extension)==1)
    {
        ad=new AudioDecoderFlac(fileName);
    }
#endif

    if(ad)
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
