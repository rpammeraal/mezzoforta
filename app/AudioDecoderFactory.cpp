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

StreamContent
AudioDecoderFactory::stream(const QString &fileName)
{
    //	Determine file extension
    StreamContent sc;
    QByteArray ba;
    QFileInfo fi(fileName);
    QString extension=fi.suffix();
    AudioDecoder* ad=NULL;

    if(AudioDecoderWave::supportFileExtension(extension)==1)
    {
        ad=new AudioDecoderWave();
    }
    else if(AudioDecoderOggVorbis::supportFileExtension(extension)==1)
    {
        ad=new AudioDecoderOggVorbis();
    }
    else if(AudioDecoderMP3::supportFileExtension(extension)==1)
    {
        ad=new AudioDecoderMP3();
    }
#ifdef Q_OS_UNIX
    else if(AudioDecoderFlac::supportFileExtension(extension)==1)
    {
        ad=new AudioDecoderFlac();
    }
#endif

    if(ad==NULL)
    {
        errStr=QString("No decoder found for filename `%1', extension `%2'").arg(fileName).arg(extension);
        qDebug() << SB_DEBUG_INFO << "No decoder found for `" << fileName << "'";
    }
    else
    {
        sc=ad->stream(fileName);
    }
    delete ad; ad=NULL;

    return sc;
}

void
AudioDecoderFactory::init()
{
    errStr.clear();
}
