#include <errno.h>
#include <fcntl.h>

#include <portaudio.h>

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

StreamContent
SBAudioDecoderWave::stream(const QString& fileName)
{
    //	New implementation using Qt infrastructure
    PaSampleFormat sampleFormat;
    StreamContent sc;
    qDebug() << SB_DEBUG_INFO << fileName;
    QFile f(fileName);
    if(!f.open(QIODevice::ReadOnly))
    {
        errStr=QString("Error opening file '%1' [%2]").arg(fileName).arg(f.error());
        qDebug() << SB_DEBUG_ERROR << errStr;
        //	CWIP: add error msg, status to StreamContent
        return sc;
    }

    QByteArray ba=f.readAll();
    qDebug() << SB_DEBUG_INFO << "fileSize=" << f.size();
    qDebug() << SB_DEBUG_INFO << "ba.size=" << ba.size();

    void* fileMap=(void *)ba.data();	//	Set filemap to start of data to interpret header.
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
        sc.setErrorMsg(errStr);
        qDebug() << SB_DEBUG_ERROR << errStr;
        return sc;
    }
    else  if(strncmp(wh->wave_ckID,"WAVE",4)!=0)
    {
        errStr="No WAVE-ID detected in '"+fileName+"'";
        sc.setErrorMsg(errStr);
        qDebug() << SB_DEBUG_ERROR << errStr;
        return sc;
    }
    else  if(strncmp(wh->fmt_ckID,"fmt",3)!=0)
    {
        errStr="No fmt-ID detected in '"+fileName+"'";
        sc.setErrorMsg(errStr);
        qDebug() << SB_DEBUG_ERROR << errStr;
        return sc;
    }
    else  if(strncmp(wh->data_ckID,"data",4)!=0)
    {
        errStr="No data-ID detected in '"+fileName+"'";
        sc.setErrorMsg(errStr);
        qDebug() << SB_DEBUG_ERROR << errStr;
        return sc;
    }
    else if(wh->nAvgBytesPerSec<=0)
    {
        errStr="Avg bytes/s <=0 in '"+fileName+"'";
        sc.setErrorMsg(errStr);
        qDebug() << SB_DEBUG_ERROR << errStr;
        return sc;
    }
    if(wh->formatTag == 1)
    {
        //	PCM
        switch(wh->nBitsPerSample)
        {
            case 8:
                sampleFormat = paInt8;
                break;

            case 16:
                sampleFormat = paInt16;
                break;

            case 32:
                sampleFormat = paInt32;
                break;

            default:
                errStr=QString("Unknown value `%1' in bitsPerSample").arg(wh->nBitsPerSample);
                sc.setErrorMsg(errStr);
                qDebug() << SB_DEBUG_ERROR << errStr;
                return sc;
        }
    }
    else if(wh->formatTag==3)
    {
        //	IEEE floatie
        if(wh->nBitsPerSample==32)
        {
            sampleFormat = paFloat32;
        }
        else
        {
            errStr=QString("Unsupported value `%1' in bitsPerSample [should be 32]").arg(wh->nBitsPerSample);
            sc.setErrorMsg(errStr);
            qDebug() << SB_DEBUG_ERROR << errStr;
            return sc;
        }
    }
    else
    {
        errStr=QString("Unknown value '`%1' in formatTag").arg(wh->formatTag);
        sc.setErrorMsg(errStr);
        qDebug() << SB_DEBUG_ERROR << errStr;
        return sc;
    }
    qDebug() << SB_DEBUG_INFO << "Wave header correct (so far)";

    //	Now create memory to put actual audio data in.
    //	Skip header
    qint64 size=f.size()-sizeof(WaveHeader);
    const void* src=(void *)f.map(sizeof(WaveHeader),size);
    sc=StreamContent(src, size,wh->nChannels,wh->nSamplesPerSec,sampleFormat,wh->nBitsPerSample);
    return sc;
}
