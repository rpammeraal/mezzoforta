#include <QFileInfo>
#include <QString>

#include "Common.h"
#include "SBAudioDecoder.h"
#include "SBAudioDecoderFactory.h"
#include "SBAudioDecoderWave.h"

SBAudioDecoderFactory::SBAudioDecoderFactory()
{

}

SBAudioDecoderFactory::~SBAudioDecoderFactory()
{

}

QIODevice*
SBAudioDecoderFactory::stream(const QString &fileName)
{
    //	Determine file extension
    QIODevice* iod=NULL;
    QFileInfo fi(fileName);
    QString extension=fi.suffix();
    SBAudioDecoder* ad=NULL;

    if(SBAudioDecoderWave::supportFileExtension(extension)==1)
    {
        ad=new SBAudioDecoderWave();
    }

    if(ad==NULL)
    {
        errStr=QString("No decoder found for filename `%1', extension `%2'").arg(fileName).arg(extension);
        qDebug() << SB_DEBUG_INFO << "No decoder found for `" << fileName << "'";
    }
    else
    {
        iod=ad->stream(fileName);

        if(iod==NULL)
        {
            errStr=ad->error();
        }
        delete ad; ad=NULL;
    }

    return iod;
}

void
SBAudioDecoderFactory::init()
{
    errStr.clear();
}
