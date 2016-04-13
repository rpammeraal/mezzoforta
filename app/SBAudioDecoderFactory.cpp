#include <QFileInfo>
#include <QString>

#include "Common.h"
#include "SBAudioDecoder.h"
#include "SBAudioDecoderFactory.h"
#include "SBAudioDecoderFlac.h"
#include "SBAudioDecoderMP3.h"
#include "SBAudioDecoderOggVorbis.h"
#include "SBAudioDecoderWave.h"

SBAudioDecoderFactory::SBAudioDecoderFactory()
{

}

SBAudioDecoderFactory::~SBAudioDecoderFactory()
{

}

StreamContent
SBAudioDecoderFactory::stream(const QString &fileName)
{
    //	Determine file extension
    StreamContent sc;
    QByteArray ba;
    QFileInfo fi(fileName);
    QString extension=fi.suffix();
    SBAudioDecoder* ad=NULL;

    if(SBAudioDecoderWave::supportFileExtension(extension)==1)
    {
        ad=new SBAudioDecoderWave();
    }
    else if(SBAudioDecoderOggVorbis::supportFileExtension(extension)==1)
    {
        ad=new SBAudioDecoderOggVorbis();
    }
    else if(SBAudioDecoderMP3::supportFileExtension(extension)==1)
    {
        ad=new SBAudioDecoderMP3();
    }
    else if(SBAudioDecoderFlac::supportFileExtension(extension)==1)
    {
        ad=new SBAudioDecoderFlac();
    }

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
SBAudioDecoderFactory::init()
{
    errStr.clear();
}
