#include <errno.h>
#include <fcntl.h>

#include <QBuffer>
#include <QDebug>
#include <QFile>

#include "SBAudioDecoderWave.h"

#include "Common.h"

SBAudioDecoderWave::SBAudioDecoderWave()
{
}

SBAudioDecoderWave::~SBAudioDecoderWave()
{
}

bool
SBAudioDecoderWave::supportFileExtension(const QString& extension)
{
    return
            (
                extension.compare("wav",Qt::CaseInsensitive)==0 ||
                extension.compare("wave",Qt::CaseInsensitive)==0
            ) ? 1: 0;
}

QIODevice*
SBAudioDecoderWave::stream(const QString& fileName)
{
    //	New implementation using Qt infrastructure
    qDebug() << SB_DEBUG_INFO << fileName;
    QFile f(fileName);
    if(!f.open(QIODevice::ReadOnly))
    {
        errStr=QString("Error opening file '%1' [%2]").arg(fileName).arg(f.error());
        qDebug() << SB_DEBUG_ERROR << errStr;
        return NULL;
    }

    QByteArray ba=f.readAll();
    qDebug() << SB_DEBUG_INFO << "fileSize=" << f.size();
    qDebug() << SB_DEBUG_INFO << "ba.size=" << ba.size();

    char* fileMap=ba.data();
    WaveHeader* wh=(WaveHeader *)fileMap;
    qDebug() << SB_DEBUG_INFO << "ckID=" << wh->ckID;
    qDebug() << SB_DEBUG_INFO << "ckSize=" << wh->ckSize;
    qDebug() << SB_DEBUG_INFO << "wave_ckID=" << wh->wave_ckID;
    qDebug() << SB_DEBUG_INFO << "fmt_ckSize=" << wh->fmt_ckSize;
    qDebug() << SB_DEBUG_INFO << "formatTag=" << wh->formatTag;
    qDebug() << SB_DEBUG_INFO << "nChannels=" << wh->nChannels;
    qDebug() << SB_DEBUG_INFO << "nSamplesPerSec=" << wh->nSamplesPerSec;
    qDebug() << SB_DEBUG_INFO << "nAvgBytesPerSec=" << wh->nAvgBytesPerSec;
    qDebug() << SB_DEBUG_INFO << "nBitsPerSample=" << wh->nBitsPerSample;
    qDebug() << SB_DEBUG_INFO << "data_ckID=" << wh->data_ckID;
    qDebug() << SB_DEBUG_INFO << "data_ckSize=" << wh->data_ckSize;
    qDebug() << SB_DEBUG_INFO << "filesize - data_ckSize=" << f.size() - wh->data_ckSize;

    //	Check header
    if(strncmp(wh->ckID,"RIFF",4)!=0)
    {
        errStr="No RIFF-ID detected in '"+fileName+"'";
        qDebug() << SB_DEBUG_INFO << errStr;
        return NULL;
    }
    else  if(strncmp(wh->wave_ckID,"WAVE",4)!=0)
    {
        errStr="No WAVE-ID detected in '"+fileName+"'";
        qDebug() << SB_DEBUG_INFO << errStr;
        return NULL;
    }
    else  if(strncmp(wh->fmt_ckID,"fmt",3)!=0)
    {
        errStr="No fmt-ID detected in '"+fileName+"'";
        qDebug() << SB_DEBUG_INFO << errStr;
        return NULL;
    }
    else if(wh->formatTag!=1)
    {
        errStr="Format other than (1) not supported in '"+fileName+"'";
        qDebug() << SB_DEBUG_INFO << errStr;
        return NULL;
    }
    else  if(strncmp(wh->data_ckID,"data",4)!=0)
    {
        errStr="No data-ID detected in '"+fileName+"'";
        qDebug() << SB_DEBUG_INFO << errStr;
        return NULL;
    }
    else if(wh->nAvgBytesPerSec<=0)
    {
        errStr="Avg bytes/s <=0 in '"+fileName+"'";
        qDebug() << SB_DEBUG_INFO << errStr;
        return NULL;
    }
    qDebug() << SB_DEBUG_INFO << "Wave header correct (so far)";

    //	Remove header. This may cause the entire data to be copied instead of just updating
    //	a pointer to start of data. Unknown, but this may cause performance issues on
    //	mobile devices and/or environments with limited CPU/memory.
    qDebug() << SB_DEBUG_INFO << "oldSize=" << ba.size();
    ba=ba.remove(0,sizeof(WaveHeader));

    qDebug() << SB_DEBUG_INFO << "newSize" << ba.size();
    QBuffer* b=new QBuffer(&ba);
    b->open(QIODevice::ReadOnly);
    b->reset();
    b->seek(0);


    return b;
}
