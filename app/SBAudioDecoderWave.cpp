#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>

#include <QBuffer>
#include <QDebug>

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
    qDebug() << SB_DEBUG_INFO << fileName;
    QByteArray fnba=fileName.toLatin1();

    int fd=open(fnba.data(), O_RDONLY, (mode_t)0600);
    if(fd==-1)
    {
        errStr=QString("Error opening file '%1': %2").arg(fileName).arg(QString(strerror(errno)));
        qDebug() << SB_DEBUG_ERROR << errStr;
        return NULL;
    }

    qDebug() << SB_DEBUG_INFO << "fd=" << fd;

    //	Get length of file
    off_t fileSize=lseek(fd,0,SEEK_END);
    lseek(fd,0,SEEK_SET);

    qDebug() << SB_DEBUG_INFO << "fileSize=" << fileSize;

    void* fileMap=malloc((size_t)fileSize+1);

    //	Allocate memory and map file to memory
    qDebug() << SB_DEBUG_INFO << "Alloc mem";
    fileMap=malloc((size_t)fileSize+1);

    qDebug() << SB_DEBUG_INFO << "Do mmap";
    fileMap=mmap(0,fileSize,PROT_READ,MAP_SHARED,fd,0);
    if(fileMap==MAP_FAILED)
    {
        close(fd);
        errStr=QString("Unable to mmap file '%1': %2").arg(fileName).arg(QString(strerror(errno)));
        qDebug() << SB_DEBUG_ERROR << errStr;
        return NULL;
    }

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
    qDebug() << SB_DEBUG_INFO << "filesize - data_ckSize=" << fileSize - wh->data_ckSize;

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

    void* startOfData=static_cast<char *>(fileMap)+44;
    QByteArray* ba=new QByteArray((char *)startOfData,wh->data_ckSize);
    QBuffer* b=new QBuffer(ba);
    return b;
}
