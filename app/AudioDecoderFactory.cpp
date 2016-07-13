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
AudioDecoderFactory::openFile(const QString &fileName,bool headerOnlyFlag)
{
    //	Flensburg - Neumuenster
    QFileInfo fi(fileName);
    QString extension=fi.suffix();
    AudioDecoder* ad=NULL;

    if(AudioDecoderWave::supportFileExtension(extension)==1)
    {
        ad=new AudioDecoderWave(fileName,headerOnlyFlag);
    }
    else if(AudioDecoderOggVorbis::supportFileExtension(extension)==1)
    {
        ad=new AudioDecoderOggVorbis(fileName,headerOnlyFlag);
    }
    else if(AudioDecoderMP3::supportFileExtension(extension)==1)
    {
        ad=new AudioDecoderMP3(fileName,headerOnlyFlag);
    }
#ifdef Q_OS_UNIX
    else if(AudioDecoderFlac::supportFileExtension(extension)==1)
    {
        ad=new AudioDecoderFlac(fileName,headerOnlyFlag);
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

AudioDecoder*
AudioDecoderFactory::openFileHeader(const QString &fileName)
{
    return this->openFile(fileName,1);
}

void
AudioDecoderFactory::init()
{
    _error.clear();
}
