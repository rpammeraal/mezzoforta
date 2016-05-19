#include <errno.h>
#include <fcntl.h>


//#include <QByteArray>
//#include <QDebug>
//#include <QFile>

#include "AudioDecoderOggVorbis.h"
#include "AudioDecoderOggVorbisReader.h"
#include "Common.h"

///	Protected methods
AudioDecoderOggVorbis::AudioDecoderOggVorbis(const QString& fileName)
{
    //	Init. What else.
    init();
    qDebug() << SB_DEBUG_INFO << fileName << this;

    //	Open file
    _file=new QFile(fileName);
    if(!_file->open(QIODevice::ReadOnly))
    {
        _error=QString("Error opening file: '%1' [%2]").arg(fileName).arg(_file->error());
        qDebug() << SB_DEBUG_ERROR << _error;
        return;
    }

    int resultCode=0;
    int fd=_file->handle();
    FILE* fp=fdopen(fd,"r");

#ifdef Q_OS_WIN
    resultCode=ov_open_callbacks(fp,&_ovf,NULL,0,OV_CALLBACKS_NOCLOSE);
#endif
#ifdef Q_OS_UNIX
    resultCode=ov_open(fp,&_ovf,NULL,0);
#endif
    if(resultCode!=0)
    {
        _error=QString("Could not open '%s' as a OGG file").arg(fileName);
        qDebug() << SB_DEBUG_ERROR << _error;
        return;
    }
    _ovInitialized=1;

    vorbis_info* vi=ov_info(&_ovf,-1);
    if(vi==NULL)
    {
        _error=QString("Could not get metadata from '%s'").arg(fileName);
        qDebug() << SB_DEBUG_NPTR << _error;
        return;
    }
    qDebug() << SB_DEBUG_INFO << "channels=" << vi->channels;
    qDebug() << SB_DEBUG_INFO << "rate=" << vi->rate;

    char **ptr=ov_comment(&_ovf,-1)->user_comments;
    while(*ptr)
    {
        qDebug() << SB_DEBUG_INFO << *ptr;
        ++ptr;
    }
    qDebug() << SB_DEBUG_INFO << "encoded by " <<  ov_comment(&_ovf,-1)->vendor;

    if(vi->channels!=2)
    {
        _error=QString("Only 2 ogg/vorbis channels supported '%s'").arg(fileName);
        qDebug() << SB_DEBUG_NPTR << _error;
        return;
    }

    quint64 numFrames=ov_pcm_total(&_ovf,-1);
    if(!numFrames)
    {
        _error=QString("Zero frames in '%s'").arg(fileName);
        qDebug() << SB_DEBUG_NPTR << _error;
        return;
    }

    //	Set stream parameters
    //	1.	_bitsPerSample
    _bitsPerSample=16;	//	CWIP: to be derived from meta data at some point

    //	2.	_length
    _length=numFrames*vi->channels*(_bitsPerSample/8);

    //	3.	_numChannels
    _numChannels=vi->channels;

    //	4.	_sampleRate
    _sampleRate=vi->rate;

    //	5.	_sampleFormat
    _sampleFormat=paInt16;	//	2 bytes per sample per channel. CWIP: to be derived from meta data

    //	Set file up for reading
    _file->reset();
    _file->seek(0);
    ov_time_seek(&_ovf,0);

    //	Allocate buffer to store stream in
    _stream=(char *)malloc(this->lengthInBytes());

    //	Put reader to work.
    _adr=new AudioDecoderOggVorbisReader(this);
    _adr->moveToThread(&_workerThread);
    connect(&_workerThread, &QThread::finished, _adr, &QObject::deleteLater);
    connect(this, &AudioDecoderOggVorbis::startBackfill, _adr, &AudioDecoderReader::backFill);
    _workerThread.start();
    emit startBackfill();
}

AudioDecoderOggVorbis::~AudioDecoderOggVorbis()
{
    qDebug() << SB_DEBUG_INFO;
    if(_ovInitialized)
    {
        qDebug() << SB_DEBUG_INFO;
        ov_clear(&_ovf);
    }
}

bool
AudioDecoderOggVorbis::supportFileExtension(const QString& extension)
{
    return
        (
            extension.compare("ogg",Qt::CaseInsensitive)==0
        ) ? 1: 0;
}

void
AudioDecoderOggVorbis::init()
{
#ifdef Q_BIG_ENDIAN
    _endianity=0;
#else
    _endianity=1;
#endif
    _ovInitialized=0;
}
