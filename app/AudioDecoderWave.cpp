#include <errno.h>
#include <fcntl.h>

#include <portaudio.h>

#include <QBuffer>
#include <QDebug>
#include <QFile>

#include "AudioDecoderWave.h"
#include "AudioDecoderWaveReader.h"
#include "Common.h"

///	Protected methods
AudioDecoderWave::AudioDecoderWave(const QString& fileName)
{
    qDebug() << SB_DEBUG_INFO << fileName;

    //	Open file
    _file=new QFile(fileName);
    if(!_file->open(QIODevice::ReadOnly))
    {
        _error=QString("Error opening file: '%1' [%2]").arg(fileName).arg(_file->error());
        qDebug() << SB_DEBUG_ERROR << _error;
        return;
    }

    //	Inspect waveheader
    void* src=(void *)_file->map(0,sizeof(WaveHeader));
    WaveHeader* wh=(WaveHeader *)src;
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
    qDebug() << SB_DEBUG_INFO << "filesize - data_ckSize=" << _file->size() - wh->data_ckSize;

    //	Check header
    if(strncmp(wh->ckID,"RIFF",4)!=0)
    {
        _error="No RIFF-ID detected in '"+fileName+"'";
        qDebug() << SB_DEBUG_ERROR << _error;
        return;
    }
    else  if(strncmp(wh->wave_ckID,"WAVE",4)!=0)
    {
        _error="No WAVE-ID detected in '"+fileName+"'";
        return;
    }
    else  if(strncmp(wh->fmt_ckID,"fmt",3)!=0)
    {
        _error="No fmt-ID detected in '"+fileName+"'";
        qDebug() << SB_DEBUG_ERROR << _error;
        return;
    }
    else  if(strncmp(wh->data_ckID,"data",4)!=0)
    {
        _error="No data-ID detected in '"+fileName+"'";
        qDebug() << SB_DEBUG_ERROR << _error;
        return;
    }
    else if(wh->nAvgBytesPerSec<=0)
    {
        _error="Avg bytes/s <=0 in '"+fileName+"'";
        qDebug() << SB_DEBUG_ERROR << _error;
        return;
    }

    //	Set stream parameters
    //	1.	_bitsPerSample
    _bitsPerSample=wh->nBitsPerSample;

    //	2.	_length
    _length=wh->data_ckSize;

    //	3.	_numChannels
    _numChannels=wh->nChannels;

    //	4.	_sampleRate
    _sampleRate=wh->nSamplesPerSec;

    //	5.	_sampleFormat
    if(wh->formatTag == 1)
    {
        //	PCM
        switch(wh->nBitsPerSample)
        {
            case 8:
                _sampleFormat = paInt8;
                break;

            case 16:
                _sampleFormat = paInt16;
                break;

            case 32:
                _sampleFormat = paInt32;
                break;

            default:
                _error=QString("Unknown value `%1' in bitsPerSample").arg(wh->nBitsPerSample);
                qDebug() << SB_DEBUG_ERROR << _error;
                return;
        }
    }
    else if(wh->formatTag==3)
    {
        //	IEEE floatie
        if(wh->nBitsPerSample==32)
        {
            _sampleFormat = paFloat32;
        }
        else
        {
            _error=QString("Unsupported value `%1' in bitsPerSample [should be 32]").arg(wh->nBitsPerSample);
            qDebug() << SB_DEBUG_ERROR << _error;
            return;
        }
    }
    else
    {
        _error=QString("Unknown value '`%1' in formatTag").arg(wh->formatTag);
        qDebug() << SB_DEBUG_ERROR << _error;
        return;
    }

    //	Set file up for reading
    _file->reset();
    _file->seek(sizeof(WaveHeader));

    //	Allocate buffer to store stream in
    _stream=(char *)malloc(this->lengthInBytes());

    //	Put workerthread to work.
    _adr=new AudioDecoderWaveReader(this);
    _adr->moveToThread(&_workerThread);
    connect(&_workerThread, &QThread::finished, _adr, &QObject::deleteLater);
    connect(this, &AudioDecoderWave::startBackfill, _adr, &AudioDecoderReader::backFill);
    _workerThread.start();
    emit startBackfill();

}

AudioDecoderWave::~AudioDecoderWave()
{
}


bool
AudioDecoderWave::supportFileExtension(const QString& extension)
{
    return
        (
            extension.compare("wav",Qt::CaseInsensitive)==0 ||
            extension.compare("wave",Qt::CaseInsensitive)==0
        ) ? 1: 0;
}
